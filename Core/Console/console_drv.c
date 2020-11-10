/*
 *  @file:   console_drv.c
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
#define FIFO_SIZE			(1024)

/* ASCII CODES DEFINITIONS */
#define ASCII_CODE_CR		(0x0D)
#define ASCII_CODE_PP		(0x3A)
#define ASCII_CODE_NL		(0x0A)
#define ASCII_CODE_SP		(0x20)
#define ASCII_CODE_BS		(0x08)
#define ASCII_CODE_DEL		(0x7F)
#define	ASCII_CODE_ESC		(0x1B)
#define ASCII_CODE_SC		(0x3B)

static UART_HandleTypeDef* g_pUartDrv;

static uint8_t Rx_Fifo[FIFO_SIZE];
static uint8_t Tx_Fifo[FIFO_SIZE];

static uint8_t Rx_Symbol = 0;

static uint8_t flCommandReceived = 0;
static uint8_t flCommandFound 	  = 0;

static uint8_t flUartTxBusy = 0;
static uint8_t flFifoFull	 = 0;

static uint16_t Rx_Fifo_Push = 0;
static uint16_t Tx_Fifo_Push = 0;
static uint16_t Tx_Fifo_Pull = 0;

static uint8_t ascii_douple_point 	 = ASCII_CODE_PP;
static uint8_t ascii_new_line	 	 = ASCII_CODE_NL;
static uint8_t ascii_carriage_return = ASCII_CODE_CR;

static void  ClearRxBuffer(void);
static void  ClearTxBuffer(void);

/* ======================================================*/
/* Public functions                                      */
/* ======================================================*/

/* ======================================================*/
void Console_Init(UART_HandleTypeDef *pUart)
/* ======================================================*/
{
	g_pUartDrv = pUart;
	HAL_UART_Receive_IT(g_pUartDrv, &Rx_Symbol, 1);
}

/* ======================================================*/
void Console_Process(void)
/* ======================================================*/
{
	int idx = 0;
	if(flCommandReceived)
	{
		Tx_Fifo_Push = 0;
		Tx_Fifo_Pull = 0;
		flCommandFound = 0;
		flCommandReceived = 0;

		while(*(UartCommands[idx]) != '\0')
		{
			if(!(strncmp(UartCommands[idx],(char *)Rx_Fifo, 4)))
			{
				flCommandFound = 1;
				(*FuncPtr[idx])(Rx_Fifo);
				break;
			}
			idx ++;
		}
		if (!flCommandFound)
		{
			(*FuncPtr[idx])(Rx_Fifo);
		}
		idx = 0;

		ClearRxBuffer();
		Console_Puts("\r\n:");
	}
}

/* Driving functions:
 * - Send Symbol
 * - Send String
 * - C's standart function printf() implementation */
/* ======================================================*/
void Console_Putc(uint8_t Symbol)
/* ======================================================*/
{
	// If fifo get full has to wait here ...
	while(flFifoFull)
	{}
	if(flUartTxBusy && !flFifoFull)
	{
		Tx_Fifo[Tx_Fifo_Push++] = Symbol;
		if(((Tx_Fifo_Push) % FIFO_SIZE) == Tx_Fifo_Pull)
		{
			flFifoFull = 1;
		}
		if(Tx_Fifo_Push == FIFO_SIZE)
			Tx_Fifo_Push = 0;
	}
	else
	{
		//		Uart_Transmit_IT(USART3, &Symbol);
		HAL_UART_Transmit_IT(g_pUartDrv, &Symbol, 1);
		flUartTxBusy = 1;
	}
}

/* ======================================================*/
void Console_Puts(char *Message)
/* ======================================================*/
{
	int symbol = 0;
	while(Message[symbol] != '\0')
	{
		Console_Putc(Message[symbol++]);
	}
	//	Console_Putc(ASCII_CODE_PP);
}


// Similar to C Standart Library Function printf()
/* ======================================================*/
void Console_Printf(char *Message, ...)
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
				Console_Putc(chArg);
				break;

			case 'd':
				iArg = va_arg(arguments, int);
				sprintf(iStr, "%d", iArg);
				Console_Puts(iStr);
				break;

			case 'x':
				iArg = va_arg(arguments, int);
				sprintf(iStr, "%x", iArg);
				Console_Puts(iStr);
				break;

			case 's':
				strArg = va_arg(arguments,char *);
				Console_Puts(strArg);
				break;

			case 'f':
				f32Arg = va_arg(arguments, double);
				sprintf(iStr, "%.2f", f32Arg);
				Console_Puts(iStr);
				break;

			default:
				Console_Putc(Message[--symbol]);
			}
			symbol++;
		}

		else
			Console_Putc(Message[symbol++]);
	}
	va_end(arguments);
}

