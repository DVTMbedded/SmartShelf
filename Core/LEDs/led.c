/*
 * led.c
 *
 *  Created on: Nov 14, 2020
 *      Author: Denislav Trifonov
 */

#include "led.h"

LEDS_DRIVER_STATE g_eLEDsState = LEDS_NOT_INIT;

/* ======================================================*/
void LEDs_Init()
/* ======================================================*/
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	HAL_PWREx_EnableVddIO2();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LED_BLUE_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : LED_GREEN_Pin */
	GPIO_InitStruct.Pin   = LED_GREEN_Pin;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LED_RED_Pin */
	GPIO_InitStruct.Pin   = LED_RED_Pin;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_RED_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : UCPD_DBN_Pin LED_BLUE_Pin */
	GPIO_InitStruct.Pin   = LED_BLUE_Pin;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_BLUE_GPIO_Port, &GPIO_InitStruct);

	g_eLEDsState = LEDS_INIT;
}

/* ======================================================*/
void LEDs_SetLEDState(BOARD_LEDS eLed, LEDS_STATE eState)
/* ======================================================*/
{
	GPIO_PinState eHALPinState = GPIO_PIN_RESET;

	if (eState == LED_ON)
	{
		eHALPinState = GPIO_PIN_SET;
	}

	if (g_eLEDsState == LEDS_INIT)
	{
		if (eLed == RED_LED)
		{
			HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, eHALPinState);
		}
		else if (eLed == GREEN_LED)
		{
			HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, eHALPinState);
		}
		else if (eLed == BLUE_LED)
		{
			HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, eHALPinState);
		}
	}
}

/* ======================================================*/
void LEDs_ToggleLED(BOARD_LEDS eLed)
/* ======================================================*/
{
	if (g_eLEDsState == LEDS_INIT)
	{
		if (eLed == RED_LED)
		{
			HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		}
		else if (eLed == GREEN_LED)
		{
			HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
		}
		else if (eLed == BLUE_LED)
		{
			HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
		}
	}
}
