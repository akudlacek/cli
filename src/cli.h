/*
* cli.h
*
*  Created on: Aug 4, 2016
*      Author: akudlacek
*/

#ifndef CLI_H_
#define CLI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>


/**************************************************************************************************
*                                             DEFINES
*************************************************^************************************************/
/* If you want to override these symbols without modifying source code you will have to add a
 * symbol definition in the preprocessor settings.
 * In VS 2019        put DEFINE_OF_INTEREST=0 in [Configuration Properties->C/C++->Preprocessor->Preprocessor Definitions]
 * In Atmel Studio 7 put DEFINE_OF_INTEREST=0 in [Properties->ARM/GNU C Compiler->Symbols->Defined symbols (-D)]
 * In Code Blocks    put DEFINE_OF_INTEREST=0 in [Properties->Project's build options...->Compiler settings->#defines]
 */

#ifndef CLI_MAX_LEN_CMD
#define CLI_MAX_LEN_CMD          10    //Max length cmd name, including null terminator
#endif

#ifndef CLI_MAX_LEN_BUFF
#define CLI_MAX_LEN_BUFF         256   //Max total length of command name and arguments/string input, including null terminator
#endif

#ifndef CLI_MAX_LEN_HELP_DESC
#define CLI_MAX_LEN_HELP_DESC    64    //if this is zero, there will be no help description
#endif

#ifndef CLI_CMD_DELIMITER
#define CLI_CMD_DELIMITER        " "   //Delimiters for tokens
#endif

#ifndef CLI_PROMPT
#define CLI_PROMPT              "> "   //The prompt for user input
#endif

#ifndef CLI_NEW_LINE
#define CLI_NEW_LINE          "\r\n"   //New line thats used within cli
#endif

/*HELP*/
#if CLI_MAX_LEN_HELP_DESC > 0
#define HELP(x)  (x)
#else
#define HELP(x)	 {0} //need braces or will cause warning under GNU
#endif

/*CLI return status*/
typedef enum cli_return_t
{
	CLI_SUCESS,
	CLI_FAIL_NULL_PARAM,
	CLI_FAIL_OUT_OF_CMD_SLOTS,
	CLI_FAIL_CMD_NAME_LEN,
	CLI_FAIL_CMD_NAME_ILLEGAL_CHAR,
	CLI_FAIL_UNSUPPORTED_ARG_TYPE
} cli_return_t;

/*CLI enable disable enum*/
typedef enum cli_enable_t
{
	CLI_DISABLED,
	CLI_ENABLED
} cli_enable_t;

/*CLI echo enable disable enum*/
typedef enum cli_echo_enable_t
{
	CLI_ECHO_DISABLED,
	CLI_ECHO_ENABLED
} cli_echo_enable_t;

/*Cli argument type*/
typedef enum cli_arg_type_t
{
	CLI_VOID,
	CLI_INT,
	CLI_UINT8,
	CLI_ULINT,
	CLI_FLOAT,
	CLI_STRING
} cli_arg_type_t;

/*Command configuration struct*/
typedef struct cli_command_t
{
	char command_name[CLI_MAX_LEN_CMD];     //holds command keyword
	
	cli_arg_type_t arg_type;                //holds the argument type

	union                                   //Pointer to your command function, multiple argument types handled
	{
		void(*cmd_void)(void);
		void(*cmd_uint8)(uint8_t);
		void(*cmd_int)(int);
		void(*cmd_ulint)(unsigned long);
		void(*cmd_float)(float);
		void(*cmd_str)(const char * const);
	} fptr;

	char help[CLI_MAX_LEN_HELP_DESC + 1];     //Holds help string
} cli_command_t;

/*Function pointer macro for helping keep table clean
 *also puts function arg type in table  
 *fptr is a union it needs to be explicitly declared*/
#define CLI_VOID_FPTR(fptr_in)    CLI_VOID,   .fptr = {.cmd_void  = fptr_in}
#define CLI_INT_FPTR(fptr_in)     CLI_INT,    .fptr = {.cmd_int   = fptr_in}
#define CLI_UINT8_FPTR(fptr_in)   CLI_UINT8,  .fptr = {.cmd_uint8 = fptr_in}
#define CLI_ULINT_FPTR(fptr_in)   CLI_ULINT,  .fptr = {.cmd_ulint = fptr_in}
#define CLI_FLOAT_FPTR(fptr_in)   CLI_FLOAT,  .fptr = {.cmd_float = fptr_in}
#define CLI_STRING_FPTR(fptr_in)  CLI_STRING, .fptr = {.cmd_str   = fptr_in}

/*End of command list*/
#define CLI_CMD_LIST_END {"", CLI_VOID_FPTR(NULL), HELP("")} //Last entry for command list

/*Help command entry macro*/
#define CLI_HELP_CMD_LIST_ENTRY {"help", CLI_VOID_FPTR(cli_help_command), HELP("Prints a list of available commands")}

/*CLI configuration struct*/
typedef	struct cli_conf_t
{
	int16_t (*rx_byte_fptr)(void);              //function pointer for received byte return -1 for no data or >=0 for ascii data
	void (*tx_string_fprt)(const char * const); //function pointer for transmit null terminated string
	cli_enable_t enable;                        //enables or disable cli
	cli_echo_enable_t echo_enable;              //enables or disables echo
	const cli_command_t *cmd_list;              //points list of commands
} cli_conf_t;


/**************************************************************************************************
*                                            PROTOTYPES
*************************************************^************************************************/
void   cli_get_config_defaults(cli_conf_t * const cli_conf);
void   cli_init               (const cli_conf_t cli_conf);
void   cli_task               (void);
void   cli_enable             (const cli_enable_t enable);
void   cli_print              (const char * const null_term_str);
void   cli_print_nl           (const char* const null_term_str);
void   cli_help_command       (void);
int    cli_strncpy            (char * const dest, const size_t dest_size, const char * const src, const size_t src_size);
char * cli_strtok_r           (char *s, const char * const delim, char ** const save_ptr);
void   cli_rx_buf_clr         (void);

#ifdef __cplusplus
}
#endif

#endif /* CLI_H_ */
