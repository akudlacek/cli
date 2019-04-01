/*EXAMPLE MAIN*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "cli.h"

/**************************************************************************************************
*                                             DEFINES
*************************************************^************************************************/


/**************************************************************************************************
*                                         LOCAL PROTOTYPES
*************************************************^************************************************/
static void void_func_ex(void);
static void int_func_ex(int arg);
static void float_func_ex(float arg);
static void str_func_ex(const char *arg);

static void cmd_line_init(void);

int16_t rx_byte_wrapper(void);
void tx_string_wrapper(const char *string_ptr);


/**************************************************************************************************
*                                            VARIABLES
*************************************************^************************************************/
char tmp_str[100];

static const cli_command_t cli_cmd_list[] =
{
	//cmd name, argument type, function pointer, help string
	{"help",  CLI_NO_ARG, .cmd_void = cli_help_command,  HELP("Lists the commands available")},
	{"void",  CLI_NO_ARG, .cmd_void = void_func_ex,      HELP("void just runs the function")},
	{"int",   CLI_INT,    .cmd_int = int_func_ex,        HELP("e.g. int -4")},
	{"float", CLI_FLOAT,  .cmd_float = float_func_ex,    HELP("e.g. float 9.87654")},
	{"str",   CLI_STRING, .cmd_str = str_func_ex,        HELP("type str followed by your string")},
	CLI_CMD_LIST_END                                             // must be LAST
};


/**************************************************************************************************
*                                            FUNCTIONS
*************************************************^************************************************/
/******************************************************************************
*  \brief MAIN
*
*  \note
******************************************************************************/
int main(void)
{
	cmd_line_init();

	printf("hello\r\n");

	while(1)
	{
		cli_task();
	}

	return 0;
}


/**************************************************************************************************
*                                         LOCAL FUNCTIONS
*************************************************^************************************************/
/******************************************************************************
*  \brief
*
*  \note
******************************************************************************/
void void_func_ex(void)
{
	cli_print("void_func_ex was entered\r\n");
	snprintf(tmp_str, sizeof(tmp_str), "this is void func\r\n");
	cli_print(tmp_str);
}

/******************************************************************************
*  \brief
*
*  \note
******************************************************************************/
static void int_func_ex(int arg)
{
	cli_print("int_func_ex was entered\r\n");
	snprintf(tmp_str, sizeof(tmp_str), "you put int: %d\r\n", arg);
	cli_print(tmp_str);
}

/******************************************************************************
*  \brief
*
*  \note
******************************************************************************/
static void float_func_ex(float arg)
{
	cli_print("float_func_ex was entered\r\n");
	snprintf(tmp_str, sizeof(tmp_str), "you put float: %f\r\n", arg);
	cli_print(tmp_str);
}

/******************************************************************************
*  \brief
*
*  \note
******************************************************************************/
static void str_func_ex(const char *arg)
{
	cli_print("str_func_ex was entered\r\n");
	snprintf(tmp_str, sizeof(tmp_str), "you put string: %s\r\n", arg);
	cli_print(arg);
	cli_print("\r\n");
}

/******************************************************************************
*  \brief Init all commands to be used in CLI
*
*  \note
******************************************************************************/
static void cmd_line_init(void)
{
	cli_conf_t cli_conf;

	/*Initialize the CLI driver*/
	cli_get_config_defaults(&cli_conf);

	cli_conf.rx_byte_fptr   = rx_byte_wrapper;
	cli_conf.tx_string_fprt = tx_string_wrapper;
	cli_conf.enable         = CLI_ENABLED;
	cli_conf.echo_enable    = CLI_ECHO_DISABLED;
	cli_conf.cmd_list       = cli_cmd_list;

	cli_init(cli_conf);
}

/******************************************************************************
*  \brief RX BYTE
*
*  \note return -1 for no data or >=0 for ascii data
******************************************************************************/
int16_t rx_byte_wrapper(void)
{
	int rx_byte;

	rx_byte = getchar();

	if(rx_byte == EOF)
	{
		return -1;
	}
	else
	{
		return (int16_t)rx_byte;
	}
}

/******************************************************************************
*  \brief TX STRING
*
*  \note transmit null terminated string
******************************************************************************/
void tx_string_wrapper(const char *string_ptr)
{
	printf("%s", string_ptr);
}