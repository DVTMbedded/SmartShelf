/*
 * led.h
 *
 *  Created on: Sep 16, 2020
 *      Author: TrifonovD
 */

#ifndef LED_H_
#define LED_H_


typedef enum {
	RED_LED   = 0,
	GREEN_LED = 1,
	BLUE_LED  = 2
}LED_T;

typedef enum {
	LED_OFF = 0,
	LED_ON  = 1
}LED_STATE_T;

void Led_Init();
void Led_SetState(LED_T eLed, LED_STATE_T eLedState);
void Led_Toggle(LED_T eLed);


#endif /* LED_H_ */
