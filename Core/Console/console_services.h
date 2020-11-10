/*
 * uart_commands.h
 *
 *  Created on: 1.08.2018 �.
 *      Author: trifonovd
 */

#ifndef UART_COMMANDS_H_
#define UART_COMMANDS_H_

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "stm32l5xx_hal.h"
#include "console_drv.h"

void Function_Help(uint8_t *RxBuff);
void Function_Unknown(uint8_t *RxBuff);


extern void (*FuncPtr[])(uint8_t *RxData);
extern const char*  UartCommands[];


#endif /* UART_COMMANDS_H_ */
