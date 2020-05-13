/*
* cli.c
*
*  Created on: Aug 4, 2016
*      Author: akudlacek
*/


#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**************************************************************************************************
*                                             DEFINES
*************************************************^************************************************/


/**************************************************************************************************
*                                            VARIABLES
*************************************************^************************************************/
//cli data
static struct
{
	char buffer[CLI_MAX_LEN_BUFF];
	uint16_t buffer_ind;
	char prev_cmd[CLI_MAX_LEN_CMD];

	char *cmd;
	char *arg;
	
	cli_conf_t conf;
	uint32_t num_cmds_added;

	uint8_t prompt_sent_flag;
} cli = {0};

static const cli_command_t default_cmd_list[1] =
{
	CLI_CMD_LIST_END // must be LAST
};


/**************************************************************************************************
*                                         LOCAL PROTOTYPES
*************************************************^************************************************/
static int16_t default_rx_byte(void);
static void default_tx_str(const char *str);


/**************************************************************************************************
*                                            FUNCTIONS
*************************************************^************************************************/
/******************************************************************************
*  \brief CLI get config defaults
*
*  \note
******************************************************************************/
void cli_get_config_defaults(cli_conf_t * const cli_conf)
{
	cli_conf->rx_byte_fptr    = default_rx_byte;
	cli_conf->tx_string_fprt  = default_tx_str;
	cli_conf->enable          = CLI_ENABLED;
	cli_conf->echo_enable     = CLI_ECHO_ENABLED;
	cli_conf->cmd_list        = default_cmd_list;
}

/******************************************************************************
*  \brief Init CLI
*
*  \note Point to receive byte and transmit string functions
******************************************************************************/
void cli_init(const cli_conf_t cli_conf)
{
	cli.conf    = cli_conf;

	cli.prompt_sent_flag     = 0;
	cli.num_cmds_added       = 0;

	//Count number of commands in list
	//looks for first null in first char of string of the command name
	while(cli.conf.cmd_list[cli.num_cmds_added].command_name[0] != 0)
	{
		//note: this will not check for null function pointers
		cli.num_cmds_added++;
	}
}

