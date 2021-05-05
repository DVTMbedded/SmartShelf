/*******************************************************************************
 * File Name          : eeprom.h
 * Author             : Denislav Trifonov
 * Date               : 04-MAY-2021
 * Description        : File which contains high level functions for interacting
 * 					    with M95650 EEPROM
 ********************************************************************************/

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include "m95640.h"

#define MAX_SHELVES_COUNT     0x05
#define DATA_STARTING_ADDRESS 0x00

typedef enum {
	DRINK = 0,
	SNACK = 1
}SHELF_TYPES;

typedef struct {
	SHELF_TYPES m_eShelfType;
	uint8_t     m_nI2cAddress;
	uint8_t     m_nInitialStock;
	uint8_t     m_nLeftStock;
}EEPROM_SHELF_INFO;

void eeprom_init();
void eeprom_readAll();
void eeprom_registerNewShelf(EEPROM_SHELF_INFO *pShelf);
void eeprom_updateShelfLeftStock(uint8_t nIndex, uint8_t nLeftStock);

EEPROM_SHELF_INFO* eeprom_getShelf(uint8_t nIndex);
uint8_t            eeprom_getTotalShelvesCount();


#endif /* INC_EEPROM_H_ */
