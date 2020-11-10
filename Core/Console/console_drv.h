/*
 *  @file:   console_drv.h
 *  @Author: Denislav Trifonov
 *  @Date:   10.11.2020
 *  @brief: This file contains UART driving functions:
 *          - Command interpreter
 *          - Transmit Char
 *          - Transmit String
 *          - C's standart function printf() implementation
 */

#ifndef CONSOLE_DRV_H_
#define CONSOLE_DRV_H_

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

#include "stm32l5xx_hal.h"
#include "console_services.h"


void  Console_Init(UART_HandleTypeDef *pUart);
void  Console_Process(void);
void  Console_Putc(uint8_t Symbol);
void  Console_Puts(char *Message);
void  Console_Printf(char *Message, ...);
int   Console_GetMessageLength(char *Message);
char* Console_GetNextArgument(char *Message);
int   Console_ConvertArgumentToDigit(char *Message);







#endif /* Console_DRV_H_ */
