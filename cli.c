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


/******************************************************************************
* Defines
******************************************************************************/
#define MAX_NUM_TOKENS 2                                //Defines the number of tokens involved in a command ***DO NOT CHANGE***
#define BUFFER_LEN (MAX_NUM_TOKENS * MAX_LEN_TOKENS)    //Size for buffer that accumulates the command and argument
#define CLI_LOC_STRN_SIZE_BYTES (MAX_LEN_TOKENS * 2)

//only used for local_token_array
enum
{
	COMMAND = 0,
	ARGUMENT = 1
};


/******************************************************************************
* Variables
******************************************************************************/
//cli data
struct
{
	char buffer[BUFFER_LEN];
	uint8_t buffer_ind;
	char prev_cmd[MAX_LEN_TOKENS];
	int16_t rx_byte;
	char *token;
	uint8_t token_ind;
	uint8_t cmd_ind;
	uint8_t cmd_found_flag;
	char strn[CLI_LOC_STRN_SIZE_BYTES];
	char token_arr[MAX_NUM_TOKENS][MAX_LEN_TOKENS];
	
	cli_conf_t conf;
	cli_command_t *cmd_list[MAX_NUM_COMMANDS];
	uint32_t num_cmds_added;
} cli;


/******************************************************************************
* Local Prototypes
******************************************************************************/
static void help_command(uint32_t cmd_num, char *arg_str);
static int16_t default_rx_byte(void);
static void default_tx_str(char *str);


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
	cli_conf->echo_enable = CLI_ECHO_DISABLED;
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
	cli.conf.echo_enable = cli_conf.echo_enable;
}

/******************************************************************************
*  \brief Add CLI Command
*
*  \note if parameters passed are incorrect this function will just not add it
******************************************************************************/
void cli_add_command(char *cmd_name, void (*command_fptr)(uint32_t, char *))
{
	//NULL check
	if(cmd_name != NULL || command_fptr != NULL)
	{
		//Number of command check
		if(cli.num_cmds_added < MAX_NUM_COMMANDS)
		{
			//Point command list to memory allocated for command
			cli.cmd_list[cli.num_cmds_added] = (cli_command_t *)malloc(sizeof(cli_command_t));
			
			//Load command configuration
			strncpy(cli.cmd_list[cli.num_cmds_added]->command_name, cmd_name, MAX_LEN_TOKENS - 1);  //add command name (this will be what you type to use a command)
			cli.cmd_list[cli.num_cmds_added]->command_fptr = command_fptr;                          //points command to your function
			
			//Increment the number of commands
			cli.num_cmds_added++;
		}
	}
}

