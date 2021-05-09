/*******************************************************************************
 * File Name          : m95640.h
 * Author             : Denislav Trifonov
 * Date               : 03-MAY-2021
 * Description        : Driver for M95640 EEPROM
 ********************************************************************************/

 /********************************************************************************
 * This file contains low-level functions for controlling M95640 SPI EEPROM.
 * Functions for read and write works in pooling mode.
 *******************************************************************************/

#ifndef INC_M95640_H_
#define INC_M95640_H_

#include "hw_resources.h"
#include "stm32l5xx_nucleo_bus.h"
#include "stm32l5xx_hal.h"
#include "main.h"
#include "sensor.h"

#define M95640_SIZE_BYTES	   8192
#define M95640_PAGE_SIZE_BYTES 32

#define INSTRUCTION_WRITE_STATUS  0x01
#define INSTRUCTION_WRITE_MEMORY  0x02
#define INSTRUCTION_READ_MEMORY   0x03
#define INSTRUCTION_WRITE_DISABLE 0x04
#define INSTRUCTION_READ_STATUS   0x05
#define INSTRUCTION_WRITE_ENABLE  0x06

#define DUMMY_BYTE 0x00

#define EEPROM_CS_PORT GPIOF
#define EEPROM_CS_PIN  GPIO_PIN_12

typedef enum {
	M95640_OK,
	M95640_INVALID_ARGUMENTS,
	M95640_ERROR_WRITE,
	M95640_ERROR_READ,
	M95640_ERROR_SPI_BUSY
}M95640_STATUS;

void M95640_Init();
void M95640_SetEnabled();
void M95640_SetDisabled();
M95640_STATUS M95640_ReadPage(uint16_t address, uint8_t* rxBuff, uint8_t size);
M95640_STATUS M95640_WritePage(uint16_t address, uint8_t* txBuff, uint8_t size);


#endif /* INC_M95640_H_ */


