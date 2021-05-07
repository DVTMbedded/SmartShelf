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

uint16_t SmartShelfServiceHandle       = 0;
uint16_t SmartShelfHeaderCharHandle    = 0;
uint16_t SmartShelfLeftStockCharHandle = 0;

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

	/* Add_HWServW2ST_Service */
	COPY_SMART_SHELF_SERVICE_UUID(uuid);
	BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);

	ret = aci_gatt_add_serv(UUID_TYPE_128, service_uuid.Service_UUID_128, PRIMARY_SERVICE,
			1+3*5, &SmartShelfServiceHandle);

	if (ret != BLE_STATUS_SUCCESS)
	{
		return BLE_STATUS_ERROR;
	}

	/* Fill the Environmental BLE Characteristic */
	COPY_SMART_SHELF_HEADER_CHAR_UUID(uuid);
	uuid[14] |= 0x04; /* One Temperature value*/
	uuid[14] |= 0x10; /* Pressure value*/
	BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);

	ret =  aci_gatt_add_char(SmartShelfServiceHandle, UUID_TYPE_128, char_uuid.Char_UUID_128,
			7,
			CHAR_PROP_NOTIFY|CHAR_PROP_READ,
			ATTR_PERMISSION_NONE,
			GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
			16, 0, &SmartShelfHeaderCharHandle);

	if (ret != BLE_STATUS_SUCCESS)
	{
		return BLE_STATUS_ERROR;
	}

	/* Fill the AccGyroMag BLE Characteristc */
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

	return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Update acceleration characteristic value
 * @param  AxesRaw_t structure containing acceleration value in mg.
 * @retval tBleStatus Status
 */
tBleStatus GattDB_UpdateSmartShelfLeftStock(uint8_t nLeftStock)
{
	uint8_t buff[5];
	tBleStatus ret;

	buff[0] = nLeftStock; // Device index
	buff[1] = nLeftStock;
	buff[2] = nLeftStock;
	buff[3] = nLeftStock;
	buff[4] = nLeftStock;

	ret = aci_gatt_update_char_value(SmartShelfServiceHandle, SmartShelfLeftStockCharHandle,
			0, 5, buff);

	if (ret != BLE_STATUS_SUCCESS)
	{
		return BLE_STATUS_ERROR ;
	}

	return BLE_STATUS_SUCCESS;
}

/*******************************************************************************
 * Function Name  : Read_Request_CB.
 * Description    : Update the sensor valuse.
 * Input          : Handle of the characteristic to update.
 * Return         : None.
 *******************************************************************************/
void Read_Request_CB(uint16_t handle)
{
	tBleStatus ret;

	if (handle == SmartShelfLeftStockCharHandle + 1)
	{
		ret = GattDB_UpdateSmartShelfLeftStock(5);
	}

	else if (handle == SmartShelfHeaderCharHandle + 1)
	{
		ret = GattDB_GetHeaderCharacteristic();
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

tBleStatus GattDB_GetHeaderCharacteristic()
{
	tBleStatus ret;
	uint8_t buff[7];

	buff[0] = 2; // Number of SmartShelf sensors
	buff[1] = 0; // Type of 1st item - Drinks
	buff[2] = 5; // Initial value of Cans
	buff[3] = 3; // Remaining value of cans
	buff[4] = 1; // Type of 2nd item - Snacks
	buff[5] = 5; // Initial value of Snacks
	buff[6] = 3; // Remaining value of snacks

	ret = aci_gatt_update_char_value(SmartShelfServiceHandle, SmartShelfHeaderCharHandle,
			0, 7, buff);

	if (ret != BLE_STATUS_SUCCESS)
	{
		return BLE_STATUS_ERROR ;
	}

	return BLE_STATUS_SUCCESS;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
