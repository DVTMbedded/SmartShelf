/********************************************************************************
 * File Name          : log.h
 * Author             : Denislav Trifonov
 * Date               : 07-MAY-2021
 * Description        : File which contains information about system events/errors/warning
 ********************************************************************************/
#ifndef INC_LOG_H_
#define INC_LOG_H_

typedef enum {
	LOG_TYPE_INFO     = 0,
	LOG_TYPE_WARNING  = 1,
	LOG_TYPE_ERROR    = 2,
	LOG_NOT_AVAILABLE = 3
}LOG_TYPE;

typedef enum {
	INFO_SYSTEM_OK   = 0,
	INFO_SHELF_EMPTY = 1
}LOG_INFO;

typedef enum {
	WARNING_NONE        = 0,
	WARNING_STOCK_TAKEN = 1,
	WARNING_STOCK_ADDED = 2
}LOG_WARNING;

typedef enum {
	ERROR_NONE   = 0,
	ERROR_EEPROM = 1
}LOG_ERROR;


void Log_SetLogType(LOG_TYPE eLogType);
void Log_SetLogInfo(LOG_INFO eLogInfo);
void Log_SetLogWarning(LOG_WARNING eLogWarning);
void Log_SetLogError(LOG_ERROR eLogError);

LOG_TYPE    Log_GetLogType(void);
LOG_INFO    Log_GetLogInfo(void);
LOG_WARNING Log_GetLogWarning(void);
LOG_ERROR   Log_GetLogError(void);

void Log_ClearLog(void);

#endif /* INC_LOG_H_ */
