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

#include <console.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32l5xx_hal.h"

typedef enum {
	USART_1,
	USART_2
}USART_MODULE;

typedef enum {
	STATUS_OK,
	STATUS_ERROR
}CONSOLE_STATUS;

typedef enum {
	CMD_NOT_RECEIVED,
	CMD_RECEIVED,
	CMD_FOUND,
	CMD_NOT_FOUND
}COMMAND_STATUS;

CONSOLE_STATUS      ConsoleDrv_Init(USART_TypeDef* eModule);
void                ConsoleDrv_Start();
void                ConsoleDrv_Putc(uint8_t Symbol);
void                ConsoleDrv_Puts(char *Message);
void                ConsoleDrv_Printf(char *Message, ...);
int                 ConsoleDrv_GetMessageLength(char *Message);
char*               ConsoleDrv_GetNextArgument(char *Message);
int                 ConsoleDrv_ConvertArgumentToDigit(char *Message);
void                ConsoleDrv_SetCommandStatus(COMMAND_STATUS eCmdStatus);
COMMAND_STATUS      ConsoleDrv_CheckCommandStatus();
uint8_t*            ConsoleDrv_GetReceivedCmd();
void                ConsoleDrv_OnCommandExecuted();
void                ConsoleDrv_ClearDrvBuffers();
UART_HandleTypeDef* ConsoleDrv_GetUartHandleTypeDef();


#endif /* Console_DRV_H_ */
