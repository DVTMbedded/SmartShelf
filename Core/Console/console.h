/*
 *  @file:   console_services.c
 *  @Author: Denislav Trifonov
 *  @Date:   10.11.2020
 *  @brief: This file contains UART console commands
 */

#ifndef UART_COMMANDS_H_
#define UART_COMMANDS_H_

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "stm32l5xx_hal.h"
#include "console_drv.h"

void  Console_Init();
void  Console_Exec(void);


#endif /* UART_COMMANDS_H_ */
