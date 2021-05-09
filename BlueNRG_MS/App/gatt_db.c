/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
 * File Name          : gatt_db.c
 * Author             :
 * Version            : V1.0.0
 * Date               : 16-September-2015
 * Description        : Functions to build GATT DB and handle GATT events.
 ********************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bluenrg_def.h"
#include "gatt_db.h"
#include "bluenrg_conf.h"
#include "bluenrg_gatt_aci.h"

/** @brief Macro that stores Value into a buffer in Little Endian Format (2 bytes)*/
#define HOST_TO_LE_16(buf, val)    ( ((buf)[0] =  (uint8_t) (val)    ) , \
		((buf)[1] =  (uint8_t) (val>>8) ) )

/** @brief Macro that stores Value into a buffer in Little Endian Format (4 bytes) */
#define HOST_TO_LE_32(buf, val)    ( ((buf)[0] =  (uint8_t) (val)     ) , \
		((buf)[1] =  (uint8_t) (val>>8)  ) , \
		((buf)[2] =  (uint8_t) (val>>16) ) , \
		((buf)[3] =  (uint8_t) (val>>24) ) )

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
		do {\
			uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
			uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
			uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
			uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
		}while(0)

/* Service and charactersitics UUIDs */
#define COPY_SMART_SHELF_SERVICE_UUID(uuid_struct)         COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0x9a,0xb4,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_SMART_SHELF_HEADER_CHAR_UUID(uuid_struct)     COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_SMART_SHELF_LEFT_STOCK_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct,0x00,0xE0,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_SMART_SHELF_LOG_MSG_CHAR_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0x00,0xE1,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)

uint16_t SmartShelfServiceHandle       = 0;
uint16_t SmartShelfHeaderCharHandle    = 0;
uint16_t SmartShelfLeftStockCharHandle = 0;
uint16_t SmartShelfLogMsgCharHandle    = 0;

/* UUIDS */
Service_UUID_t service_uuid;
Char_UUID_t char_uuid;

extern uint16_t connection_handle;
extern uint32_t start_time;

/**
 * @brief  Add the main SmartShelf service (it contains 2 characteristics).
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus GattDB_RegisterSmartShelfService(void)
{
	tBleStatus ret;
	uint8_t uuid[16];

	/* Add SmartShelf Service */
	COPY_SMART_SHELF_SERVICE_UUID(uuid);
	BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);

	ret = aci_gatt_add_serv(UUID_TYPE_128, service_uuid.Service_UUID_128, PRIMARY_SERVICE,
			1+3*5, &SmartShelfServiceHandle);

	if (ret != BLE_STATUS_SUCCESS)
	{
		return BLE_STATUS_ERROR;
	}

	/* Add Header Characteristic. It contains information about
	 * all connected SmartShelf devices - their type and amount of items */
	COPY_SMART_SHELF_HEADER_CHAR_UUID(uuid);
	BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);

	ret =  aci_gatt_add_char(SmartShelfServiceHandle, UUID_TYPE_128, char_uuid.Char_UUID_128,
			16,
			CHAR_PROP_NOTIFY|CHAR_PROP_READ,
			ATTR_PERMISSION_NONE,
			GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
			16, 0, &SmartShelfHeaderCharHandle);

	if (ret != BLE_STATUS_SUCCESS)
	{
		return BLE_STATUS_ERROR;
	}

	/* Add Left Stock Characteristic. It contains information about
	   remaining stock into all connected SmartShelf devices. */
	COPY_SMART_SHELF_LEFT_STOCK_CHAR_UUID(uuid);
	BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);

	ret =  aci_gatt_add_char(SmartShelfServiceHandle, UUID_TYPE_128, char_uuid.Char_UUID_128,
			5,
			CHAR_PROP_NOTIFY|CHAR_PROP_READ,
			ATTR_PERMISSION_NONE,
			GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
			16, 0, &SmartShelfLeftStockCharHandle);

	if (ret != BLE_STATUS_SUCCESS)
	{
		return BLE_STATUS_ERROR;
	}


	/* Add Log Message Characteristic.
	  It contains information about system events/warnings/errors */
	COPY_SMART_SHELF_LOG_MSG_CHAR_UUID(uuid);
	BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);

	ret =  aci_gatt_add_char(SmartShelfServiceHandle, UUID_TYPE_128, char_uuid.Char_UUID_128,
			2,
			CHAR_PROP_NOTIFY|CHAR_PROP_READ,
			ATTR_PERMISSION_NONE,
			GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
			16, 0, &SmartShelfLogMsgCharHandle);

	if (ret != BLE_STATUS_SUCCESS)
	{
		return BLE_STATUS_ERROR;
	}

	return BLE_STATUS_SUCCESS;
}

/*******************************************************************************
 * Function Name  : Read_Request_DB.
 * Description    : Get characteristic value
 * Input          : Handle of the characteristic to update.
 * Return         : None.
 *******************************************************************************/
