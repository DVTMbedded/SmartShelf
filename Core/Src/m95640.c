/*******************************************************************************
 * File Name          : m95640.c
 * Author             : Denislav Trifonov
 * Date               : 03-MAY-2021
 * Description        : Driver for M95640 EEPROM
 ********************************************************************************
 * This file contains low-level functions for controlling M95640 SPI EEPROM.
 * Functions for read and write works in pooling mode.
 *******************************************************************************/

#include "m95640.h"

/* @brief Initialize M95640 EEPROM.
 * SPI interface is already initialized, so all that has to be done is to configure CS GPIO pin.
 */
// ===========================================================
void M95640_Init()
// ===========================================================
{
	// Initialize Chip select pin
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin   = EEPROM_CS_PIN;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EEPROM_CS_PORT, &GPIO_InitStruct);

	// Enable writing to memory array
	M95640_SetEnabled();
	HAL_Delay(1);
	uint8_t txData = INSTRUCTION_WRITE_ENABLE;
	BSP_SPI1_Send(&txData, 1);

	// According to the datasheet write time is 5 mS.
	HAL_Delay(5);

	// Set initial state of the EEPROM to be disabled
	M95640_SetDisabled();
}

/**
  * @brief Enable SPI communication with EEPROM (set CS pin low)
  */
// ===========================================================
void M95640_SetEnabled()
// ===========================================================
{
	HAL_GPIO_WritePin(EEPROM_CS_PORT, EEPROM_CS_PIN, GPIO_PIN_RESET);
}

/**
  * @brief Enable SPI communication with EEPROM (set CS pin high)
  */
// ===========================================================
void M95640_SetDisabled()
// ===========================================================
{
	HAL_GPIO_WritePin(EEPROM_CS_PORT, EEPROM_CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Read bytes from EEPROM data array. Maximum amount that could be read is 32 (page size)
  * @param  address: address in EEPROM from where to start reading
  * @param  rxBuff: pointer where the data bytes to be stored
  * @param  size: Amount of data bytes to be read
  * @retval M95640_STATUS
  */
// ===========================================================
M95640_STATUS M95640_ReadPage(uint16_t address, uint8_t* rxBuff, uint8_t size)
// ===========================================================
{
	M95640_STATUS eStatus = M95640_OK;
	uint8_t arrRxData[M95640_PAGE_SIZE_BYTES + 3];

	// Check if function arguments are correct
	if (rxBuff == nullptr || address > M95640_SIZE_BYTES || size > M95640_PAGE_SIZE_BYTES)
	{
		eStatus = M95640_INVALID_ARGUMENTS;
	}

	// Check if SPI communication is not used from BLE module (or if BLE module is initialized)
	else if (HAL_GPIO_ReadPin(HCI_TL_SPI_CS_PORT, HCI_TL_SPI_CS_PIN) == GPIO_PIN_RESET)
	{
		eStatus = M95640_ERROR_SPI_BUSY;
	}

	// Check if there is enough space in the page to write the data bytes
	else if ((address / M95640_PAGE_SIZE_BYTES) != ((address + size - 1) / M95640_PAGE_SIZE_BYTES))
	{
		eStatus = M95640_INVALID_ARGUMENTS;
	}

	else
	{
		HAL_Delay(1);

		// Enable EEPROM
		M95640_SetEnabled();

		HAL_Delay(1);
		// Read EEPROM
		uint8_t txData[M95640_PAGE_SIZE_BYTES + 3];

		txData[0] = INSTRUCTION_READ_MEMORY;
		txData[1] = (uint8_t)(address >> 8);
		txData[2] = (uint8_t)(address);

		if (BSP_SPI1_SendRecv(txData, arrRxData, size + 3) != BSP_ERROR_NONE)
		{
			eStatus = M95640_ERROR_READ;
		}
		else
		{
			for (uint8_t idx = 0; idx < size; idx ++)
			{
				rxBuff[idx] = arrRxData[idx + 3];
			}
		}

		// Disable EEPROM
		M95640_SetDisabled();
	}

	return eStatus;
}

/**
  * @brief  Write bytes to EEPROM data array. Maximum amount that could be written is 32 (page size)
  * @param  address: address in EEPROM from where to start writing
  * @param  rxBuff: pointer to data bytes which are going to be written
  * @param  size: Amount of data bytes to be written
  * @retval M95640_STATUS
  */
// ===========================================================
M95640_STATUS M95640_WritePage(uint16_t address, uint8_t* txBuff, uint8_t size)
// ===========================================================
{
	M95640_STATUS eStatus = M95640_OK;

	// Check if function arguments are correct
	if (txBuff == nullptr || address > M95640_SIZE_BYTES || size > M95640_PAGE_SIZE_BYTES)
	{
		eStatus = M95640_INVALID_ARGUMENTS;
	}

	// Check if SPI communication is not used from BLE module
	else if (HAL_GPIO_ReadPin(HCI_TL_SPI_CS_PORT, HCI_TL_SPI_CS_PIN) == GPIO_PIN_RESET)
	{
		eStatus = M95640_ERROR_SPI_BUSY;
	}

	// Check if there is enough space in the page to write the data bytes
	else if ((address / M95640_PAGE_SIZE_BYTES) != ((address + size - 1) / M95640_PAGE_SIZE_BYTES))
	{
		eStatus = M95640_INVALID_ARGUMENTS;
	}

	else
	{
		HAL_Delay(1);

		// Enable EEPROM
		M95640_SetEnabled();

		HAL_Delay(1);

		// Write EEPROM
		uint8_t txData[M95640_PAGE_SIZE_BYTES + 3];
		txData[0] = INSTRUCTION_WRITE_MEMORY;
		txData[1] = (uint8_t)(address >> 8);
		txData[2] = (uint8_t)(address);

		// Copy the data bytes
		for (uint8_t idx = 0; idx < size; idx ++)
		{
			txData[idx + 3] = txBuff[idx];
		}

		if (BSP_SPI1_Send(txData, size + 3) != BSP_ERROR_NONE)
		{
			eStatus = M95640_ERROR_WRITE;
		}

		else
		{
			// According to the datasheet write time is 5 mS, but let's make it 10
			HAL_Delay(10);
		}

		// Disable EEPROM
		M95640_SetDisabled();
	}

	return eStatus;
}
// ===========================================================
