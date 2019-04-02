/*
* cli_test.c
*
*  Created on: Apr 2, 2019
*      Author: akudlacek
*/


#include "cli_test.h"

#include "cli.h"

#include <string.h>


/**************************************************************************************************
*                                             DEFINES
*************************************************^************************************************/


/**************************************************************************************************
*                                            VARIABLES
*************************************************^************************************************/


/**************************************************************************************************
*                                         LOCAL PROTOTYPES
*************************************************^************************************************/


/**************************************************************************************************
*                                            FUNCTIONS
*************************************************^************************************************/
/******************************************************************************
*  \brief test cli_strncpy
*
*  \note
******************************************************************************/
cli_test_result_t tst_cli_strncpy(void)
{
	char a_str[15] = "This is a test";
	char b_str[15];
	char cont_str[30];

	//check for null pointer
	if(cli_strncpy(0, sizeof(b_str), a_str, sizeof(a_str)) != -1)
		return CLI_TEST_FAIL;
	if(cli_strncpy(b_str, sizeof(b_str), 0, sizeof(a_str)) != -1)
		return CLI_TEST_FAIL;

	//check for zero size
	if(cli_strncpy(b_str, 0, a_str, sizeof(a_str)) != -1)
		return CLI_TEST_FAIL;
	if(cli_strncpy(b_str, sizeof(b_str), a_str, 0) != -1)
		return CLI_TEST_FAIL;

	//check for overlapping memory
	//A----B
	//C----D
	if(cli_strncpy(cont_str, sizeof(cont_str), cont_str, sizeof(cont_str)) != -1)
		return CLI_TEST_FAIL;
	//A----B
	//  C----D
	if (cli_strncpy(cont_str, 10, &cont_str[5], 10) != -1)
		return CLI_TEST_FAIL;
	//  A----B
	//C----D
	if(cli_strncpy(&cont_str[5], 10, cont_str, 10) != -1)
		return CLI_TEST_FAIL;
	//A----B
	//     C----D
	if(cli_strncpy(cont_str, 10, &cont_str[9], 10) != -1)
		return CLI_TEST_FAIL;
	//     A----B
	//C----D
	if(cli_strncpy(&cont_str[9], 10, cont_str, 10) != -1)
		return CLI_TEST_FAIL;
	//A----B
	//       C----D
	if(cli_strncpy(cont_str, 10, &cont_str[10], 10) != 0)
		return CLI_TEST_FAIL;
	//       A----B
	//C----D
	if(cli_strncpy(&cont_str[10], 10, cont_str, 10) != 0)
		return CLI_TEST_FAIL;

	//check non null term source, will truncate
	memset(cont_str, 'a', sizeof(cont_str));
	if(cli_strncpy(cont_str, sizeof(cont_str), a_str, 5) != 0)
		return CLI_TEST_FAIL;
	if(cont_str[5] != 0) //make sure destination is null terminated
		return CLI_TEST_FAIL;
	if(!(strnlen(cont_str, sizeof(cont_str)) < strnlen(a_str, sizeof(a_str)))) //make sure string was truncated
		return CLI_TEST_FAIL;
	if(strncmp(cont_str, a_str, 5) != 0) //make sure string matches
		return CLI_TEST_FAIL;

	//check src smaller then dest
	memset(cont_str, 'b', sizeof(cont_str));
	if(cli_strncpy(cont_str, sizeof(cont_str), a_str, sizeof(a_str)) != 0)
		return CLI_TEST_FAIL;
	if(cont_str[sizeof(a_str) - 1] != 0) //make sure destination is null terminated
		return CLI_TEST_FAIL;
	if((strnlen(cont_str, sizeof(cont_str)) != strnlen(a_str, sizeof(a_str)))) //make sure string was not truncated
		return CLI_TEST_FAIL;
	if(strncmp(cont_str, a_str, sizeof(a_str)) != 0) //make sure string matches
		return CLI_TEST_FAIL;

	//check dest smaller then src
	memset(cont_str, 'c', sizeof(cont_str));
	if(cli_strncpy(cont_str, 10, a_str, sizeof(a_str)) != 0)
		return CLI_TEST_FAIL;
	if(cont_str[9] != 0) //make sure destination is null terminated
		return CLI_TEST_FAIL;
	if(!(strnlen(cont_str, sizeof(cont_str)) < strnlen(a_str, sizeof(a_str)))) //make sure string was truncated
		return CLI_TEST_FAIL;
	if(strncmp(cont_str, a_str, 9) != 0) //make sure string matches
		return CLI_TEST_FAIL;


		return CLI_TEST_PASS;
}


/**************************************************************************************************
*                                         LOCAL FUNCTIONS
*************************************************^************************************************/
