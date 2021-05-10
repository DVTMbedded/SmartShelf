/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "D:\Work\University\Master Thesis\Firmware\SmartShelf\Core\EEPROM\eeprom.h"

static void GPIO_Init(void);

int main(void)
{
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	System_ClockInit();

	/* Initialize all configured peripherals */
	RTC_Init();
	LEDs_Init();
	Console_Init();
	ToF_Init(TOF_CENTRAL);

	// Initialize all GPIOs
	GPIO_Init();

	// Initialize EEPROM
	EEPROM_Init();
	// Get the Smart Shelves data from EEPROM
	EEPROM_ReadAll();
	// Initialize BLE module
	BlueNRG_Init();

	while (1)
	{
		Console_Exec();
		ToF_Exec();
		BlueNRG_Process();
	}
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	HAL_PWREx_EnableVddIO2();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, BT_CS_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(BT_RESET_GPIO_Port, BT_RESET_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(UCPD_DBN_GPIO_Port, UCPD_DBN_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : PC0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : BT_CS_Pin */
	GPIO_InitStruct.Pin = BT_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : BT_IRQ_Pin */
	GPIO_InitStruct.Pin = BT_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(BT_IRQ_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : BT_RESET_Pin */
	GPIO_InitStruct.Pin = BT_RESET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(BT_RESET_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : UCPD_FLT_Pin */
	GPIO_InitStruct.Pin = UCPD_FLT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(UCPD_FLT_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : UCPD_DBN_Pin */
	GPIO_InitStruct.Pin = UCPD_DBN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(UCPD_DBN_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	HAL_NVIC_SetPriority(EXTI13_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI13_IRQn);

}
