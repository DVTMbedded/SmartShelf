/*
 * Console_commands.c
 *
 *  Created on: 13.07.2018 �.
 *      Author: trifonovd
 */

/* User implemented commands */


#include <console_services.h>



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
		&Function_Help,
		&Function_Unknown
};
/* ====================================================== */
void Function_Help(uint8_t *RxBuff)
/* ====================================================== */
{
	Console_Puts("	Available commands:\r\n");
	Console_Puts("	HELP - This information\r\n:");
}

/* ====================================================== */
void Function_Unknown(uint8_t *RxBuff)
/* ====================================================== */
{
	Console_Puts("Unknown Command!\r\n");
}
/* ======================================================*/
