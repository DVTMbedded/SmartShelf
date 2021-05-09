/*******************************************************************************
 * File Name          : main.c
 * Author             : Denislav Trifonov
 * Date               : 11-11-2020
 * Description        : Main program file
 ********************************************************************************/

#include <app_bluenrg.h>
#include "main.h"
#include "m95640.h"
#include "eeprom.h"
#include "tof.h"
#include "console.h"
#include "system.h"



int main(void)
{
	// Initialize Clocks and HAL libraries
	System_Init();
	// Initialize board LEDs
	LEDs_Init();
	// Initialize EEPROM
	EEPROM_Init();
	// Get the Smart Shelves data from EEPROM
	EEPROM_ReadAll();
	// Initialize BLE module
	BlueNRG_Init();
	// Initialize ToF sensor
	ToF_Init(TOF_CENTRAL);
	// Initialize system console
	Console_Init();

	while (1)
	{
		Console_Exec();
		ToF_Exec();
		BlueNRG_Process();
	}
}