/* ======================================================*/
int Console_GetMessageLength(char *Message)
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
char* Console_GetNextArgument(char *Message)
/* ======================================================*/
{
	char *StrPtr = NULL;
	int i = 0;

	while(Message[i] != '\0')
	{
		if(Message[i] == ASCII_CODE_SP)
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
int Console_ConvertArgumentToDigit(char *Message)
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
/* Private functions                                     */
/* ======================================================*/

/* ======================================================*/
void ClearRxBuffer(void)
/* ======================================================*/
{
	for(int i = 0; i < FIFO_SIZE; i++)
	{
		Rx_Fifo[i] = 0;
	}
}

/* ======================================================*/
void ClearTxBuffer(void)
/* ======================================================*/
{
	Tx_Fifo_Pull = 0;
	Tx_Fifo_Push = 0;
	for(int i = 0; i < FIFO_SIZE; i++)
	{
		Tx_Fifo[i] = 0;
	}
}


/* This function handles UART RX complete interrupt request. */
/* ======================================================*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
/* ======================================================*/
{
	switch(Rx_Symbol)
	{
	case ASCII_CODE_CR:
		if(Rx_Fifo_Push)
		{
			Rx_Fifo_Push = 0;
			flCommandReceived = 1;
			HAL_UART_Transmit(g_pUartDrv, &ascii_carriage_return, 1, 10);
			HAL_UART_Transmit(g_pUartDrv, &ascii_new_line, 1, 10);
		}
		else
		{
			HAL_UART_Transmit(g_pUartDrv, &ascii_carriage_return, 1, 10);
			HAL_UART_Transmit(g_pUartDrv, &ascii_new_line, 1, 10);
			HAL_UART_Transmit(g_pUartDrv, &ascii_douple_point, 1, 10);
		}
		HAL_UART_Receive_IT(g_pUartDrv, &Rx_Symbol, 1);
		break;

	case ASCII_CODE_NL:
		if(Rx_Fifo_Push)
		{
			Rx_Fifo_Push = 0;
			flCommandReceived = 1;
			HAL_UART_Transmit(g_pUartDrv, &ascii_carriage_return, 1, 10);
			HAL_UART_Transmit(g_pUartDrv, &ascii_new_line, 1, 10);
		}
		else
		{
			HAL_UART_Transmit(g_pUartDrv, &ascii_carriage_return, 1, 10);
			HAL_UART_Transmit(g_pUartDrv, &ascii_new_line, 1, 10);
			HAL_UART_Transmit(g_pUartDrv, &ascii_douple_point, 1, 10);
		}
		HAL_UART_Receive_IT(g_pUartDrv, &Rx_Symbol, 1);
		break;


	case ASCII_CODE_BS:
		if(Rx_Fifo_Push)
		{
			HAL_UART_Transmit(g_pUartDrv, &Rx_Symbol, 1, 10);
			Rx_Fifo_Push --;
		}
		HAL_UART_Receive_IT(g_pUartDrv, &Rx_Symbol, 1);
		break;

	case ASCII_CODE_DEL:
		if(Rx_Fifo_Push)
		{
			HAL_UART_Transmit(g_pUartDrv, &Rx_Symbol, 1, 10);
			Rx_Fifo_Push --;
		}
		HAL_UART_Receive_IT(g_pUartDrv, &Rx_Symbol, 1);
		break;

	case ASCII_CODE_ESC:
		if(Rx_Fifo_Push)
		{
			HAL_UART_Transmit(g_pUartDrv, &Rx_Symbol, 1, 10);
			Rx_Fifo_Push --;
		}
		HAL_UART_Receive_IT(g_pUartDrv, &Rx_Symbol, 1);
		break;

	case ASCII_CODE_SC:
		HAL_UART_Receive_DMA(g_pUartDrv, &Rx_Symbol, 1);
		//flDMAtransferEN = 1;
		break;

	default:
		HAL_UART_Transmit(g_pUartDrv, &Rx_Symbol, 1, 10);
		Rx_Fifo[Rx_Fifo_Push++] = Rx_Symbol;
		HAL_UART_Receive_IT(g_pUartDrv, &Rx_Symbol, 1);
	}

	if(Rx_Fifo_Push == FIFO_SIZE)	Rx_Fifo_Push--;
}

/* This function handles UART TX complete interrupt request. */
/* ======================================================*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
/* ======================================================*/
{
	if(flUartTxBusy)
	{
		if(Tx_Fifo_Pull != Tx_Fifo_Push || flFifoFull)
		{
			HAL_UART_Transmit_IT(g_pUartDrv, (Tx_Fifo + Tx_Fifo_Pull++), 1);
			//	Tx_Fifo_Pull++;
			flFifoFull = 0;
			if(Tx_Fifo_Pull == FIFO_SIZE)
				Tx_Fifo_Pull = 0;
		}
		else
		{
			flUartTxBusy = 0;
		}
	}
}
/* ======================================================*/
