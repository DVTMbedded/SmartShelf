/*
 *  @file:   console_services.c
 *  @Author: Denislav Trifonov
 *  @Date:   10.11.2020
 *  @brief: This file contains UART console commands
 */

#include "console.h"

static void Service_Help(uint8_t *RxBuff);
static void Service_StartMeasurement(uint8_t *RxBuff);
static void Service_GetDistance(uint8_t *RxBuff);
static void Service_GetStock(uint8_t *RxBuff);
static void Service_Unknown(uint8_t *RxBuff);

static const char*  UartCommands[] = {
		"HELP",
		"STAM",
		"GETD",
		"GETS",
		""
};

static void (*FuncPtr[])(uint8_t *RxData) =
{
		&Service_Help,
		&Service_StartMeasurement,
		&Service_GetDistance,
		&Service_GetStock,
		&Service_Unknown
};

/* ======================================================*/
void  Console_Init()
/* ======================================================*/
{
	ConsoleDrv_Init(LPUART1);
	ConsoleDrv_Start();
}

/* ======================================================*/
void Console_Exec(void)
/* ======================================================*/
{
	int idx = 0;
	if (ConsoleDrv_CheckCommandStatus() == CMD_RECEIVED)
	{
		while (*(UartCommands[idx]) != '\0')
		{
			if(!(strncmp(UartCommands[idx],(char *)ConsoleDrv_GetReceivedCmd(), 4)))
			{
				ConsoleDrv_ClearDrvBuffers();
				ConsoleDrv_SetCommandStatus(CMD_FOUND);
				(*FuncPtr[idx])(ConsoleDrv_GetReceivedCmd());
				break;
			}
			else
			{
				ConsoleDrv_SetCommandStatus(CMD_NOT_FOUND);
			}
			idx ++;
		}

		if (ConsoleDrv_CheckCommandStatus() == CMD_NOT_FOUND)
		{
			(*FuncPtr[idx])(ConsoleDrv_GetReceivedCmd());
		}
		idx = 0;
		ConsoleDrv_OnCommandExecuted();
	}
}

/* Available commands  */
/* ====================================================== */
void Service_Help(uint8_t *RxBuff)
/* ====================================================== */
{
	ConsoleDrv_Puts(" Available commands:\r\n");
	ConsoleDrv_Puts("  - HELP - This information\r\n");
	ConsoleDrv_Puts("  - STAM - Initiate measurement with ToF sensor\r\n");
	ConsoleDrv_Puts("  - GETD - Get ToF sensor measurement\r\n");
	ConsoleDrv_Puts("  - GETS - Get left shelf items\r\n");
}

/* ======================================================*/
void Service_StartMeasurement(uint8_t *RxBuff)
/* ======================================================*/
{
	if (ToF_Measure(TOF_CENTRAL) == TOF_STATUS_OK)
	{
		ConsoleDrv_Puts("Measuring in process ...");
	}
	else
	{
		ConsoleDrv_Puts("Error measuring distance!");
	}
}

/* ======================================================*/
void Service_GetDistance(uint8_t *RxBuff)
/* ======================================================*/
{
	VL53LX_MultiRangingData_t* pData = ToF_GetDistance_mm(TOF_CENTRAL);

	if (pData != NULL)
	{
		if (pData->NumberOfObjectsFound)
		{
			for (uint8_t i = 0; i < pData->NumberOfObjectsFound; i++)
			{
				ConsoleDrv_Puts("\n\r-----------------------\n\r");
				ConsoleDrv_Printf("status=%d, D=%dmm, Min=%dmm, MAX=%dmm",
						pData->RangeData[i].RangeStatus,
						pData->RangeData[i].RangeMilliMeter,
						pData->RangeData[i].RangeMinMilliMeter,
						pData->RangeData[i].RangeMaxMilliMeter);
			}
			ConsoleDrv_Puts("\n\r-----------------------\n\r");
		}
		else
		{
			ConsoleDrv_Puts("There are no valid measurements!");
		}
	}
	else
	{
		ConsoleDrv_Puts("Error measuring distance!");
	}
}

/* ====================================================== */
void Service_GetStock(uint8_t *RxBuff)
/* ====================================================== */
{
	uint8_t nLeftItems = ToF_GetLeftItems(TOF_CENTRAL);

	ConsoleDrv_Printf("Left items: %d", nLeftItems);
}

/* ====================================================== */
void Service_Unknown(uint8_t *RxBuff)
/* ====================================================== */
{
	ConsoleDrv_Puts(" Unknown Command!\r\n");
}
/* ======================================================*/
