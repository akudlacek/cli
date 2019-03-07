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


/**************************************************************************************************
*                                         LOCAL PROTOTYPES
*************************************************^************************************************/
static void cmd_line_handler(uint32_t cmd_num, char *arg_str);
static void cmd_line_init(void);

int16_t rx_byte_wrapper(void);
void tx_string_wrapper(char *string_ptr);


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
*  \brief CLI command function
*
*  \note
******************************************************************************/
static void cmd_line_handler(uint32_t cmd_num, char *arg_str)
{
	float arg_num_f;
	int arg_num_i;

	if(arg_str != NULL)
	{
		/*Convert argument string to float*/
		arg_num_f = (float)atof(arg_str);

		/*Convert argument string to int*/
		arg_num_i = atoi(arg_str);
	}

	/*NOTE: The order of this is set by the order the commands are added in command_line_init*/
	switch (cmd_num)
	{
	case 0:
		//used by help func
		break;

	case 1:
		cli_print("CMD 1 was entered\r\n");
		break;

	case 2:
		cli_print("CMD 2 was entered\r\n");
		cli_print(arg_str);
		cli_print("\r\n");
		break;

	default:
		cli_print("NO CMD FOUND\r\n");
		break;
	}
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
	cli_add_command("cmd1", CLI_SINGLE_WORD, cmd_line_handler);   //CMD 1
	cli_add_command("cmd2", CLI_STRING,      cmd_line_handler);   //CMD 2

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
void tx_string_wrapper(char *string_ptr)
{
	printf("%s", string_ptr);
}
