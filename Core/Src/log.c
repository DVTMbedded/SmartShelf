/********************************************************************************
 * File Name          : log.c
 * Author             : Denislav Trifonov
 * Date               : 07-MAY-2021
 * Description        : File which contains information about system events/errors/warning
 ********************************************************************************/

#include "log.h"

static LOG_TYPE    g_eLogType    = LOG_NOT_AVAILABLE;

static LOG_INFO    g_eLogInfo    = INFO_SYSTEM_OK;
static LOG_WARNING g_eLogWarning = WARNING_NONE;
static LOG_ERROR   g_eLogError   = ERROR_NONE;

/* @brief Set type of logged data */
// ===========================================================
void Log_SetLogType(LOG_TYPE eLogType)
// ===========================================================
{
	if (g_eLogType != eLogType)
	{
		g_eLogType = eLogType;
	}
}

/* @brief Set logged data of type LOG_INFO */
// ===========================================================
void Log_SetLogInfo(LOG_INFO eLogInfo)
// ===========================================================
{
	if (g_eLogInfo != eLogInfo)
	{
		g_eLogInfo = eLogInfo;
	}
}

/* @brief Set logged data of type LOG_WARNING */
// ===========================================================
void Log_SetLogWarning(LOG_WARNING eLogWarning)
// ===========================================================
{
	if (g_eLogWarning != eLogWarning)
	{
		g_eLogWarning = eLogWarning;
	}
}

/* @brief Get logged data of type LOG_ERROR */
// ===========================================================
void Log_SetLogError(LOG_ERROR eLogError)
// ===========================================================
{
	if (g_eLogError != eLogError)
	{
		g_eLogError = eLogError;
	}
}

/* @brief Get type of logged data */
// ===========================================================
LOG_TYPE Log_GetLogType(void)
// ===========================================================
{
	return g_eLogType;
}

/* @brief Get logged data of type LOG_INFO */
// ===========================================================
LOG_INFO Log_GetLogInfo(void)
// ===========================================================
{
	return g_eLogInfo;
}

/* @brief Get logged data of type LOG_WARNING */
// ===========================================================
LOG_WARNING Log_GetLogWarning(void)
// ===========================================================
{
	return g_eLogWarning;
}

/* @brief Get logged data of type LOG_ERROR */
// ===========================================================
LOG_ERROR Log_GetLogError(void)
// ===========================================================
{
	return g_eLogError;
}

/* @brief Clear previously logged system messages */
// ===========================================================
void Log_ClearLog(void)
// ===========================================================
{
	g_eLogType = LOG_NOT_AVAILABLE;
}

// ===========================================================
