/*
 * led.h
 *
 *  Created on: Nov 14, 2020
 *      Author: Denislav Trifonov
 */

#ifndef LEDS_LED_H_
#define LEDS_LED_H_

#include "main.h"

typedef enum
{
	LEDS_NOT_INIT,
	LEDS_INIT
}LEDS_DRIVER_STATE;

typedef enum
{
	RED_LED,
	GREEN_LED,
	BLUE_LED
}BOARD_LEDS;

typedef enum
{
	LED_OFF,
	LED_ON
}LEDS_STATE;

void LEDs_Init();
void LEDs_SetLEDState(BOARD_LEDS eLed, LEDS_STATE eState);
void LEDs_ToggleLED(BOARD_LEDS eLed);


#endif /* LEDS_LED_H_ */