void GattDB_ReadRequest(uint16_t handle)
{
	tBleStatus ret;

	if (handle == SmartShelfLeftStockCharHandle + 1)
	{
		ret = GattDB_UpdateSmartShelfLeftStock(EEPROM_GetTotalShelvesCount(), 5);
	}

	else if (handle == SmartShelfHeaderCharHandle + 1)
	{
		ret = GattDB_GetHeaderCharacteristic();
	}

	else if (handle == SmartShelfLogMsgCharHandle + 1)
	{
		ret = GattDB_GetSystemMessage();
	}

	if (connection_handle !=0)
	{
		ret = aci_gatt_allow_read(connection_handle);
	}

	if (ret != BLE_STATUS_SUCCESS)
	{
		// Log an BLE error
	}
}

/******************************************************************************
 * @brief  Get information about all used SmartShelf devices and their current state
 * @retval tBleStatus Status
 ******************************************************************************/
tBleStatus GattDB_GetHeaderCharacteristic()
{
	tBleStatus ret;
	uint8_t buff[16];

	buff[0] = EEPROM_GetTotalShelvesCount(); // Number of SmartShelf sensors

	if (buff[0] <= MAX_SHELVES_COUNT)
	{
		for (uint8_t idx = 0; idx < buff[0]; idx ++)
		{
			// Get SmartShelf type
			buff[idx * 3 + 1] = EEPROM_GetShelf(idx)->m_eShelfType;
			// Get initial stock value
			buff[idx * 3 + 2] = EEPROM_GetShelf(idx)->m_nInitialStock;
			// Get left stock value
			buff[idx * 3 + 3] = EEPROM_GetShelf(idx)->m_nLeftStock;
		}

		ret = aci_gatt_update_char_value(SmartShelfServiceHandle, SmartShelfHeaderCharHandle,
				0, 16, buff);

		if (ret != BLE_STATUS_SUCCESS)
		{
			return BLE_STATUS_ERROR ;
		}

		return BLE_STATUS_SUCCESS;
	}

	else
	{
		return BLE_STATUS_ERROR;
	}
}

/*******************************************************************************
 * @brief  Update acceleration characteristic value
 * @param  nIndex - SmartShelf index (from 0 to MAX_SHELVES_COUNT)
 * @param  nLeftStock - number of items that are available on the shelf
 * @retval tBleStatus Status
 *******************************************************************************/
tBleStatus GattDB_UpdateSmartShelfLeftStock(uint8_t nIndex, uint8_t nLeftStock)
{
	static uint8_t buff[MAX_SHELVES_COUNT] = {0, 0, 0, 0, 0};
	tBleStatus ret;

	if (nIndex < EEPROM_GetTotalShelvesCount())
	{
		buff[nIndex] = nLeftStock;

		ret = aci_gatt_update_char_value(SmartShelfServiceHandle, SmartShelfLeftStockCharHandle,
				0, 5, buff);

		if (ret != BLE_STATUS_SUCCESS)
		{
			return BLE_STATUS_ERROR;
		}
	}

	else if (nIndex == EEPROM_GetTotalShelvesCount())
	{
		for (uint8_t idx = 0; idx < EEPROM_GetTotalShelvesCount(); idx++)
		{
			buff[idx] = nLeftStock;
		}

		ret = aci_gatt_update_char_value(SmartShelfServiceHandle, SmartShelfLeftStockCharHandle,
				0, 5, buff);

		if (ret != BLE_STATUS_SUCCESS)
		{
			return BLE_STATUS_ERROR;
		}
	}

	else
	{
		return BLE_STATUS_ERROR;
	}

	return BLE_STATUS_SUCCESS;
}

/*******************************************************************************
 * @brief  Update acceleration characteristic value
 * @param  nIndex - SmartShelf index (from 0 to MAX_SHELVES_COUNT)
 * @param  nLeftStock - number of items that are available on the shelf
 * @retval tBleStatus Status
 *******************************************************************************/
tBleStatus GattDB_GetSystemMessage(void)
{
	tBleStatus ret;
	uint8_t arrSystemLogs[2];
	LOG_TYPE eLogType = Log_GetLogType();

	arrSystemLogs[0] = (uint8_t)eLogType;
	if (eLogType == LOG_TYPE_INFO)
	{
		arrSystemLogs[1] = Log_GetLogInfo();
	}
	else if (eLogType == LOG_TYPE_WARNING)
	{
		arrSystemLogs[1] = Log_GetLogWarning();
	}
	else
	{
		arrSystemLogs[1] = Log_GetLogError();
	}

	ret = aci_gatt_update_char_value(SmartShelfServiceHandle, SmartShelfLogMsgCharHandle,
			0, 2, arrSystemLogs);

	if (ret != BLE_STATUS_SUCCESS)
	{
		return BLE_STATUS_ERROR;
	}

	return BLE_STATUS_SUCCESS;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
