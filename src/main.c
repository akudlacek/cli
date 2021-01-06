/*EXAMPLE MAIN*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "cli.h"
#include "cli_test.h"


/**************************************************************************************************
*                                             DEFINES
*************************************************^************************************************/


/**************************************************************************************************
*                                         LOCAL PROTOTYPES
*************************************************^************************************************/
static void void_func_ex(void);
static void int_func_ex(int arg);
static void uint8_func_ex(uint8_t arg);
static void ulint_func_ex(unsigned long arg);
static void float_func_ex(float arg);
static void str_func_ex(const char *arg);
static void duck_func_ex(void);
static void cli_error_test(void);

static void cmd_line_init(void);

static int16_t rx_byte_wrapper(void);
static void tx_string_wrapper(const char *string_ptr);
static int16_t inj_rx_byte_wrapper(void);


/**************************************************************************************************
*                                            VARIABLES
*************************************************^************************************************/
static char tmp_str[100];


/* Available function types
	CLI_VOID,
	CLI_INT,
	CLI_UINT8,
	CLI_ULINT,
	CLI_FLOAT,
	CLI_STRING
*/
static const cli_command_t cli_cmd_list[] =
{
	//cmd name, fptr, help string
	CLI_HELP_CMD_LIST_ENTRY,

	{"void",   CLI_VOID_FPTR(void_func_ex),      HELP("void just runs the function")},
	{"int",    CLI_INT_FPTR(int_func_ex),        HELP("e.g. int -4")},
	{"uint8",  CLI_UINT8_FPTR(uint8_func_ex),    HELP("e.g. uint8_t 200")},
	{"ulint",  CLI_ULINT_FPTR(ulint_func_ex),    HELP("e.g. unsigned long 4294967295")},
	{"float",  CLI_FLOAT_FPTR(float_func_ex),    HELP("e.g. float 9.87654")},
	{"str",    CLI_STRING_FPTR(str_func_ex),     HELP("type str followed by your string")},
	{"duck",   CLI_VOID_FPTR(duck_func_ex),      HELP("prints a duck")},
	{"errtst", CLI_VOID_FPTR(cli_error_test),    HELP("test all error messages")},
	{"null",   CLI_VOID_FPTR(NULL),              HELP("null function for testing")},

	CLI_CMD_LIST_END                                             // must be LAST
};

static int16_t inj_rx_byte = 0;


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

	//unit test
	printf("cli_strncpy %s\r\n", (tst_cli_strncpy() == CLI_TEST_FAIL ? "FAIL" : "PASS"));
	

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
static void void_func_ex(void)
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
	snprintf(tmp_str, sizeof(tmp_str), "you put int: %i\r\n", arg);
	cli_print(tmp_str);
}

/******************************************************************************
*  \brief
*
*  \note
******************************************************************************/
static void uint8_func_ex(uint8_t arg)
{
	cli_print("uint8_func_ex was entered\r\n");
	snprintf(tmp_str, sizeof(tmp_str), "you put uint8_t: %u\r\n", arg);
	cli_print(tmp_str);
}

/******************************************************************************
*  \brief
*
*  \note
******************************************************************************/
static void ulint_func_ex(unsigned long arg)
{
	cli_print("ulint_func_ex was entered\r\n");
	snprintf(tmp_str, sizeof(tmp_str), "you put unsigned long: %lu\r\n", arg);
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
*  \brief
*
*  \note
******************************************************************************/
static void duck_func_ex(void)
{
	cli_print(
				"        ,----,\r\n"
				"   ___.`      `,\r\n"
				"   `===  D     :\r\n"
				"     `'.      .'\r\n"
				"        )    (                   ,\r\n"
				"       /      \\_________________/|\r\n"
				"      /                          |\r\n"
				"     |                           ;\r\n"
				"     |               _____       /\r\n"
				"     |      \\       ______7    ,'\r\n"
				"     |       \\    ______7     /\r\n"
				"      \\       `-,____7      ,'   jgs\r\n"
				"^~^~^~^`\\                  /~^~^~^~^\r\n"
				"  ~^~^~^ `----------------' ~^~^~^\r\n"
				" ~^~^~^~^~^^~^~^~^~^~^~^~^~^~^~^~\r\n"
			);
}

/******************************************************************************
*  \brief
*
*  \note
******************************************************************************/
static void cli_error_test(void)
{
	uint32_t i;
	char null_cmd[] = "null\r";
	
	cli_conf_t cli_conf;

	//Init but inject rx byte
	cli_get_config_defaults(&cli_conf);
	cli_conf.rx_byte_fptr = inj_rx_byte_wrapper; //changed
	cli_conf.tx_string_fprt = tx_string_wrapper;
	cli_conf.enable = CLI_ENABLED;
	cli_conf.echo_enable = CLI_ECHO_ENABLED; //changed
	cli_conf.cmd_list = cli_cmd_list;
	cli_init(cli_conf);

	/*Command length error test*/
	cli_print("The following should display a command length error");
	inj_rx_byte = 's'; //set rx byte to some garbage
	//run task and rx more than command buffer can hold
	for(i = 0; i < CLI_MAX_LEN_BUFF; i++)
	{
		cli_task();
	}
	cli_rx_buf_clr();

	/*No command found error test*/
	cli_print("The following should display a no command found error");
	cli_task();
	inj_rx_byte = '\r';
	cli_task();

	/*Null function pointer error test*/
	cli_print("The following should display a null function pointer error");
	for(i = 0; i < sizeof(null_cmd); i++)
	{
		inj_rx_byte = null_cmd[i];
		cli_task();
	}

	//return to normal init
	cli_print("Error message test complete");
	cmd_line_init();
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
static int16_t rx_byte_wrapper(void)
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
static void tx_string_wrapper(const char *string_ptr)
{
	printf("%s", string_ptr);
}

/******************************************************************************
*  \brief Injection RX BYTE
*
*  \note use inj_rx_byte variable to inject
******************************************************************************/
static int16_t inj_rx_byte_wrapper(void)
{
	return inj_rx_byte;
}
