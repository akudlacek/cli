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
#define CLI_MAX_LEN_CMD          10    //Max length cmd name, including null terminator
#define CLI_MAX_BUFF_LEN         255   //Max total length of command name and arguments/string input, including null terminator

/*HELP*/
#define CLI_CMD_MAX_HELP_LENGTH  64    //if this is zero, there will be no help
#if CLI_CMD_MAX_HELP_LENGTH > 0
#define HELP(x)  (x)
#else
#define HELP(x)	  0
#endif

#define CLI_CMD_DELIMITER        " "   //Delimiters for tokens
#define CLI_PROMPT              "> "   //The prompt for user input
#define CLI_NEW_LINE          "\r\n"   //New line thats used within cli

#define CLI_CMD_LIST_END  {0, 0, 0, 0} //Last entry for command list

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

/*Command configuration struct*/
typedef struct
{
	char command_name[CLI_MAX_LEN_CMD];     //holds command keyword
	cli_arg_type_t arg_type;                //holds the argument type

	union cmd_fptr                          //Pointer to your command function, multiple argument types handled
	{
		void(*cmd_void)(void);
		void(*cmd_int)(int);
		void(*cmd_float)(float);
		void(*cmd_str)(const char *);
	};

#if CLI_CMD_MAX_HELP_LENGTH > 0
	char help[CLI_CMD_MAX_HELP_LENGTH];     //Holds help string
#else
	uint8_t junk;
#endif
} cli_command_t;

/*CLI configuration struct*/
typedef	struct
{
	int16_t (*rx_byte_fptr)(void);       //function pointer for received byte return -1 for no data or >=0 for ascii data
	void (*tx_string_fprt)(const char*); //function pointer for transmit null terminated string
	cli_enable_t enable;                 //enables or disable cli
	cli_echo_enable_t echo_enable;       //enables or disables echo
	const cli_command_t *cmd_list;       //points list of commands
} cli_conf_t;


/**************************************************************************************************
*                                            PROTOTYPES
*************************************************^************************************************/
void cli_get_config_defaults(cli_conf_t *cli_conf);
void cli_init(cli_conf_t cli_conf);
void cli_task(void);
void cli_enable(cli_enable_t enable);
void cli_print(const char *null_term_str);
void cli_help_command(void);
int cli_strncpy(char *dest, size_t dest_size, const char *src, size_t src_size);
char * cli_strtok_r(char *s, const char *delim, char **save_ptr);


#endif /* CLI_H_ */
