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
#define MAX_NUM_COMMANDS 25 //Max number of commands that can be entered
#define MAX_NUM_TOKENS 2    //Defines the number of tokens involved in a command ***DO NOT CHANGE***
#define MAX_LEN_TOKENS 10   //Max length of each token, should be greater than largest key word or argument
#define MAX_NUM_STRN_ARG 2  //How many different possible string arguments
#define CMD_DELIMITER " "   //Delimiter for tokens

#define BUFFER_LEN (MAX_NUM_TOKENS * MAX_LEN_TOKENS) //Size for buffer that accumulates the command and argument

/*Argument type for command*/
typedef enum
{
	NONE,
	STRING,
	NUMBER
} cli_argument_type_t;

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
	cli_argument_type_t argument_type;                       //declares if argument is a number or string
	char string_arguments[MAX_NUM_STRN_ARG][MAX_LEN_TOKENS]; //holds string argument key words
	void (*command_fptr)(char *, int32_t);                   //pointer to your command function
} cli_command_t;


/******************************************************************************
* Prototypes
******************************************************************************/
void cli_init(int16_t (*rx_byte_fptr)(void), void (*tx_string_fprt)(char*));
void cli_add_command(char *cmd_name, cli_argument_type_t arg_type, void (*command_fptr)(char *, int32_t), char *str_arg_0, char *str_arg_1);
void cli_task(void);
void cli_add_help_command(void);


#endif /* CLI_H_ */
