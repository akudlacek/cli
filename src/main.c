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
*                                            VARIABLES
*************************************************^************************************************/
char tmp_str[100];


/**************************************************************************************************
*                                         LOCAL PROTOTYPES
*************************************************^************************************************/
void void_func_ex(void);
void int_func_ex(int arg);
void float_func_ex(float arg);
void str_func_ex(const char *arg);
static void cmd_line_init(void);

int16_t rx_byte_wrapper(void);
void tx_string_wrapper(const char *string_ptr);


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
void int_func_ex(int arg)
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
void float_func_ex(float arg)
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
void str_func_ex(const char *arg)
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

	/*NOTE: max num of commands is in cli.h on a define*/
	/*Add built in 'help' command*/
	cli_add_help_command();                      //CMD 0

	/*Add custom commands*/
	cli_add_command("void",  CLI_NO_ARG, void_func_ex);
	cli_add_command("int",   CLI_INT,    int_func_ex);
	cli_add_command("float", CLI_FLOAT,  float_func_ex);
	cli_add_command("str",   CLI_STRING, str_func_ex);

	/*Initialize the CLI driver*/
	cli_get_config_defaults(&cli_conf);
	cli_conf.rx_byte_fptr   = rx_byte_wrapper;
	cli_conf.tx_string_fprt = tx_string_wrapper;
	cli_conf.enable         = CLI_ENABLED;
	cli_conf.echo_enable    = CLI_ECHO_DISABLED;
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
