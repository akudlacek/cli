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
#define MAX_NUM_TOKENS     2                                                        //Defines the number of tokens involved in a command ***DO NOT CHANGE***
#define BUFFER_LEN        ((MAX_NUM_TOKENS * CLI_MAX_LEN_CMD_ARG) + CLI_MAX_STRN_LEN)    //Size for buffer that accumulates the command and argument

//only used for local_token_array
enum
{
	COMMAND = 0,
	ARGUMENT = 1
};


/**************************************************************************************************
*                                            VARIABLES
*************************************************^************************************************/
//cli data
struct
{
	char buffer[BUFFER_LEN];
	uint16_t buffer_ind;
	char prev_cmd[CLI_MAX_LEN_CMD_ARG];
	int16_t rx_byte;
	char *token;
	uint8_t token_ind;
	uint8_t cmd_ind;
	uint8_t cmd_found_flag;
	char strn[CLI_MAX_STRN_LEN];
	char *token_arr[MAX_NUM_TOKENS];
	
	cli_conf_t conf;
	uint32_t num_cmds_added;

	uint8_t prompt_sent_flag;
} cli;

static const cli_command_t default_cmd_lsit[] =
{
	CLI_CMD_LIST_END                                             // must be LAST
};


/**************************************************************************************************
*                                         LOCAL PROTOTYPES
*************************************************^************************************************/
static int16_t default_rx_byte(void);
static void default_tx_str(const char *str);

static void send_new_line(void);


/**************************************************************************************************
*                                            FUNCTIONS
*************************************************^************************************************/
/******************************************************************************
*  \brief CLI get config defaults
*
*  \note
******************************************************************************/
void cli_get_config_defaults(cli_conf_t *cli_conf)
{
	cli_conf->rx_byte_fptr    = default_rx_byte;
	cli_conf->tx_string_fprt  = default_tx_str;
	cli_conf->enable          = CLI_ENABLED;
	cli_conf->echo_enable     = CLI_ECHO_DISABLED;
	cli_conf->cmd_list        = default_cmd_lsit;
}

