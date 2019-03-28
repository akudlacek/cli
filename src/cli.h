/*
* cli.h
*
*  Created on: Aug 4, 2016
*      Author: akudlacek
*/

#ifndef CLI_H_
#define CLI_H_


#include <stdint.h>


/**************************************************************************************************
*                                             DEFINES
*************************************************^************************************************/
#define CLI_MAX_NUM_CMDS         30    //Max number of commands that can be entered ***the larger the number the more memory cli uses***
#define CLI_MAX_LEN_CMD_ARG      10    //Max length of each token, should be greater than largest cmd name or argument including null terminator
#define CLI_MAX_STRN_LEN         255   //Largest string for arg type: CLI_STRING including null terminator
#define CLI_CMD_DELIMITER        " "   //Delimiters for tokens
#define CLI_PROMPT              "> "   //The prompt for user input
#define CLI_NEW_LINE          "\r\n"   //New line thats used within cli

/*CLI return status*/
typedef enum
{
	CLI_SUCESS,
	CLI_FAIL_NULL_PARAM,
	CLI_FAIL_OUT_OF_CMD_SLOTS,
	CLI_FAIL_CMD_NAME_LEN,
	CLI_FAIL_CMD_NAME_ILLEGAL_CHAR,
	CLI_FAIL_UNSUPPORTED_ARG_TYPE
} cli_return_t;

/*CLI enable disable enum*/
typedef enum
{
	CLI_DISABLED,
	CLI_ENABLED
} cli_enable_t;

/*CLI echo enable disable enum*/
typedef enum
{
	CLI_ECHO_DISABLED,
	CLI_ECHO_ENABLED
} cli_echo_enable_t;

/*Cli argument type*/
typedef enum
{
	CLI_NO_ARG,
	CLI_INT,
	CLI_FLOAT,
	CLI_STRING
} cli_arg_type_t;

/*CLI configuration struct*/
typedef	struct
{
	int16_t (*rx_byte_fptr)(void);     //function pointer for received byte return -1 for no data or >=0 for ascii data
	void (*tx_string_fprt)(char*);     //function pointer for transmit null terminated string
	cli_enable_t enable;               //enables or disable cli
	cli_echo_enable_t echo_enable;     //enables or disables echo
} cli_conf_t;

/*Command configuration struct*/
typedef struct
{
	char command_name[CLI_MAX_LEN_CMD_ARG]; //holds command keyword
	cli_arg_type_t arg_type;                //holds the argument type

	union cmd_fptr
	{
		void(*cmd_void)(void);            //pointer to your command function, no argument
		void(*cmd_int)(int);              //pointer to your command function, int argument
		void(*cmd_float)(float);          //pointer to your command function, float argument
		void(*cmd_str)(char *);           //pointer to your command function, string argument
	};
} cli_command_t;


/**************************************************************************************************
*                                            PROTOTYPES
*************************************************^************************************************/
void cli_get_config_defaults(cli_conf_t *cli_conf);
void cli_init(cli_conf_t cli_conf);
cli_return_t cli_add_command(char *cmd_name, cli_arg_type_t arg_type, void(*command_fptr));
void cli_task(void);
void cli_add_help_command(void);
void cli_enable(cli_enable_t enable);
void cli_print(char *null_term_str);


#endif /* CLI_H_ */
