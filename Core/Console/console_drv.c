/*
 *  @file:   ConsoleDrv_drv.c
 *  @Author: Denislav Trifonov
 *  @Date:   10.11.2020
 *  @brief: This file contains UART driving functions:
 *          - Command interpreter
 *          - Transmit Char
 *          - Transmit String
 *          - C's standart function printf() implementation
 */
#include "console_drv.h"

/* ======================================================*/
/* Private variables                                     */
/* ======================================================*/

/* ASCII CODES DEFINITIONS */
#define FIFO_SIZE           1024

static const char g_cAsciiSymbolColon   = ':';
static const char g_cAsciiSymbolGreater = '>';
static const char g_cAsciiSymbolNL	    = '\n';
static const char g_cAsciiSymbolCR      = '\r';
static const char g_cAsciiSymbolSpace   = ' ';
static const char g_cAsciiSymbolDel	    = 0x7f;

static UART_HandleTypeDef g_UartDrv;

static uint8_t g_arrRxFifo[FIFO_SIZE];
static uint8_t g_arrTxFifo[FIFO_SIZE];

static uint8_t g_byNextRxSymbol         = 0;
static COMMAND_STATUS g_bCommandStatus  = CMD_NOT_RECEIVED;
static uint8_t g_bUartTxBusy            = 0;
static uint8_t g_bFifoFull	            = 0;
static uint16_t g_nRxFifoPushPointer    = 0;
static uint16_t g_nTxFifoPushPointer    = 0;
static uint16_t g_nTxFifoPullPointer    = 0;

static void ClearRxBuffer(void);

/* ======================================================*/
/* Public functions                                      */
/* ======================================================*/

/* ======================================================*/
CONSOLE_STATUS ConsoleDrv_Init(USART_TypeDef* eModule)
/* ======================================================*/
{
	CONSOLE_STATUS eStatus = STATUS_OK;

	g_UartDrv.Instance = eModule;
	g_UartDrv.Init.BaudRate = 115200;
	g_UartDrv.Init.WordLength = UART_WORDLENGTH_8B;
	g_UartDrv.Init.StopBits = UART_STOPBITS_1;
	g_UartDrv.Init.Parity = UART_PARITY_NONE;
	g_UartDrv.Init.Mode = UART_MODE_TX_RX;
	g_UartDrv.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	g_UartDrv.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	g_UartDrv.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	g_UartDrv.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	g_UartDrv.FifoMode = UART_FIFOMODE_DISABLE;

	if (HAL_UART_Init(&g_UartDrv) != HAL_OK)
	{
		eStatus = STATUS_ERROR;
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&g_UartDrv, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		eStatus = STATUS_ERROR;
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&g_UartDrv, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		eStatus = STATUS_ERROR;
	}
	if (HAL_UARTEx_DisableFifoMode(&g_UartDrv) != HAL_OK)
	{
		eStatus = STATUS_ERROR;
	}

	return eStatus;
}

/* ======================================================*/
void  ConsoleDrv_Start()
/* ======================================================*/
{
	HAL_UART_Receive_IT(&g_UartDrv, &g_byNextRxSymbol, 1);
}

/* Driving functions:
 * - Send Symbol
 * - Send String
 * - C's standart function printf() implementation
 */
/* ======================================================*/
void ConsoleDrv_Putc(uint8_t Symbol)
/* ======================================================*/
{
	// If fifo get full has to wait here ...
	while(g_bFifoFull)
	{}
	if(g_bUartTxBusy && !g_bFifoFull)
	{
		g_arrTxFifo[g_nTxFifoPushPointer++] = Symbol;
		if(((g_nTxFifoPushPointer) % FIFO_SIZE) == g_nTxFifoPullPointer)
		{
			g_bFifoFull = 1;
		}
		if(g_nTxFifoPushPointer == FIFO_SIZE)
			g_nTxFifoPushPointer = 0;
	}
	else
	{
		HAL_UART_Transmit_IT(&g_UartDrv, &Symbol, 1);
		g_bUartTxBusy = 1;
	}
}

/* ======================================================*/
void ConsoleDrv_Puts(char *Message)
/* ======================================================*/
{
	int symbol = 0;
	while(Message[symbol] != '\0')
	{
		ConsoleDrv_Putc(Message[symbol++]);
	}

	if (Message[symbol] == '\0')
	{
		symbol = 0;
	}
}


// Similar to C Standart Library Function printf()
/* ======================================================*/
void ConsoleDrv_Printf(char *Message, ...)
/* ======================================================*/
{
	int symbol = 0;
	va_list arguments;
	va_start(arguments, Message);
	int iArg = 0;
	char iStr[10];
	char chArg = 0;
	char *strArg = 0;
	float f32Arg = 0.0f;

	while(Message[symbol] != '\0')
	{
		if(Message[symbol] == '%')
		{
			symbol++;
			switch(Message[symbol])
			{
			case 'c':
				chArg = va_arg(arguments, int);
				ConsoleDrv_Putc(chArg);
				break;

			case 'd':
				iArg = va_arg(arguments, int);
				sprintf(iStr, "%d", iArg);
				ConsoleDrv_Puts(iStr);
				break;

			case 'x':
				iArg = va_arg(arguments, int);
				sprintf(iStr, "%x", iArg);
				ConsoleDrv_Puts(iStr);
				break;

			case 's':
				strArg = va_arg(arguments,char *);
				ConsoleDrv_Puts(strArg);
				break;

			case 'f':
				f32Arg = va_arg(arguments, double);
				sprintf(iStr, "%.2f", f32Arg);
				ConsoleDrv_Puts(iStr);
				break;

			default:
				ConsoleDrv_Putc(Message[--symbol]);
			}
			symbol++;
		}

		else
			ConsoleDrv_Putc(Message[symbol++]);
	}
	va_end(arguments);
}

