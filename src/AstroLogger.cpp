/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Logger
*/

#include "Astruino.h"
#include <stdio.h>
#include <string.h>

AstroLogger::AstroLogger()
    : _logLevel(Astro_LogLevel_All), _data(nullptr), _logSignal()
{ ; }

void AstroLogger::setSubData(AstroLoggerSubData *data)
{
    _data = data;
}

void AstroLogger::setLogLevel(Astro_LogLevel logLevel)
{
    if (_data) { _data->logLevel = logLevel; }
    _logLevel = logLevel;
}

Astro_LogLevel AstroLogger::getLogLevel() const
{
    return _data ? _data->logLevel : _logLevel;
}

void AstroLogger::log(Astro_LogLevel level, int64_t timestamp, const char *prefix, const char *message)
{
    if (getLogLevel() == Astro_LogLevel_None || level < getLogLevel()) { return; }

    AstroLogEvent event;
    event.level = level;
    event.timestamp = timestamp;
    snprintf(event.prefix, sizeof(event.prefix), "%s", prefix ? prefix : "");
    snprintf(event.message, sizeof(event.message), "%s", message ? message : "");
    _logSignal.fire(event);
}

Signal<const AstroLogEvent, ASTRO_DEFAULT_MAXSIZE> &AstroLogger::getLogSignal()
{
    return _logSignal;
}

void AstroLogger::logMessage(int64_t timestamp, const char *message)
{
    log(Astro_LogLevel_Info, timestamp, "[INFO] ", message);
}

void AstroLogger::logWarning(int64_t timestamp, const char *message)
{
    log(Astro_LogLevel_Warnings, timestamp, "[WARN] ", message);
}

void AstroLogger::logError(int64_t timestamp, const char *message)
{
    log(Astro_LogLevel_Errors, timestamp, "[ERROR] ", message);
}

void AstroLogger::logField(int64_t timestamp, const char *fieldName, double value, const char *units)
{
    char message[ASTRO_LOG_MESSAGE_MAXSIZE];
    if (units && units[0]) { snprintf(message, sizeof(message), "%s: %.3f %s", fieldName, value, units); }
    else { snprintf(message, sizeof(message), "%s: %.3f", fieldName, value); }
    logMessage(timestamp, message);
}

void AstroLogger::logEnvironment(int64_t timestamp, double ambientC, double humidity, double dewPointC,
                                 double opticsC, double cameraSensorC, double cameraBodyC,
                                 float dewHeaterPower, float cameraCoolingPower, float cameraFanPower)
{
    AstroString envReport = SFP(AStr_EnvironmentReport);
    logMessage(timestamp, envReport.c_str());
    logField(timestamp, "Ambient temperature", ambientC, "C");
    logField(timestamp, "Humidity", humidity, "%");
    logField(timestamp, "Dew point", dewPointC, "C");
    if (opticsC < 900.0) { logField(timestamp, "Optics temperature", opticsC, "C"); }
    if (cameraSensorC < 900.0) { logField(timestamp, "Camera sensor temperature", cameraSensorC, "C"); }
    if (cameraBodyC < 900.0) { logField(timestamp, "Camera body temperature", cameraBodyC, "C"); }
    if (dewHeaterPower >= 0.0f) { logField(timestamp, "Dew heater", dewHeaterPower * 100.0, "%"); }
    if (cameraCoolingPower >= 0.0f) { logField(timestamp, "Camera cooling", cameraCoolingPower * 100.0, "%"); }
    if (cameraFanPower >= 0.0f) { logField(timestamp, "Camera fan", cameraFanPower * 100.0, "%"); }
}

AstroLoggerSubData::AstroLoggerSubData()
    : AstroSubData(0), logLevel(Astro_LogLevel_All), logFilePrefix{0},
      logToSDCard(false), logToWiFiStorage(false)
{
    snprintf(logFilePrefix, sizeof(logFilePrefix), "logs/astro");
}

void AstroLoggerSubData::toJSONObject(JsonObject &objectOut) const
{
    AstroSubData::toJSONObject(objectOut);
    objectOut["logLevel"] = (int)logLevel;
    objectOut["logFilePrefix"] = logFilePrefix;
    objectOut["logToSDCard"] = logToSDCard;
    objectOut["logToWiFiStorage"] = logToWiFiStorage;
}

void AstroLoggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroSubData::fromJSONObject(objectIn);
    logLevel = (Astro_LogLevel)(objectIn["logLevel"] | (int)logLevel);
    const char *prefix = objectIn["logFilePrefix"] | nullptr;
    if (prefix) {
        strncpy(logFilePrefix, prefix, ASTRO_PREFIX_MAXSIZE - 1);
        logFilePrefix[ASTRO_PREFIX_MAXSIZE - 1] = '\0';
    }
    logToSDCard = objectIn["logToSDCard"] | logToSDCard;
    logToWiFiStorage = objectIn["logToWiFiStorage"] | logToWiFiStorage;
}
