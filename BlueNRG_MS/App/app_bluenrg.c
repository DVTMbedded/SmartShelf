/**
 ******************************************************************************
 * File Name          : app_bluenrg_ms.c
 * Description        : Source file
 *
 ******************************************************************************
 *
 * COPYRIGHT 2020 STMicroelectronics
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <app_bluenrg.h>
#include "hci.h"
#include "hci_le.h"
#include "hci_tl.h"
#include "link_layer.h"
#include "sensor.h"
#include "gatt_db.h"

#include "compiler.h"
#include "bluenrg_utils.h"
#include "stm32l5xx_nucleo.h"
#include "bluenrg_gap.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_hal_aci.h"
#include "sm.h"
#include "stm32l5xx_hal_tim.h"

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern volatile uint8_t set_connectable;

/* at startup, suppose the X-NUCLEO-IDB04A1 is used */
uint8_t bnrg_expansion_board = IDB04A1;
uint8_t bdaddr[BDADDR_SIZE];
static volatile uint8_t user_button_init_state = 1;
static volatile uint8_t user_button_pressed = 0;

/* Private function prototypes -----------------------------------------------*/
static void User_Process(void);
static void User_Init(void);

void BlueNRG_Init(void)
{
	/* Initialize the peripherals and the BLE Stack */
	const char *name = "BlueNRG";
	uint16_t service_handle, dev_name_char_handle, appearance_char_handle;

	uint8_t  bdaddr_len_out;
	uint8_t  hwVersion;
	uint16_t fwVersion;
	int ret;

	User_Init();

	/* Get the User Button initial state */
	user_button_init_state = BSP_PB_GetState(BUTTON_KEY);

	hci_init(user_notify, NULL);

	/* get the BlueNRG HW and FW versions */
	getBlueNRGVersion(&hwVersion, &fwVersion);

	/*
	 * Reset BlueNRG again otherwise we won't
	 * be able to change its MAC address.
	 * aci_hal_write_config_data() must be the first
	 * command after reset otherwise it will fail.
	 */
	hci_reset();
	HAL_Delay(100);

	if (hwVersion > 0x30)
	{
		/* X-NUCLEO-IDB05A1 expansion board is used */
		bnrg_expansion_board = IDB05A1;
	}

	ret = aci_hal_read_config_data(CONFIG_DATA_RANDOM_ADDRESS, BDADDR_SIZE, &bdaddr_len_out, bdaddr);

	if ((bdaddr[5] & 0xC0) != 0xC0)
	{
		while(1);
	}

	/* GATT Init */
	ret = aci_gatt_init();

	/* GAP Init */
	if (bnrg_expansion_board == IDB05A1)
	{
		ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
	}
	else
	{
		ret = aci_gap_init_IDB04A1(GAP_PERIPHERAL_ROLE_IDB04A1, &service_handle, &dev_name_char_handle, &appearance_char_handle);
	}

	/* Update device name */
	ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0,
			strlen(name), (uint8_t *)name);
	if (ret)
	{
		while(1);
	}

	ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
			OOB_AUTH_DATA_ABSENT,
			NULL,
			7,
			16,
			USE_FIXED_PIN_FOR_PAIRING,
			123456,
			BONDING);
	if (ret)
	{
		while(1);
	}

	ret = GattDB_RegisterSmartShelfService();
	if(ret != BLE_STATUS_SUCCESS)
	{
		while(1);
	}

	/* Set output power level */
	ret = aci_hal_set_tx_power_level(1,4);
}

/*
 * BlueNRG-MS background task
 */
void BlueNRG_Process(void)
{
	User_Process();
	hci_user_evt_proc();
}

/**
 * @brief  Initialize User process.
 *
 * @param  None
 * @retval None
 */
static void User_Init(void)
{
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
}

/**
 * @brief  Process user input (i.e. pressing the USER button on Nucleo board)
 *         and send the updated acceleration data to the remote client.
 *
 * @param  None
 * @retval None
 */
static void User_Process(void)
{
	static uint8_t arrShelveItems[MAX_SHELVES_COUNT];
	BLE_CHAR_UPDATE_STATUS eBleCharStatus = BLE_CHAR_IDLE;

	if (set_connectable)
	{
		Set_DeviceConnectable();
		set_connectable = FALSE;
	}

	for (uint8_t i = 0; i < EEPROM_GetTotalShelvesCount(); i++)
	{
		uint8_t nCurrentShelfItems = ToF_GetLeftItems(i);

		if (arrShelveItems[i] != nCurrentShelfItems)
		{
			arrShelveItems[i] = nCurrentShelfItems;
			eBleCharStatus = BLE_CHAR_UPDATED;
			GattDB_UpdateSmartShelfLeftStock(i, arrShelveItems[i]);
		}
	}
}

/**
 * @brief  BSP Push Button callback
 * @param  Button Specifies the pin connected EXTI line
 * @retval None.
 */
void BSP_PB_Callback(Button_TypeDef Button)
{
	/* Set the User Button flag */
	user_button_pressed = 1;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
