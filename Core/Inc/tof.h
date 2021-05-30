/*
 * tof.h
 *
 *  Created on: 11.11.2020 г.
 *      Author: Denislav Trifonov
 */

#ifndef TOF_TOF_H_
#define TOF_TOF_H_

#include <math.h>
#include "vl53lx_api.h"
#include "53L3A2.h"
#include "53l3a2.h"
#include "stm32l5xx_hal.h"
#include "main.h"
#include "vl53lx_def.h"
#include "led.h"
#include "eeprom.h"
#include "log.h"

#define SENSORS_SUPPORTED 1

#define XNUCLEO_SENSOR_LEFT    0
#define XNUCLEO_SENSOR_CENTER  1
#define XNUCLEO_SENSOR_RIGHT   2

#define DRINK_SIZE_MM                 55
#define TOF_INITIAL_OFFSET_MM         100
#define TOF_DISTANCE_BETWEEN_ITEMS_MM 20

#define TOF_POLYFIT_COEF_A 0.9740
#define TOF_POLYFIT_COEF_B 26.0097

typedef enum {
	TOF_CENTRAL = 0,
}TOF_SUPPORTED_SENSORS;

typedef enum {
	TOF_STATUS_OK,
	TOF_STATUS_ERROR
}TOF_STATUS;

typedef enum {
	STATE_NOT_INIT,
	STATE_INIT_IN_PROCESS,
	STATE_IDLE,
	STATE_PENDING_MEASUREMENT,
	STATE_MEASURING,
	STATE_IGNORE_FIRST_DATA,
	STATE_ERROR
}TOF_STATE;

typedef enum {
	MEASUREMENT_NOT_PERFORMED = 0,
	MEASUREMENT_PERFORMED     = 1
}TOF_MEASUREMENT_PERFORMED;

typedef enum {
	TOF_MEASURING_MODE_INTERRUPT,
	TOF_MEASURING_MODE_POLLING
}TOF_MEASURING_MODE;

typedef enum {
	I2C_STATUS_NOT_INIT,
	I2C_STATUS_INIT
}I2C_STATUS;

typedef enum {
	XNUCLEO_53L3A2 = 0,
	CUSTOM_PCB
}PCB_USED;

void ToF_Init(TOF_SUPPORTED_SENSORS eSensor);
void ToF_Exec();
void ToF_InitiateMeasurement(TOF_SUPPORTED_SENSORS eSensor);
TOF_STATUS ToF_Measure(TOF_SUPPORTED_SENSORS eSensor);
VL53LX_MultiRangingData_t* ToF_GetDistance_mm(TOF_SUPPORTED_SENSORS eSensor);
uint8_t ToF_GetLeftItems(TOF_SUPPORTED_SENSORS eSensor);

#endif /* TOF_TOF_H_ */
