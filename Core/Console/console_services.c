/*
 *  @file:   console_services.c
 *  @Author: Denislav Trifonov
 *  @Date:   10.11.2020
 *  @brief: This file contains UART console commands
 */


#include <console_services.h>


static void Service_Help(uint8_t *RxBuff);
static void Service_Unknown(uint8_t *RxBuff);

/* ====================================================== */
/* Available commands  */
/* ====================================================== */
const char*  UartCommands[] = {
		"HELP",
		""
};
/* ====================================================== */
/* Depending on the written command the corresponding function is executed. */
void (*FuncPtr[])(uint8_t *RxData) =
{
		&Service_Help,
		&Service_Unknown
};
/* ====================================================== */
void Service_Help(uint8_t *RxBuff)
/* ====================================================== */
{
	Console_Puts("Available commands:\r\n");
	Console_Puts("HELP - This information\r\n:");
}

/* ====================================================== */
void Service_Unknown(uint8_t *RxBuff)
/* ====================================================== */
{
	Console_Puts("Unknown Command!\r\n");
}
/* ======================================================*/
