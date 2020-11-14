/*
 *  @file:   console_services.c
 *  @Author: Denislav Trifonov
 *  @Date:   10.11.2020
 *  @brief: This file contains UART console commands
 */

#include "console.h"

static void Service_Help(uint8_t *RxBuff);
static void Service_GetDistance(uint8_t *RxBuff);
static void Service_Unknown(uint8_t *RxBuff);

static const char*  UartCommands[] = {
		"HELP",
		"GETD",
		""
};

static void (*FuncPtr[])(uint8_t *RxData) =
{
		&Service_Help,
		&Service_GetDistance,
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
static void Service_GetDistance(uint8_t *RxBuff)
/* ======================================================*/
{
	VL53LX_MultiRangingData_t* pData = NULL;

	if (ToF_Measure(TOF_CENTRAL) == TOF_STATUS_OK)
	{
		pData = ToF_GetDistance_mm(TOF_CENTRAL);

		if (pData != NULL)
		{
			//ConsoleDrv_Printf("Distance: %d mm", nDistance);
#if 1
			for (uint8_t i = 0; i < pData->NumberOfObjectsFound; i++)
			{
				ConsoleDrv_Printf("Distance %d: %d mm", i, pData->RangeData[i].RangeMilliMeter);
			}
#endif
		}
		else
		{
			ConsoleDrv_Puts("Error measuring distance!");
		}
	}
	else
	{
		ConsoleDrv_Puts("Error measuring distance!");
	}
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
	ConsoleDrv_Puts("  - GETD - Get ToF sensor measurement\r\n");
}

/* ====================================================== */
void Service_Unknown(uint8_t *RxBuff)
/* ====================================================== */
{
	ConsoleDrv_Puts(" Unknown Command!\r\n");
}
/* ======================================================*/
