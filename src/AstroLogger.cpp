/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Logger
*/

#include "AstroLogger.h"
#include "AstroStrings.h"
#include "AstroUtils.h"
#include <stdio.h>
#include <string.h>

AstroLogger::AstroLogger()
    : _sink(nullptr), _context(nullptr), _logLevel(Astro_LogLevel_All), _data(nullptr)
{ ; }

void AstroLogger::setSink(LogSink sink, void *context)
{
    _sink = sink;
    _context = context;
}

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
    if (!_sink || getLogLevel() == Astro_LogLevel_None || level < getLogLevel()) { return; }
    AstroLogEvent event;
    event.level = level;
    event.timestamp = timestamp;
    snprintf(event.prefix, sizeof(event.prefix), "%s", prefix ? prefix : "");
    snprintf(event.message, sizeof(event.message), "%s", message ? message : "");
    _sink(_context, event);
}

void AstroLogger::logMessage(int64_t timestamp, const char *message) { log(Astro_LogLevel_Info, timestamp, "[INFO] ", message); }
void AstroLogger::logWarning(int64_t timestamp, const char *message) { log(Astro_LogLevel_Warnings, timestamp, "[WARN] ", message); }
void AstroLogger::logError(int64_t timestamp, const char *message) { log(Astro_LogLevel_Errors, timestamp, "[ERROR] ", message); }

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
    const AstroString envReport = SFP(AStr_Environmentreport);
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
    : logLevel(Astro_LogLevel_All), logFilePrefix{0}, logToSDCard(false), logToWiFiStorage(false)
{
    snprintf(logFilePrefix, sizeof(logFilePrefix), "logs/astro");
}

bool AstroLoggerSubData::toJSON(char *bufferOut, size_t bufferSize) const
{
    if (!bufferOut || !bufferSize) { return false; }
    int written = snprintf(bufferOut, bufferSize,
        "{\"logLevel\":%d,\"logFilePrefix\":\"%s\",\"logToSDCard\":%s,\"logToWiFiStorage\":%s}",
        (int)logLevel, logFilePrefix, logToSDCard ? "true" : "false", logToWiFiStorage ? "true" : "false");
    return written >= 0 && (size_t)written < bufferSize;
}

bool AstroLoggerSubData::fromJSON(const char *jsonIn)
{
    if (!jsonIn) { return false; }
    long levelIn;
    char prefixIn[ASTRO_PREFIX_MAXSIZE] = {0};
    bool sdIn, wifiIn;
    if (!astroJSONGetLong(jsonIn, "logLevel", &levelIn) ||
        !astroJSONGetString(jsonIn, "logFilePrefix", prefixIn, sizeof(prefixIn)) ||
        !astroJSONGetBool(jsonIn, "logToSDCard", &sdIn) ||
        !astroJSONGetBool(jsonIn, "logToWiFiStorage", &wifiIn)) { return false; }
    logLevel = (Astro_LogLevel)levelIn;
    snprintf(logFilePrefix, sizeof(logFilePrefix), "%s", prefixIn);
    logToSDCard = sdIn;
    logToWiFiStorage = wifiIn;
    return true;
}