/******************************************************************************
*  \brief Init CLI
*
*  \note Point to receive byte and transmit string functions
******************************************************************************/
void cli_init(cli_conf_t cli_conf)
{
	cli.conf.rx_byte_fptr    = cli_conf.rx_byte_fptr;
	cli.conf.tx_string_fprt  = cli_conf.tx_string_fprt;
	cli.conf.enable          = cli_conf.enable;
	cli.conf.echo_enable     = cli_conf.echo_enable;
	cli.conf.cmd_list        = cli_conf.cmd_list;

	cli.prompt_sent_flag     = 0;
	cli.num_cmds_added       = 0;

	//Count number of commands in list
	//looks for first null in first char of string of the command name
	while(cli.conf.cmd_list[cli.num_cmds_added].command_name[0] != 0)
	{
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
	int arg_int     = 0;
	
	/*If cli is disabled do not run*/
	if(cli.conf.enable == CLI_DISABLED) return;

	/*Send prompt for CLI*/
	if(cli.prompt_sent_flag == 0)
	{
		cli.conf.tx_string_fprt(CLI_PROMPT);
		cli.prompt_sent_flag = 1;
	}

	//grab from received buffer
	cli.rx_byte = cli.conf.rx_byte_fptr();

	//if we have received data
	if(cli.rx_byte != -1)
	{
		//echo character but suppress tab if enabled
		if((cli.rx_byte != '\t') && (cli.conf.echo_enable == CLI_ECHO_ENABLED))
		{
			echo_str[0] = (char)cli.rx_byte;
			echo_str[1] = 0;
			cli.conf.tx_string_fprt(echo_str);
		}

		//////////////////////////////ADD TO LOCAL BUFFER//////////////////////////////
		//if data is ASCII printable text
		if((cli.rx_byte >= ' ') && (cli.rx_byte <= '~'))
		{
			//if exceed buffer length minus one for null terminator
			//aways leave last null terminator
			if(cli.buffer_ind >= BUFFER_LEN - 1)
			{
				//erase whole buffer
				cli.buffer_ind = 0;
				memset(cli.buffer, 0, BUFFER_LEN); //clear buffer
				
				cli.conf.tx_string_fprt(CLI_NEW_LINE); //These new lines need to stay on
				cli.conf.tx_string_fprt("ERROR: COMMAND LENGTH");
				cli.conf.tx_string_fprt(CLI_NEW_LINE); //These new lines need to stay on

				/*Send prompt next pass*/
				cli.prompt_sent_flag = 0;
			}
			else
			{
				//put in local buffer
				cli.buffer[cli.buffer_ind] = (char)cli.rx_byte;
				cli.buffer_ind++;
			}
		}

		//////////////////////////////DELETE FROM LOCAL BUFFER//////////////////////////////
		//if backspace and there is data to backspace out
		else if(cli.rx_byte == '\b' && cli.buffer_ind != 0)
		{
			//remove one from local buffer
			cli.buffer_ind--;
			cli.buffer[cli.buffer_ind] = 0;
		}

		//////////////////////////////TAB LAST COMMAND//////////////////////////////
		else if(cli.rx_byte == '\t')
		{
			cli_strncpy(cli.buffer, sizeof(cli.buffer), cli.prev_cmd, sizeof(cli.prev_cmd)); //copy last valid command
			cli.buffer_ind = (uint16_t)strlen(cli.prev_cmd);  //update index
			
			cli.conf.tx_string_fprt(cli.buffer);
		}

		//////////////////////////////COMMAND ENTERED (TOKENIZATION)//////////////////////////////
		//if enter key pressed and buffer has some data
		else if(((cli.rx_byte == '\r') || (cli.rx_byte == '\n')) && (cli.buffer_ind != 0))
		{
			send_new_line();
			
			/*Send prompt next pass*/
			cli.prompt_sent_flag = 0;
			
			//clear token index
			cli.token_ind = 0;
			
			//clear token array
			memset(cli.token_arr, 0, sizeof(cli.token_arr));

			//get first token
			cli.token = strtok(cli.buffer, CLI_CMD_DELIMITER);
			
			//get string after first token
			if(cli.token != NULL)
			{
				//since strtok places a null at the first delimiter
				//we need to find the last null start coping from there
				cli_strncpy(
								cli.strn,                                                          //dest
								sizeof(cli.strn),                                                  //size of dest
								(strrchr(cli.token, 0) + 1),                                       //src - right after token null
								((sizeof(cli.buffer) - 1) - (strnlen(cli.buffer, sizeof(cli.buffer)) + 1)) //size of src - (buffer num of bytes - ((len of first token + 1for null))
						);
			}

			//get tokens from local received buffer and put into local token array
			while(cli.token != NULL)
			{	
				//make sure we don't exceed the max number of tokens
				//break out if we exceed the max
				if(cli.token_ind >= MAX_NUM_TOKENS)
				{
					break; //leave while loop
				}
				
				//if token is too large
				if(strlen(cli.token) >= CLI_MAX_LEN_CMD_ARG)
				{
					break; //leave while loop
				}
				
				//copys token to token array
				else
				{
					cli.token_arr[cli.token_ind] = cli.token;
				}

				//get next token
				cli.token = strtok(NULL, CLI_CMD_DELIMITER);
				cli.token_ind++;
			}

			/*if there are tokens found, then search for a command*/
			if(cli.token_ind > 0)
			{
				//////////////////////////////COMMAND SEARCH AND COMMAND HANDELING//////////////////////////////
				for (cli.cmd_ind = 0; cli.cmd_ind < cli.num_cmds_added; cli.cmd_ind++)
				{
					//if first entry of local token array matches one of the commands
					if (!strncmp(cli.token_arr[COMMAND], cli.conf.cmd_list[cli.cmd_ind].command_name, CLI_MAX_LEN_CMD_ARG))
					{
						//run command based on type
						switch(cli.conf.cmd_list[cli.cmd_ind].arg_type)
						{
							case CLI_INT:
								if (cli.token_arr[ARGUMENT] != NULL) //to prevent illegal mem access
									arg_int = atoi(cli.token_arr[ARGUMENT]);
								cli.conf.cmd_list[cli.cmd_ind].cmd_int(arg_int);
								break;
							case CLI_FLOAT:
								if (cli.token_arr[ARGUMENT] != NULL) //to prevent illegal mem access
									arg_float = (float)atof(cli.token_arr[ARGUMENT]);
								cli.conf.cmd_list[cli.cmd_ind].cmd_float(arg_float);
								break;
							case CLI_STRING:
								cli.conf.cmd_list[cli.cmd_ind].cmd_str(cli.strn);
								break;
							default:
								cli.conf.cmd_list[cli.cmd_ind].cmd_void();
								break;
						}

						//to show that we found a command
						cli.cmd_found_flag = 1;

						//records previous command for tab complete
						cli_strncpy(cli.prev_cmd, sizeof(cli.prev_cmd), cli.token_arr[COMMAND], CLI_MAX_LEN_CMD_ARG);

						break; //break out of for loop if command found
					}
				}
			}
			
			//since static, reset to prepare for next command
			cli.buffer_ind = 0;

			//clear local received buffer, done to ensure strtok has null for next time
			//has to be done AFTER done with 'token', and 'token_arr' because they are pointing to 'buffer'
			memset(cli.buffer, 0, BUFFER_LEN);

			//prints if no command found
			if(cli.cmd_found_flag == 0)
			{
				cli.conf.tx_string_fprt("ERROR: NO COMMAND FOUND TYPE \"help\"");
				cli.conf.tx_string_fprt(CLI_NEW_LINE);
			}
			
			//reset flag
			cli.cmd_found_flag  =  0;
		}
		
		//Put newline on empty enter press
		else if((cli.rx_byte == '\r') || (cli.rx_byte == '\n'))
		{
			send_new_line();

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
void cli_enable(cli_enable_t enable)
{
	cli.conf.enable = enable;
}

/******************************************************************************
*  \brief CLI print message
*
*  \note
******************************************************************************/
void cli_print(const char *null_term_str)
{
	/*If cli is disabled do not run*/
	if(cli.conf.enable == CLI_DISABLED) return;
	
	cli.conf.tx_string_fprt(null_term_str);
}

/******************************************************************************
*  \brief Help command
*
*  \note
******************************************************************************/
void cli_help_command(void)
{
	char str[CLI_MAX_LEN_CMD_ARG + 2 + CLI_CMD_MAX_HELP_LENGTH + sizeof(CLI_NEW_LINE)];

	for (cli.cmd_ind = 0; cli.cmd_ind < cli.num_cmds_added; cli.cmd_ind++)
	{
		//prints command name
		snprintf(str, sizeof(str), "%s: %s%s", cli.conf.cmd_list[cli.cmd_ind].command_name, cli.conf.cmd_list[cli.cmd_ind].help, CLI_NEW_LINE);
		cli.conf.tx_string_fprt(str);
	}
}

/******************************************************************************
*  \brief CLI STRNCPY
*
*  \note
******************************************************************************/
char * cli_strncpy(char *dest, size_t dest_size, const char *src, size_t src_size)
{
	char *a = dest;
	char *b = dest + dest_size;
	const char *c = src;
	const char *d = src + src_size;
	size_t index;

	//check for null pointer
	if(dest == 0 || src == 0)
		return dest;

	//check for zero size
	if(dest_size == 0 || src_size == 0)
		return dest;

	//check for overlap
	if((a <= c && b >= c) || (c <= a && d >= a))
		return dest;

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

	return dest;
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

/******************************************************************************
*  \brief Send new line
*
*  \note function so new line can be turned on and off with
*        CLI_ECHO_ENABLED/CLI_ECHO_DISABLED
******************************************************************************/
static void send_new_line(void)
{
	if(cli.conf.echo_enable == CLI_ECHO_ENABLED)
		cli.conf.tx_string_fprt(CLI_NEW_LINE);
}
