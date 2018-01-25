/*
 * example.c
 *
 * Created: 12/12/2017 2:26:33 PM
 *  Author: akudlacek
 */ 


#include "cli.h"


/******************************************************************************
* Local Prototypes
******************************************************************************/
static void cli_hello_world(char *arg, int32_t num);
static void init_command_line(void);
static void serial_hello_message(void);


/******************************************************************************
*  \brief
*
*  \note
*
*  \param[out] none
*  \param[in]  none
******************************************************************************/
static void cli_hello_world(char *arg, int32_t num)
{
	serial_hello_message();
}

/******************************************************************************
*  \brief
*
*  \note
*
*  \param[out] none
*  \param[in]  none
******************************************************************************/
static void init_command_line(void)
{
	cli_add_command("hello", NONE, cli_hello_world, NULL, NULL);
	
	cli_add_help_command();   //automatically add help command
	
	cli_init(drvr_usart_rx_byte, drvr_usart_tx_str);
}

/******************************************************************************
*  \brief
*
*  \note
*
*  \param[out] none
*  \param[in]  none
******************************************************************************/
static void serial_hello_message(void)
{
	drvr_usart_tx_str("\n\rhello world!");

	snprintf(strn, 200,
	"\n\rSerial Number: %08lX %08lX %08lX %08lX \n\r"
	"REV %c\n\r",
	SAMC21_SN_WORD3,
	SAMC21_SN_WORD2,
	SAMC21_SN_WORD1,
	SAMC21_SN_WORD0,
	DSU->DID.bit.REVISION+'A');
	drvr_usart_tx_str(strn);
}


/******************************************************************************
*  \brief
*
*  \note
*
*  \param[out] none
*  \param[in]  none
******************************************************************************/
void tst_init(void)
{
	drvr_usart_init();
	
	init_command_line();
}


/******************************************************************************
*  \brief
*
*  \note
*
*  \param[out] none
*  \param[in]  none
******************************************************************************/
inline void tst_task(void)
{
	cli_process_task();
}