/******************************************************************************
*  \brief Command Line Interface Task
*
*  \note run in main while loop.
******************************************************************************/
void cli_task(void)
{
	char echo_str[2];
	float arg_float = 0;
	int arg_int = 0;
	unsigned long arg_ulint = 0;
	int16_t rx_byte;
	uint8_t cmd_ind = 0;
	char *saveptr;
	
	/*If cli is disabled do not run*/
	if(cli.conf.enable == CLI_DISABLED) return;

	/*Send prompt for CLI*/
	if(cli.prompt_sent_flag == 0)
	{
		cli.conf.tx_string_fprt(CLI_PROMPT);
		cli.prompt_sent_flag = 1;
	}

	//grab from received buffer
	rx_byte = cli.conf.rx_byte_fptr();

	//if we have received data
	if(rx_byte != -1)
	{
		//echo character but suppress tab and backspace if enabled
		if((rx_byte != '\t') && (rx_byte != '\b') && (cli.conf.echo_enable == CLI_ECHO_ENABLED))
		{
			echo_str[0] = (char)rx_byte;
			echo_str[1] = 0;
			cli.conf.tx_string_fprt(echo_str);
		}

		//////////////////////////////ADD TO LOCAL BUFFER//////////////////////////////
		//if data is ASCII printable text
		if((rx_byte >= ' ') && (rx_byte <= '~'))
		{
			//if exceed buffer length minus one for null terminator
			//aways leave last null terminator
			if(cli.buffer_ind >= CLI_MAX_LEN_BUFF - 1)
			{
				//reset buffer index
				cli.buffer_ind = 0;
				 //necessary to clear whole buffer because there could be garbage after the
				 //cmd name entry that can be interpreted as an argument input
				 //this also null terminates
				memset(cli.buffer, 0, sizeof(cli.buffer));
				
				cli.conf.tx_string_fprt(CLI_NEW_LINE); //These new lines need to stay on
				cli.conf.tx_string_fprt("ERROR: COMMAND LENGTH");
				cli.conf.tx_string_fprt(CLI_NEW_LINE); //These new lines need to stay on

				/*Send prompt next pass*/
				cli.prompt_sent_flag = 0;
			}
			else
			{
				//put in local buffer
				cli.buffer[cli.buffer_ind] = (char)rx_byte;
				cli.buffer[cli.buffer_ind + 1] = 0; //null terminate
				cli.buffer_ind++;
			}
		}

		//////////////////////////////DELETE FROM LOCAL BUFFER//////////////////////////////
		//if backspace and there is data to backspace out
		else if(rx_byte == '\b' && cli.buffer_ind != 0)
		{
			//remove one from local buffer
			cli.buffer_ind--;
			cli.buffer[cli.buffer_ind] = 0; //null terminate
			
			//send backspace
			cli.conf.tx_string_fprt("\b");
		}

		//////////////////////////////TAB LAST COMMAND//////////////////////////////
		else if(rx_byte == '\t')
		{
			cli_strncpy(cli.buffer, sizeof(cli.buffer), cli.prev_cmd, sizeof(cli.prev_cmd)); //copy last valid command
			cli.buffer_ind = (uint16_t)strlen(cli.prev_cmd);  //update index
			
			cli.conf.tx_string_fprt(cli.buffer);
		}

		//////////////////////////////COMMAND ENTERED (TOKENIZATION)//////////////////////////////
		//if enter key pressed and buffer has some data
		else if(((rx_byte == '\r') || (rx_byte == '\n')) && (cli.buffer_ind != 0))
		{
			cli.conf.tx_string_fprt(CLI_NEW_LINE);
			
			/*Send prompt next pass*/
			cli.prompt_sent_flag = 0;

			//get first token (command name)
			cli.cmd = cli_strtok_r(cli.buffer, CLI_CMD_DELIMITER, &saveptr); //puts null at delimiter
			
			/*if the cmd tokens found, then search for a command*/
			if(cli.cmd != NULL)
			{
				//get arg ptr
				cli.arg = strrchr(cli.cmd, 0) + 1; //gets ptr to 1 after the delimiter that got converted to null

				//////////////////////////////COMMAND SEARCH AND COMMAND HANDELING//////////////////////////////
				for(cmd_ind = 0; cmd_ind < cli.num_cmds_added; cmd_ind++)
				{
					//if first entry of local token array matches one of the commands
					if(!strncmp(cli.cmd, cli.conf.cmd_list[cmd_ind].command_name, CLI_MAX_LEN_CMD))
					{
						//Check for valid function pointer
						if(cli.conf.cmd_list[cmd_ind].fptr.cmd_void == NULL)
						{
							cli.conf.tx_string_fprt("ERROR: NULL FUNCTION POINTER");
							cli.conf.tx_string_fprt(CLI_NEW_LINE);
						}
						else
						{
							//run command based on type
							switch(cli.conf.cmd_list[cmd_ind].arg_type)
							{
								if(cli.arg != NULL) //to prevent illegal mem access
								{
									case CLI_INT:
									arg_int = atoi(cli.arg);
									cli.conf.cmd_list[cmd_ind].fptr.cmd_int(arg_int);
									break;
									case CLI_UINT8:
									arg_int = atoi(cli.arg);
									cli.conf.cmd_list[cmd_ind].fptr.cmd_uint8((uint8_t)arg_int);
									break;
									case CLI_ULINT:
									arg_ulint = strtoul(cli.arg, NULL, 0);
									cli.conf.cmd_list[cmd_ind].fptr.cmd_ulint(arg_ulint);
									break;
									case CLI_FLOAT:
									arg_float = (float)atof(cli.arg);
									cli.conf.cmd_list[cmd_ind].fptr.cmd_float(arg_float);
									break;
								}
								case CLI_STRING:
								cli.conf.cmd_list[cmd_ind].fptr.cmd_str(cli.arg);
								break;
								default:
								cli.conf.cmd_list[cmd_ind].fptr.cmd_void();
								break;
							}

							//records previous command for tab complete
							cli_strncpy(cli.prev_cmd, sizeof(cli.prev_cmd), cli.cmd, CLI_MAX_LEN_CMD);
						}

						break; //break out of for loop if command found
					}
				}
			}
			
			//reset buffer index
			cli.buffer_ind = 0;
			//necessary to clear whole buffer because there could be garbage after the
			//cmd name entry that can be interpreted as an argument input
			//this also null terminates
			memset(cli.buffer, 0, sizeof(cli.buffer));

			//prints if no command found, when cmd_ind is past the end of the list of commands
			if(cmd_ind == cli.num_cmds_added)
			{
				//This advice is only valid if the help command is present in the command table
				cli.conf.tx_string_fprt("ERROR: NO COMMAND FOUND TYPE \"help\"");
				cli.conf.tx_string_fprt(CLI_NEW_LINE);
			}
		}
		
		//Put newline on empty enter press
		else if((rx_byte == '\r') || (rx_byte == '\n'))
		{
			cli.conf.tx_string_fprt(CLI_NEW_LINE);

			/*Send prompt next pass*/
			cli.prompt_sent_flag = 0;
		}
	}
}

