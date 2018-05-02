/*
* cli.h
*
*  Created on: Aug 4, 2016
*      Author: akudlacek
*/

#ifndef CLI_H_
#define CLI_H_


#include <stdint.h>


/******************************************************************************
* Defines
******************************************************************************/
#define MAX_NUM_COMMANDS 26 //Max number of commands that can be entered ***the larger the number the more memory cli uses***
#define MAX_LEN_TOKENS 10   //Max length of each token, should be greater than largest key word or argument
#define CMD_DELIMITER " "   //Delimiter for tokens

/*CLI configuration struct*/
typedef	struct
{
	int16_t (*rx_byte_fptr)(void);    //function pointer for received byte return -1 for no data or >=0 for ascii data
	void (*tx_string_fprt)(char*);    //function pointer for transmit null terminated string
} cli_config_t;

/*Command configuration struct*/
typedef struct
{
	char command_name[MAX_LEN_TOKENS];                       //holds command keyword
	void (*command_fptr)(uint32_t, char *);                  //pointer to your command function fun_ptr(command_num, argument_str)
} cli_command_t;


/******************************************************************************
* Prototypes
******************************************************************************/
void cli_init(int16_t (*rx_byte_fptr)(void), void (*tx_string_fprt)(char*));
void cli_add_command(char *cmd_name, void (*command_fptr)(uint32_t, char *));
void cli_task(void);
void cli_add_help_command(void);


#endif /* CLI_H_ */