/* ======================================================*/
int ConsoleDrv_GetMessageLength(char *Message)
/* ======================================================*/
{
	int length = 0;
	int i = 0;
	while(Message[i++] != '\0')
	{
		length ++;
	}
	return length;
}

/* ======================================================*/
char* ConsoleDrv(char *Message)
/* ======================================================*/
{
	char *StrPtr = NULL;
	int i = 0;

	while(Message[i] != '\0')
	{
		if(Message[i] == g_cAsciiSymbolSpace)
		{
			i++;
			StrPtr = &Message[i];
			break;
		}
		i++;
	}

	return StrPtr;
}

/* ======================================================*/
int ConsoleDrv_ConvertArgumentToDigit(char *Message)
/* ======================================================*/
{
	char StrPtr[10];
	int i = 0;
	int arg_num = 0;

	/* Check if the next symbol is a digit */
	while((Message[i] >= 0x30) && (Message[i] <= 0x39))
	{
		StrPtr[i] = Message[i];
		i++;
	}
	StrPtr[i] = '\0';

	/* Convert string to int using standard function from <stdlib.h> */
	arg_num = atoi(StrPtr);

	return arg_num;
}

/* ======================================================*/
void ConsoleDrv_SetCommandStatus(COMMAND_STATUS eCmdStatus)
/* ======================================================*/
{
	if (g_bCommandStatus != eCmdStatus)
	{
		g_bCommandStatus = eCmdStatus;
	}
}

/* ======================================================*/
COMMAND_STATUS ConsoleDrv_CheckCommandStatus()
/* ======================================================*/
{
	return g_bCommandStatus;
}

/* ======================================================*/
uint8_t* ConsoleDrv_GetReceivedCmd()
/* ======================================================*/
{
	return g_arrRxFifo;
}

/* ======================================================*/
void ConsoleDrv_OnCommandExecuted()
/* ======================================================*/
{
	g_bCommandStatus     = CMD_NOT_RECEIVED;
	ClearRxBuffer();
	ConsoleDrv_Puts("\r\n>");
}

/* ======================================================*/
void ConsoleDrv_ClearDrvBuffers()
/* ======================================================*/
{
	g_nTxFifoPushPointer = 0;
	g_nTxFifoPullPointer = 0;
}

/* ======================================================*/
UART_HandleTypeDef* ConsoleDrv_GetUartHandleTypeDef()
/* ======================================================*/
{
	return &g_UartDrv;
}

/* ======================================================*/
/* Private functions                                     */
/* ======================================================*/

/* ======================================================*/
void ClearRxBuffer(void)
/* ======================================================*/
{
	for(int i = 0; i < FIFO_SIZE; i++)
	{
		g_arrRxFifo[i] = 0;
	}
}

/* This function handles UART RX complete interrupt request. */
/* ======================================================*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
/* ======================================================*/
{
	if (g_byNextRxSymbol == g_cAsciiSymbolCR)
	{
		if(g_nRxFifoPushPointer)
		{
			g_nRxFifoPushPointer = 0;
			g_bCommandStatus = CMD_RECEIVED;
			HAL_UART_Transmit(&g_UartDrv, (uint8_t *)&g_cAsciiSymbolCR, 1, 10);
			HAL_UART_Transmit(&g_UartDrv, (uint8_t *)&g_cAsciiSymbolNL, 1, 10);
		}
		else
		{
			HAL_UART_Transmit(&g_UartDrv, (uint8_t *)&g_cAsciiSymbolCR, 1, 10);
			HAL_UART_Transmit(&g_UartDrv, (uint8_t *)&g_cAsciiSymbolNL, 1, 10);
			HAL_UART_Transmit(&g_UartDrv, (uint8_t *)&g_cAsciiSymbolGreater, 1, 10);
		}
		HAL_UART_Receive_IT(&g_UartDrv, &g_byNextRxSymbol, 1);
	}

	else if (g_byNextRxSymbol == g_cAsciiSymbolDel)
	{
		if(g_nRxFifoPushPointer)
		{
			HAL_UART_Transmit(&g_UartDrv, &g_byNextRxSymbol, 1, 10);
			g_nRxFifoPushPointer --;
		}
		HAL_UART_Receive_IT(&g_UartDrv, &g_byNextRxSymbol, 1);
	}

	else
	{
		HAL_UART_Transmit(&g_UartDrv, &g_byNextRxSymbol, 1, 10);
		g_arrRxFifo[g_nRxFifoPushPointer++] = g_byNextRxSymbol;
		HAL_UART_Receive_IT(&g_UartDrv, &g_byNextRxSymbol, 1);
	}

	if (g_nRxFifoPushPointer == FIFO_SIZE)
	{
		g_nRxFifoPushPointer--;
	}
}

/* This function handles UART TX complete interrupt request. */
/* ======================================================*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
/* ======================================================*/
{
	if(g_bUartTxBusy)
	{
		if(g_nTxFifoPullPointer != g_nTxFifoPushPointer || g_bFifoFull)
		{
			HAL_UART_Transmit_IT(&g_UartDrv, (g_arrTxFifo + g_nTxFifoPullPointer++), 1);
			g_bFifoFull = 0;
			if(g_nTxFifoPullPointer == FIFO_SIZE)
				g_nTxFifoPullPointer = 0;
		}
		else
		{
			g_bUartTxBusy = 0;
		}
	}
}
/* ======================================================*/