/******************************************************************************
*  \brief CLI enable disable
*
*  \note disables or enables cli task and cli print
******************************************************************************/
void cli_enable(const cli_enable_t enable)
{
	cli.conf.enable = enable;
}

/******************************************************************************
*  \brief CLI print message
*
*  \note
******************************************************************************/
void cli_print(const char * const null_term_str)
{
	/*If cli is disabled do not run*/
	if(cli.conf.enable == CLI_DISABLED) return;
	
	cli.conf.tx_string_fprt(CLI_NEW_LINE);
	cli.conf.tx_string_fprt(null_term_str);
	
	/*Send prompt next pass*/
	cli.prompt_sent_flag = 0;
}

/******************************************************************************
*  \brief Help command
*
*  \note
******************************************************************************/
void cli_help_command(void)
{
	uint8_t cmd_ind;

	char str[CLI_MAX_LEN_CMD + 1 + CLI_MAX_LEN_HELP_DESC + sizeof(CLI_NEW_LINE)];

	for(cmd_ind = 0; cmd_ind < cli.num_cmds_added; cmd_ind++)
	{
		//prints command name
		snprintf(str, sizeof(str), "%-*s %s%s", CLI_MAX_LEN_CMD, cli.conf.cmd_list[cmd_ind].command_name, cli.conf.cmd_list[cmd_ind].help, CLI_NEW_LINE);
		cli.conf.tx_string_fprt(str);
	}
}

/******************************************************************************
*  \brief CLI STRNCPY
*
*  \note return -1 for err 0 for success
******************************************************************************/
int cli_strncpy(char * const dest, const size_t dest_size, const char * const src, const size_t src_size)
{
	char *a = dest;
	char *b = &dest[dest_size - 1];
	const char *c = src;
	const char *d = &src[src_size - 1];
	size_t index;

	//check for null pointer
	if(dest == 0 || src == 0)
		return -1;

	//check for zero size
	if(dest_size == 0 || src_size == 0)
		return -1;

	//check for overlap
	if((a <= c && b >= c) || (c <= a && d >= a))
		return -1;

	for(index = 0; index < dest_size; index++)
	{
		//Exceeded src size
		if(index == src_size)
		{
			dest[index] = 0;
			break;
		}

		dest[index] = src[index];

		//Src null found
		if(src[index] == 0)
		{
			break;
		}
	}

	//Put null at end
	dest[dest_size - 1] = 0;

	return 0;
}

/******************************************************************************
*  \brief CLI STRTOK_R, Reentrant string tokenizer.
*
*  \note Taken from The GNU C Library (glibc)
*        https://github.com/bminor/glibc/blob/master/string/strtok_r.c
*       
* Parse S into tokens separated by characters in DELIM.
*  If S is NULL, the saved pointer in SAVE_PTR is used as
*  the next starting point.  For example:
*   char s[] = "-abc-=-def";
*   char *sp;
*   x = strtok_r(s, "-", &sp);	// x = "abc", sp = "=-def"
*   x = strtok_r(NULL, "-=", &sp);	// x = "def", sp = NULL
*   x = strtok_r(NULL, "=", &sp);	// x = NULL
*      // s = "abc\0-def\0"
******************************************************************************/
char * cli_strtok_r(char *s, const char * const delim, char ** const save_ptr)
{
	char *end;

	if (s == NULL)
	s = *save_ptr;

	if (*s == '\0')
	{
		*save_ptr = s;
		return NULL;
	}

	/* Scan leading delimiters.  */
	s += strspn (s, delim);
	if (*s == '\0')
	{
		*save_ptr = s;
		return NULL;
	}

	/* Find the end of the token.  */
	end = s + strcspn (s, delim);
	if (*end == '\0')
	{
		*save_ptr = end;
		return s;
	}

	/* Terminate the token and make *SAVE_PTR point past it.  */
	*end = '\0';
	*save_ptr = end + 1;
	return s;
}


/**************************************************************************************************
*                                         LOCAL FUNCTIONS
*************************************************^************************************************/
/******************************************************************************
*  \brief Default rx byte function
*
*  \note
******************************************************************************/
static int16_t default_rx_byte(void)
{
	return -1;
}

/******************************************************************************
*  \brief Default tx string function
*
*  \note
******************************************************************************/
static void default_tx_str(const char *str)
{
	//empty
}

