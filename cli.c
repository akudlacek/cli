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
	uint8_t arg_ind;
	uint8_t cmd_found_flag;
	uint8_t arg_found_flag;
	char strn[MAX_LEN_TOKENS];
	char token_arr[MAX_NUM_TOKENS][MAX_LEN_TOKENS];
	
	cli_config_t conf;
	cli_command_t *cmd_list[MAX_NUM_COMMANDS];
	uint32_t num_cmds_added;
} cli;


/******************************************************************************
* Local Prototypes
******************************************************************************/
static void help_command(char * str, int32_t num);


/******************************************************************************
*  \brief Init CLI
*
*  \note Point to receive byte and transmit string functions
******************************************************************************/
void cli_init(int16_t (*rx_byte_fptr)(void), void (*tx_string_fprt)(char*))
{
	//configure cli
	cli.conf.rx_byte_fptr = rx_byte_fptr;
	cli.conf.tx_string_fprt = tx_string_fprt;
}

/******************************************************************************
*  \brief Add CLI Command
*
*  \note
******************************************************************************/
void cli_add_command(char *cmd_name, cli_argument_type_t arg_type, void (*command_fptr)(char *, int32_t), char *str_arg_0, char *str_arg_1)
{
	//NULL check
	if(cmd_name != NULL || command_fptr != NULL)
	{
		//Point command list to memory allocated for command
		cli.cmd_list[cli.num_cmds_added] = (cli_command_t *)malloc(sizeof(cli_command_t));
	
		//Load command configuration
		strncpy(cli.cmd_list[cli.num_cmds_added]->command_name, cmd_name, MAX_LEN_TOKENS - 1);          //add command name (this will be what you type to use a command)
		cli.cmd_list[cli.num_cmds_added]->argument_type = arg_type;                //this is the command argument (NONE just calls the function you put in below)
		strncpy(cli.cmd_list[cli.num_cmds_added]->string_arguments[0], str_arg_0, MAX_LEN_TOKENS - 1);  //add string arg if used
		strncpy(cli.cmd_list[cli.num_cmds_added]->string_arguments[1], str_arg_1, MAX_LEN_TOKENS - 1);  //add string arg if used
		cli.cmd_list[cli.num_cmds_added]->command_fptr = command_fptr;             //points command to your function
	
		//Increment the number of commands
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
	cli.rx_byte = -1;
	cli.token = 0;
	cli.cmd_found_flag = 0;
	cli.arg_found_flag = 0;

	//grab from received buffer
	cli.rx_byte = cli.conf.rx_byte_fptr();

	//if we have received data
	if(cli.rx_byte != -1)
	{
		//echo character but suppress tab
		if(cli.rx_byte != '\t')
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

			//get first token
			cli.token = strtok(cli.buffer, CMD_DELIMITER);
			
			//clear token index
			cli.token_ind = 0;

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
					switch(cli.cmd_list[cli.cmd_ind]->argument_type)
					{
						//NOTE: if argument type is NONE then no argument so just set command ready flag

						case STRING:
						//search for string argument
						for(cli.arg_ind = 0; cli.arg_ind < MAX_NUM_STRN_ARG; cli.arg_ind++)
						{
							//if token string matches an argument
							if(!strncmp(&cli.token_arr[ARGUMENT][0], cli.cmd_list[cli.cmd_ind]->string_arguments[cli.arg_ind], MAX_LEN_TOKENS))
							{
								//run user function
								cli.cmd_list[cli.cmd_ind]->command_fptr(&cli.token_arr[ARGUMENT][0], cli.arg_ind);
								
								//show we found argument
								cli.arg_found_flag = 1;

								break; //since command found break out of search
							}
						}

						//prints if no argument found
						if(cli.arg_found_flag == 0)
						{
							cli.conf.tx_string_fprt("\r\nERROR: INVALID ARGUMENT TYPE \"help\"\r\n");
						}
						break;

						case NUMBER:
						//run user function and convert ASCII to number from second token entry put in found command
						cli.cmd_list[cli.cmd_ind]->command_fptr(&cli.token_arr[ARGUMENT][0], strtol(&cli.token_arr[ARGUMENT][0], NULL, 10));
						break;
						
						case NONE:
						//run user function
						cli.cmd_list[cli.cmd_ind]->command_fptr(&cli.token_arr[COMMAND][0], 0);
						break;
					}
					
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
				cli.conf.tx_string_fprt("\r\nERROR: INVALID COMMAND TYPE \"help\"\r\n");
			}
			
			//clear token array
			memset(cli.token_arr, 0, sizeof(cli.token_arr));
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
	cli_add_command("help", NONE, help_command, NULL, NULL);
}

/******************************************************************************
*  \brief Help command
*
*  \note
******************************************************************************/
static void help_command(char * str, int32_t num)
{
	for(cli.cmd_ind = 0; cli.cmd_ind < cli.num_cmds_added; cli.cmd_ind++)
	{
		//prints command name
		snprintf(cli.strn, MAX_LEN_TOKENS, "%*s", MAX_LEN_TOKENS - 1, cli.cmd_list[cli.cmd_ind]->command_name);
		cli.conf.tx_string_fprt(cli.strn);

		//print argument type
		switch(cli.cmd_list[cli.cmd_ind]->argument_type)
		{
			case NONE:
			cli.conf.tx_string_fprt(" NONE\r\n");
			break;
			
			case STRING:
			cli.conf.tx_string_fprt(" STRING: ");

			//print all string arguments
			for(cli.arg_ind = 0; cli.arg_ind < MAX_NUM_STRN_ARG; cli.arg_ind++)
			{
				cli.conf.tx_string_fprt(cli.cmd_list[cli.cmd_ind]->string_arguments[cli.arg_ind]);
				cli.conf.tx_string_fprt(" ");
			}
			cli.conf.tx_string_fprt("\r\n");

			break;
			
			case NUMBER:
			cli.conf.tx_string_fprt(" NUMBER\r\n");
			break;
		}
	}
}