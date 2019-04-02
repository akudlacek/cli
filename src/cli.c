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
struct
{
	char buffer[CLI_MAX_BUFF_LEN];
	uint16_t buffer_ind;
	char prev_cmd[CLI_MAX_LEN_CMD];

	char *cmd;
	char *arg;
	
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
	float arg_float;
	int arg_int;
	int16_t rx_byte;
	uint8_t cmd_ind;
	uint8_t cmd_found_flag = 0;
	
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
		//echo character but suppress tab if enabled
		if((rx_byte != '\t') && (cli.conf.echo_enable == CLI_ECHO_ENABLED))
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
			if(cli.buffer_ind >= CLI_MAX_BUFF_LEN - 1)
			{
				//erase whole buffer
				cli.buffer_ind = 0;
				//todo: remove memset?
				memset(cli.buffer, 0, sizeof(cli.buffer)); //clear buffer
				
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
				cli.buffer_ind++;
			}
		}

		//////////////////////////////DELETE FROM LOCAL BUFFER//////////////////////////////
		//if backspace and there is data to backspace out
		else if(rx_byte == '\b' && cli.buffer_ind != 0)
		{
			//remove one from local buffer
			cli.buffer_ind--;
			cli.buffer[cli.buffer_ind] = 0;
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
			send_new_line();
			
			/*Send prompt next pass*/
			cli.prompt_sent_flag = 0;

			//get first token (command name)
			cli.cmd = strtok(cli.buffer, CLI_CMD_DELIMITER); //puts null at delimiter
			
			/*if the cmd tokens found, then search for a command*/
			if(cli.cmd != NULL)
			{
				//get arg ptr
				cli.arg = strrchr(cli.cmd, 0) + 1; //gets ptr to 1 after the delimiter that got converted to null

				//////////////////////////////COMMAND SEARCH AND COMMAND HANDELING//////////////////////////////
				for(cmd_ind = 0; cmd_ind < cli.num_cmds_added; cmd_ind++)
				{
					//if first entry of local token array matches one of the commands
					if (!strncmp(cli.cmd, cli.conf.cmd_list[cmd_ind].command_name, CLI_MAX_LEN_CMD))
					{
						//run command based on type
						switch(cli.conf.cmd_list[cmd_ind].arg_type)
						{
							case CLI_INT:
								if (cli.arg != NULL) //to prevent illegal mem access
									arg_int = atoi(cli.arg);
								cli.conf.cmd_list[cmd_ind].cmd_int(arg_int);
								break;
							case CLI_FLOAT:
								if (cli.arg != NULL) //to prevent illegal mem access
									arg_float = (float)atof(cli.arg);
								cli.conf.cmd_list[cmd_ind].cmd_float(arg_float);
								break;
							case CLI_STRING:
								cli.conf.cmd_list[cmd_ind].cmd_str(cli.arg);
								break;
							default:
								cli.conf.cmd_list[cmd_ind].cmd_void();
								break;
						}

						//to show that we found a command
						cmd_found_flag = 1;

						//records previous command for tab complete
						cli_strncpy(cli.prev_cmd, sizeof(cli.prev_cmd), cli.cmd, CLI_MAX_LEN_CMD);

						break; //break out of for loop if command found
					}
				}
			}
			
			//since static, reset to prepare for next command
			cli.buffer_ind = 0;

			//clear local received buffer, done to ensure strtok has null for next time
			//has to be done AFTER done with 'token', and 'token_arr' because they are pointing to 'buffer'
			memset(cli.buffer, 0, sizeof(cli.buffer)); //todo: remove memset?

			//prints if no command found
			if(cmd_found_flag == 0)
			{
				cli.conf.tx_string_fprt("ERROR: NO COMMAND FOUND TYPE \"help\"");
				cli.conf.tx_string_fprt(CLI_NEW_LINE);
			}
		}
		
		//Put newline on empty enter press
		else if((rx_byte == '\r') || (rx_byte == '\n'))
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
	uint8_t cmd_ind;

	char str[CLI_MAX_LEN_CMD + 2 + CLI_CMD_MAX_HELP_LENGTH + sizeof(CLI_NEW_LINE)];

	for (cmd_ind = 0; cmd_ind < cli.num_cmds_added; cmd_ind++)
	{
		//prints command name
		snprintf(str, sizeof(str), "%s: %s%s", cli.conf.cmd_list[cmd_ind].command_name, cli.conf.cmd_list[cmd_ind].help, CLI_NEW_LINE);
		cli.conf.tx_string_fprt(str);
	}
}

/******************************************************************************
*  \brief CLI STRNCPY
*
*  \note return -1 for err 0 for success
******************************************************************************/
int cli_strncpy(char *dest, size_t dest_size, const char *src, size_t src_size)
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
