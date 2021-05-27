/*
 * tof.cpp
 *
 *  Created on: 11.11.2020 г.
 *      Author: Denislav Trifonov
 */

#include "tof.h"

/* Private data  ---------------------------------------------------------*/
static I2C_HandleTypeDef hi2c1;
static I2C_STATUS g_eI2cStatus                = I2C_STATUS_NOT_INIT;

static TOF_STATE g_eToFSensorState[SENSORS_SUPPORTED] = {STATE_NOT_INIT};

static GPIO_InitTypeDef g_arrToFGPIOs[SENSORS_SUPPORTED] = {
		{GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0}};

static GPIO_TypeDef* g_arrToFPorts[SENSORS_SUPPORTED] =
{GPIOC};

static GPIO_InitTypeDef g_arrToFXShutDownPin[SENSORS_SUPPORTED] = {
		{GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0}};

static GPIO_TypeDef* g_arrToFXShutDownPorts[SENSORS_SUPPORTED] =
{GPIOF};

static VL53LX_Dev_t g_ToFSensorDriverData[SENSORS_SUPPORTED];
static VL53LX_MultiRangingData_t g_ToFSensorMeasurementData[SENSORS_SUPPORTED];

static uint8_t g_arrLeftItems[SENSORS_SUPPORTED];

/* Private function prototypes -----------------------------------------------*/
static void I2C_Init(void);
static void GPIO_Init(TOF_SUPPORTED_SENSORS eSensor);
static void CalculateLeftShelfItems(TOF_SUPPORTED_SENSORS eSensor);

/* Public function definitions  -----------------------------------------------*/

// @brief initialize selected ToF sensor
/* ======================================================*/
void ToF_Init(TOF_SUPPORTED_SENSORS eSensor)
/* ======================================================*/
{
	uint8_t nDummyByte  = 0;

	EEPROM_SHELF_INFO *arrShelfInfo = EEPROM_GetShelf(eSensor);

	g_eToFSensorState[eSensor] = STATE_INIT_IN_PROCESS;

	// Initialize the VL53L3CX GPIO pin
	GPIO_Init(eSensor);

	/* Initialize I2C communication.
	 * This function is going to be executed only once
	 * because each VL53L3CX sensor is connected to the MCU
	 * via same I2C interface.
	 */
	if (g_eI2cStatus == I2C_STATUS_NOT_INIT)
	{
		g_eI2cStatus = I2C_STATUS_INIT;
		I2C_Init();
	}

	g_ToFSensorDriverData[eSensor].I2cHandle  = &hi2c1;
	g_ToFSensorDriverData[eSensor].I2cDevAddr = arrShelfInfo->m_nI2cAddress;

	HAL_GPIO_WritePin(g_arrToFXShutDownPorts[eSensor], g_arrToFXShutDownPin[eSensor].Pin, GPIO_PIN_RESET);
	HAL_Delay(2);
	HAL_GPIO_WritePin(g_arrToFXShutDownPorts[eSensor], g_arrToFXShutDownPin[eSensor].Pin, GPIO_PIN_SET);
	HAL_Delay(2);

	// Check the I2C communication with VL53L3CX
	VL53LX_RdByte(&g_ToFSensorDriverData[eSensor], 0x010F, &nDummyByte);
	if (nDummyByte != 0xEA)
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	VL53LX_RdByte(&g_ToFSensorDriverData[eSensor], 0x0110, &nDummyByte);

	if (nDummyByte != 0xAA)
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	if (VL53LX_WaitDeviceBooted(&g_ToFSensorDriverData[eSensor]))
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	if (VL53LX_DataInit(&g_ToFSensorDriverData[eSensor]))
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	VL53LX_CalibrationData_t CalibrationData;
	VL53LX_GetCalibrationData(&g_ToFSensorDriverData[eSensor], &CalibrationData);

	if (VL53LX_SetCalibrationData(&g_ToFSensorDriverData[eSensor], &CalibrationData))
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	if (VL53LX_SetDistanceMode(&g_ToFSensorDriverData[eSensor], 1))
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	if (VL53LX_SetTuningParameter(&g_ToFSensorDriverData[eSensor], VL53LX_TUNINGPARM_PHASECAL_PATCH_POWER, 2))
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	if (VL53LX_SmudgeCorrectionEnable(&g_ToFSensorDriverData[eSensor], VL53LX_SMUDGE_CORRECTION_CONTINUOUS))
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	if (VL53LX_StartMeasurement(&g_ToFSensorDriverData[eSensor]))
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	for (uint8_t i = 0; i < SENSORS_SUPPORTED; i++)
	{
		g_arrLeftItems[i] = 0;
	}
}

