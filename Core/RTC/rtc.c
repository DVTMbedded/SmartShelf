/*
 * rtc.c
 *
 *  Created on: Nov 14, 2020
 *      Author: Denislav Trifonov
 */

#include "rtc.h"

static RTC_HandleTypeDef hrtc;

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
/* ======================================================*/
void RTC_Init()
/* ======================================================*/
{
	RTC_PrivilegeStateTypeDef privilegeState = {0};

	/** Initialize RTC Only */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}
	privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
	privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
	privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
	privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
	if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
	{
		LEDs_SetLEDState(RED_LED, LED_ON);
	}
}