/******************************************************************************
*  \brief Command Line Interface Task
*
*  \note run in main while loop.
******************************************************************************/
void cli_task(void)
{
	cli.rx_byte        = -1;
	cli.token           = 0;
	cli.cmd_found_flag  = 0;
	
	/*If cli is disabled do not run*/
	if(cli.conf.enable == CLI_DISABLED) return;

	//grab from received buffer
	cli.rx_byte = cli.conf.rx_byte_fptr();

	//if we have received data
	if(cli.rx_byte != -1)
	{
		//echo character but suppress tab if enabled
		if((cli.rx_byte != '\t') && (cli.conf.echo_enable == CLI_ECHO_ENABLED))
		{
			cli.strn[0] = cli.rx_byte;
			cli.strn[1] = 0;
			cli.conf.tx_string_fprt(cli.strn);
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
				cli.conf.tx_string_fprt("\r\nERROR: COMMAND LENGTH\r\n");
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
			memset(cli.buffer, 0, BUFFER_LEN); //clear buffer
			strncpy(cli.buffer, cli.prev_cmd, BUFFER_LEN - 1); //copy last valid command leaving space for null
			cli.buffer_ind = strlen(cli.prev_cmd); //update index
			cli.conf.tx_string_fprt(cli.buffer);
		}

		//////////////////////////////COMMAND ENTERED (TOKENIZATION)//////////////////////////////
		//if enter key pressed and buffer has some data
		else if(((cli.rx_byte == '\r') || (cli.rx_byte == '\n')) && (cli.buffer_ind != 0))
		{
			//print new line
			cli.conf.tx_string_fprt("\r\n");
			
			//clear token index
			cli.token_ind = 0;
			
			//clear token array
			memset(cli.token_arr, 0, sizeof(cli.token_arr));

			//get first token
			cli.token = strtok(cli.buffer, CMD_DELIMITER);

			//get tokens from local received buffer and put into local token array
			while(cli.token != NULL)
			{
				//make sure we don't exceed the max number of tokens
				//break out if we exceed the max
				if(cli.token_ind >= MAX_NUM_TOKENS)
				{
					cli.conf.tx_string_fprt("\r\nERROR: TOKEN COUNT\r\n");
					break; //leave while loop
				}
				
				//if token is too large
				if(strlen(cli.token) >= MAX_LEN_TOKENS)
				{
					cli.conf.tx_string_fprt("\r\nERROR: TOKEN LENGTH\r\n");
					break;
				}
				//copys token to token array
				else
				{
					//leaves room for null terminator
					strncpy(&cli.token_arr[cli.token_ind][0], cli.token, MAX_LEN_TOKENS - 1);
				}

				//get next token
				cli.token = strtok(NULL, CMD_DELIMITER);
				cli.token_ind++;
			}

			//since static, reset to prepare for next command
			cli.buffer_ind = 0;

			//clear local received buffer, done to ensure strtok has null for next time
			memset(cli.buffer, 0, BUFFER_LEN);

			//////////////////////////////COMMAND SEARCH AND COMMAND HANDELING//////////////////////////////
			for(cli.cmd_ind = 0; cli.cmd_ind < cli.num_cmds_added; cli.cmd_ind++)
			{
				//if first entry of local token array matches one of the commands
				if(!strncmp(&cli.token_arr[COMMAND][0], cli.cmd_list[cli.cmd_ind]->command_name, MAX_LEN_TOKENS))
				{
					//run user function sends command number and ptr to argument
					cli.cmd_list[cli.cmd_ind]->command_fptr(cli.cmd_ind, &cli.token_arr[ARGUMENT][0]);
					
					//to show that we found a command
					cli.cmd_found_flag = 1;

					//records previous command for tab complete, leaves room for null terminator
					strncpy(cli.prev_cmd, &cli.token_arr[COMMAND][0], MAX_LEN_TOKENS - 1);

					break; //break out of for loop if command found
				}
			}

			//prints if no command found
			if(cli.cmd_found_flag == 0)
			{
				cli.conf.tx_string_fprt("\r\nERROR: NO COMMAND FOUND TYPE \"help\"\r\n");
			}
		}
	}
}

/******************************************************************************
*  \brief Add help command
*
*  \note
******************************************************************************/
void cli_add_help_command(void)
{
	cli_add_command("help", help_command);
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
void cli_print(char *null_term_str)
{
	/*If cli is disabled do not run*/
	if(cli.conf.enable == CLI_DISABLED) return;
	
	cli.conf.tx_string_fprt(null_term_str);
}


/**************************************************************************************************
*                                       LOCAL FUNCTIONS
**************************************************************************************************/
/******************************************************************************
*  \brief Help command
*
*  \note
******************************************************************************/
static void help_command(uint32_t cmd_num, char *arg_str)
{
	for(cli.cmd_ind = 0; cli.cmd_ind < cli.num_cmds_added; cli.cmd_ind++)
	{
		//prints command name
		snprintf(cli.strn, CLI_LOC_STRN_SIZE_BYTES, "%*s\r\n", MAX_LEN_TOKENS - 1, cli.cmd_list[cli.cmd_ind]->command_name);
		cli.conf.tx_string_fprt(cli.strn);
	}
}

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
static void default_tx_str(char *str)
{
	//empty
}
