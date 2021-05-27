/********************************************************************************
 * File Name          : eeprom.c
 * Author             : Denislav Trifonov
 * Date               : 04-MAY-2021
 * Description        : File which contains high level functions for interacting
 * 					    with M95650 EEPROM
 ********************************************************************************/

/********************************************************************************
 * M95640 Memory Map:
 * -------------------------------
 * Address 0 - 31 : Header
 * -------------------------------
 * Header Id             - 2 Bytes (0xAABB)
 * Shelves count         - 1 Byte
 * -------------------------------
 * Shelf 1 type          - 1 Byte
 * Shelf 1 I2C address   - 1 Byte
 * Shelf 1 initial stock - 1 Byte
 * Shelf 1 left stock    - 1 Byte
 * -------------------------------
 * Shelf 2 type          - 1 Byte
 * Shelf 2 I2C address   - 1 Byte
 * Shelf 2 initial stock - 1 Byte
 * Shelf 2 left stock    - 1 Byte
 * -------------------------------
 * ......
 * -------------------------------
 * Shelf n type          - 1 Byte
 * Shelf n initial stock - 1 Byte
 * -------------------------------
 ********************************************************************************/

#include "eeprom.h"


static EEPROM_SHELF_INFO g_arrShelves[MAX_SHELVES_COUNT];
static uint8_t           g_nShelvesCount = 0;


/* @brief  Initialize the EEPROM - setting the low-level driver. */
// ===========================================================
void EEPROM_Init()
// ===========================================================
{
	// Initialize SPI interface used for EEPROM and BLE peripherals
	BSP_SPI1_Init();
	M95640_Init();
}

// ===========================================================
void EEPROM_ReadAll()
// ===========================================================
{
	uint8_t arrEepromPage[32];

	// Read first EEPROM page
	if (M95640_ReadPage(DATA_STARTING_ADDRESS, arrEepromPage, 32) == M95640_OK)
	{
		// Analyze data and fill g_arrShelves array.
		if (arrEepromPage[0] == 0xAA && arrEepromPage[1] == 0xBB)
		{
			g_nShelvesCount = arrEepromPage[2];

			if (g_nShelvesCount <= MAX_SHELVES_COUNT)
			{
				for (uint8_t idx = 0; idx < g_nShelvesCount; idx++)
				{
					g_arrShelves[idx].m_eShelfType    = (SHELF_TYPES)arrEepromPage[idx * 4 + 3];
					g_arrShelves[idx].m_nI2cAddress   = arrEepromPage[idx * 4 + 4];
					g_arrShelves[idx].m_nInitialStock = arrEepromPage[idx * 4 + 5];
					g_arrShelves[idx].m_nLeftStock    = arrEepromPage[idx * 4 + 6];
				}
			}
			else
			{
				// TODO: Log an EEPROM error!
			}
		}
		else
		{
			// TODO: Log an EEPROM error!
		}
	}
	else
	{
		// TODO: Log an EEPROM error!
	}
}

// ===========================================================
EEPROM_SHELF_INFO* EEPROM_GetShelf(uint8_t nIndex)
// ===========================================================
{
	if (nIndex < g_nShelvesCount)
	{
		return &g_arrShelves[nIndex];
	}
	else
	{
		return nullptr;
	}
}

// ===========================================================
SHELF_TYPES EEPROM_GetShelfType(uint8_t nIndex)
// ===========================================================
{
	if (nIndex < g_nShelvesCount)
	{
		return g_arrShelves[nIndex].m_eShelfType;
	}
	return NONE;
}

// ===========================================================
uint8_t EEPROM_GetShelfInitialStock(uint8_t nIndex)
// ===========================================================
{
	if (nIndex < g_nShelvesCount)
	{
		return g_arrShelves[nIndex].m_nInitialStock;
	}
}

// ===========================================================
void EEPROM_RegisterNewShelf(EEPROM_SHELF_INFO *pShelf)
// ===========================================================
{
	if (g_nShelvesCount < MAX_SHELVES_COUNT)
	{
		g_arrShelves[g_nShelvesCount].m_eShelfType    = pShelf->m_eShelfType;
		g_arrShelves[g_nShelvesCount].m_nI2cAddress   = pShelf->m_nI2cAddress;
		g_arrShelves[g_nShelvesCount].m_nInitialStock = pShelf->m_nInitialStock;
		g_arrShelves[g_nShelvesCount].m_nLeftStock    = pShelf->m_nLeftStock;

	    // Write the new shelf data into EEPROM
		uint8_t arrShelfData[4];
		arrShelfData[0] = (uint8_t)g_arrShelves[g_nShelvesCount].m_eShelfType;
		arrShelfData[1] = g_arrShelves[g_nShelvesCount].m_nI2cAddress;
		arrShelfData[2] = g_arrShelves[g_nShelvesCount].m_nInitialStock;
		arrShelfData[3] = g_arrShelves[g_nShelvesCount].m_nLeftStock;

		if (M95640_WritePage((g_nShelvesCount * 4 + 3), arrShelfData, 4) == M95640_OK)
		{
			// Shelf written successfully!
			g_nShelvesCount++;
		}
		else
		{
			// TODO: Log an EEPROM error!
		}
	}
	else
	{
		// TODO: Log that maximum amount of shelves are used
	}
}

// ===========================================================
void EEPROM_UpdateShelfLeftStock(uint8_t nIndex, uint8_t nLeftStock)
// ===========================================================
{
	if (nIndex < g_nShelvesCount)
	{
		if (g_arrShelves[nIndex].m_nInitialStock >= nLeftStock)
		{
			g_arrShelves[nIndex].m_nLeftStock = nLeftStock;

			// Write the new shelf data into EEPROM
			if (M95640_WritePage((nIndex * 4 + 6), &g_arrShelves[nIndex].m_nLeftStock, 1) != M95640_OK)
			{
				// TODO: Log an EEPROM error!
			}
		}
		else
		{
			// TODO: Log that nLeftStock value is wrong
		}
	}
	else
	{
		// TODO: Log that no such shelf exist
	}
}
// ===========================================================
uint8_t EEPROM_GetTotalShelvesCount()
// ===========================================================
{
	return g_nShelvesCount;
}
// ===========================================================