/* @brief: This function is called in the main loop.
 *         It implements the device driver state machine
 */
/* ======================================================*/
void ToF_Exec()
/* ======================================================*/
{
	for (uint8_t i = 0; i < SENSORS_SUPPORTED; i++)
	{
		// ToF sensor is not initialized
		if (g_eToFSensorState[i] == STATE_NOT_INIT || g_eToFSensorState[i] == STATE_ERROR)
		{
			continue;
		}

		// ToF sensor is initializing
		else if (g_eToFSensorState[i] == STATE_INIT_IN_PROCESS)
		{
			// Check if interrupt has occurred
			if (HAL_GPIO_ReadPin(g_arrToFPorts[i], g_arrToFGPIOs[i].Pin) == GPIO_PIN_RESET)
			{
				if(!VL53LX_GetMultiRangingData(&g_ToFSensorDriverData[i], &g_ToFSensorMeasurementData[i]))
				{
					// Perform measurement which has to be ignored
					if (!VL53LX_ClearInterruptAndStartMeasurement(&g_ToFSensorDriverData[i]))
					{
						g_eToFSensorState[i] = STATE_IGNORE_FIRST_DATA;
					}
					else
					{
						g_eToFSensorState[i] = STATE_ERROR;
					}
				}
				else
				{
					g_eToFSensorState[i] = STATE_ERROR;
				}
			}
		}

		// First data needs to be ignored (when the distance is changing)
		else if (g_eToFSensorState[i] == STATE_IGNORE_FIRST_DATA)
		{
			// Check if interrupt has occurred
			if (HAL_GPIO_ReadPin(g_arrToFPorts[i], g_arrToFGPIOs[i].Pin) == GPIO_PIN_RESET)
			{
				if(!VL53LX_GetMultiRangingData(&g_ToFSensorDriverData[i], &g_ToFSensorMeasurementData[i]))
				{
					// Perform measurement which has to be ignored
					if (!VL53LX_ClearInterruptAndStartMeasurement(&g_ToFSensorDriverData[i]))
					{
						g_eToFSensorState[i] = STATE_MEASURING;
					}
					else
					{
						g_eToFSensorState[i] = STATE_ERROR;
					}
				}
				else
				{
					g_eToFSensorState[i] = STATE_ERROR;
				}
			}
		}

		else if (g_eToFSensorState[i] == STATE_PENDING_MEASUREMENT)
		{
			ToF_Measure(i);
		}

		// ToF sensor is performing measurement
		else if (g_eToFSensorState[i] == STATE_MEASURING)
		{
			// Check if interrupt has occurred
			if (HAL_GPIO_ReadPin(g_arrToFPorts[i], g_arrToFGPIOs[i].Pin) == GPIO_PIN_RESET)
			{
				if(!VL53LX_GetMultiRangingData(&g_ToFSensorDriverData[i], &g_ToFSensorMeasurementData[i]))
				{
					g_eToFSensorState[i] = STATE_IDLE;
					CalculateLeftShelfItems(i);
				}
				else
				{
					g_eToFSensorState[i] = STATE_ERROR;
				}
			}
		}
	}
}

/* ======================================================*/
TOF_STATUS ToF_Measure(TOF_SUPPORTED_SENSORS eSensor)
/* ======================================================*/
{
	TOF_STATUS eStatus = TOF_STATUS_OK;

	if (g_eToFSensorState[eSensor] == STATE_IDLE || g_eToFSensorState[eSensor] == STATE_PENDING_MEASUREMENT)
	{
		if(!VL53LX_GetMultiRangingData(&g_ToFSensorDriverData[eSensor], &g_ToFSensorMeasurementData[eSensor]))
		{
			if (!VL53LX_ClearInterruptAndStartMeasurement(&g_ToFSensorDriverData[eSensor]))
			{
				g_eToFSensorState[eSensor] = STATE_IGNORE_FIRST_DATA;
			}
			else
			{
				eStatus                    = TOF_STATUS_ERROR;
				g_eToFSensorState[eSensor] = STATE_ERROR;
			}
		}
		else
		{
			g_eToFSensorState[eSensor] = STATE_ERROR;
		}
	}
	else
	{
		eStatus = TOF_STATUS_ERROR;
	}

	return eStatus;
}

/* ======================================================*/
VL53LX_MultiRangingData_t* ToF_GetDistance_mm(TOF_SUPPORTED_SENSORS eSensor)
/* ======================================================*/
{
	VL53LX_MultiRangingData_t* pData = NULL;

	if (g_eToFSensorState[eSensor] == STATE_IDLE)
	{
		pData = &g_ToFSensorMeasurementData[eSensor];
	}

	return pData;
}

/* ======================================================*/
uint8_t ToF_GetLeftItems(TOF_SUPPORTED_SENSORS eSensor)
/* ======================================================*/
{
	if (eSensor < SENSORS_SUPPORTED)
	{
		return g_arrLeftItems[eSensor];
	}
	else
	{
		return 0;
	}
}

/* ======================================================*/
void ToF_InitiateMeasurement(TOF_SUPPORTED_SENSORS eSensor)
/* ======================================================*/
{
	if (eSensor < SENSORS_SUPPORTED)
	{
		if (g_eToFSensorState[eSensor] == STATE_IDLE)
		{
			g_eToFSensorState[eSensor] = STATE_PENDING_MEASUREMENT;
		}
	}
}


/* Private function definitions  -----------------------------------------------*/
/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
/* ======================================================*/
static void I2C_Init(void)
/* ======================================================*/
{
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x2050133E;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}

	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}
}

/* ======================================================*/
void GPIO_Init(TOF_SUPPORTED_SENSORS eSensor)
/* ======================================================*/
{
	HAL_GPIO_Init(g_arrToFPorts[eSensor], &g_arrToFGPIOs[eSensor]);
	HAL_GPIO_Init(g_arrToFXShutDownPorts[eSensor], &g_arrToFXShutDownPin[eSensor]);
}

/* ======================================================*/
void CalculateLeftShelfItems(TOF_SUPPORTED_SENSORS eSensor)
/* ======================================================*/
{
	static uint8_t m_arrShelvesLeftItems[SENSORS_SUPPORTED];
	static uint8_t m_nDebounceCounters[SENSORS_SUPPORTED];

	VL53LX_MultiRangingData_t* pData = ToF_GetDistance_mm(eSensor);
	int16_t nMeasuredDistance_mm     = 0;

	if (pData != NULL)
	{
		if (pData->RangeData[0].RangeStatus == VL53LX_RANGESTATUS_RANGE_VALID)
		{
			/* Check the measured distance and based on this determine how many items are left
			 * In order to say that a measurement is valid, 3 consecutive measurements has to be the
			 * in the same measurement area.
			 */
			int8_t shelfLeftItems;
			nMeasuredDistance_mm   = pData->RangeData[eSensor].RangeMilliMeter;
			SHELF_TYPES eShelfType = EEPROM_GetShelfType(eSensor);
			uint8_t eShelfMaxItems = EEPROM_GetShelfInitialStock(eSensor);

			if (eShelfType == DRINK)
			{
				if (nMeasuredDistance_mm < TOF_INITIAL_OFFSET_MM)
				{
					shelfLeftItems = eShelfMaxItems;
				}
				else if (nMeasuredDistance_mm > (eShelfMaxItems * DRINK_SIZE_MM + TOF_OFFSET_DISTANCE_MM))
				{
					shelfLeftItems = 0;
				}
				else
				{
					shelfLeftItems = (eShelfMaxItems - (nMeasuredDistance_mm - TOF_OFFSET_DISTANCE_MM) / DRINK_SIZE_MM);

					if (shelfLeftItems < 0)
					{
						shelfLeftItems = 0;
					}
					if (shelfLeftItems > eShelfMaxItems)
					{
						shelfLeftItems = eShelfMaxItems;
					}
				}

				if (m_arrShelvesLeftItems[eSensor] == shelfLeftItems)
				{
					if (++m_nDebounceCounters[eSensor] == 3)
					{
						g_arrLeftItems[eSensor]      = shelfLeftItems;
						m_nDebounceCounters[eSensor] = 0;
					}
				}
				else
				{
					m_arrShelvesLeftItems[eSensor] = shelfLeftItems;
					m_nDebounceCounters[eSensor]   = 0;
				}
			}
		}
	}
}

/* ======================================================*/
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
/* ======================================================*/
{
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
}
/* ======================================================*/
