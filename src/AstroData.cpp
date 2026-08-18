/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Data
*/

#include "AstroData.h"
#include "AstroUtils.h"
#include "AstroStrings.h"
#include <stdio.h>
#include <string.h>

AstroSystemData::AstroSystemData()
    : systemName{0}, systemMode(Astro_SystemMode_Tracking),
      measurementMode(Astro_MeasurementMode_Metric), timeZoneOffset(0),
      pollingInterval(ASTRO_SYS_POLLING_INTERVAL), observer(), scheduler(), logger(), publisher()
{
    { const AstroString name = SFP(AStr_Astruino); snprintf(systemName, sizeof(systemName), "%s", name.c_str()); }
}

bool AstroSystemData::toJSON(char *bufferOut, size_t bufferSize) const
{
    if (!bufferOut || !bufferSize) { return false; }
    int written = snprintf(bufferOut, bufferSize,
        "{\"type\":\"ASYS\",\"systemName\":\"%s\",\"systemMode\":%d,\"measurementMode\":%d,"
        "\"timeZoneOffset\":%d,\"pollingInterval\":%u,\"lat\":%.7f,\"lon\":%.7f,\"elev\":%.2f,"
        "\"deploySunAlt\":%.4f,\"stowSunAlt\":%.4f,\"alignTol\":%.5f,\"settleSecs\":%u,\"reportSecs\":%lu,"
        "\"logLevel\":%d,\"logFilePrefix\":\"%s\",\"logToSDCard\":%s,\"logToWiFiStorage\":%s,"
        "\"dataFilePrefix\":\"%s\",\"pubToSDCard\":%s,\"pubToWiFiStorage\":%s,\"pubToMQTT\":%s}",
        systemName, (int)systemMode, (int)measurementMode, (int)timeZoneOffset, (unsigned int)pollingInterval,
        observer.latitudeDegrees, observer.longitudeDegrees, observer.elevationMeters,
        scheduler.deploySunAltitudeDegrees, scheduler.stowSunAltitudeDegrees,
        scheduler.alignmentToleranceDegrees, (unsigned int)scheduler.settleSeconds,
        (unsigned long)scheduler.reportIntervalSeconds, (int)logger.logLevel, logger.logFilePrefix,
        logger.logToSDCard ? "true" : "false", logger.logToWiFiStorage ? "true" : "false",
        publisher.dataFilePrefix, publisher.pubToSDCard ? "true" : "false",
        publisher.pubToWiFiStorage ? "true" : "false", publisher.pubToMQTT ? "true" : "false");
    return written >= 0 && (size_t)written < bufferSize;
}

bool AstroSystemData::fromJSON(const char *jsonIn)
{
    if (!jsonIn) { return false; }

    char typeIn[8] = {0};
    char systemNameIn[ASTRO_NAME_MAXSIZE] = {0};
    char logPrefixIn[ASTRO_PREFIX_MAXSIZE] = {0};
    char dataPrefixIn[ASTRO_PREFIX_MAXSIZE] = {0};
    long systemModeIn, measurementModeIn, timeZoneIn, logLevelIn;
    unsigned long pollingIn, settleIn, reportIn;
    double latitudeIn, longitudeIn, elevationIn, deployIn, stowIn, alignIn;
    bool logSDIn, logWiFiIn, pubSDIn, pubWiFiIn, pubMQTTIn;

    if (!astroJSONGetString(jsonIn, "type", typeIn, sizeof(typeIn)) || strcmp(typeIn, "ASYS") != 0 ||
        !astroJSONGetString(jsonIn, "systemName", systemNameIn, sizeof(systemNameIn)) ||
        !astroJSONGetLong(jsonIn, "systemMode", &systemModeIn) ||
        !astroJSONGetLong(jsonIn, "measurementMode", &measurementModeIn) ||
        !astroJSONGetLong(jsonIn, "timeZoneOffset", &timeZoneIn) ||
        !astroJSONGetUnsignedLong(jsonIn, "pollingInterval", &pollingIn) ||
        !astroJSONGetDouble(jsonIn, "lat", &latitudeIn) ||
        !astroJSONGetDouble(jsonIn, "lon", &longitudeIn) ||
        !astroJSONGetDouble(jsonIn, "elev", &elevationIn) ||
        !astroJSONGetDouble(jsonIn, "deploySunAlt", &deployIn) ||
        !astroJSONGetDouble(jsonIn, "stowSunAlt", &stowIn) ||
        !astroJSONGetDouble(jsonIn, "alignTol", &alignIn) ||
        !astroJSONGetUnsignedLong(jsonIn, "settleSecs", &settleIn) ||
        !astroJSONGetUnsignedLong(jsonIn, "reportSecs", &reportIn) ||
        !astroJSONGetLong(jsonIn, "logLevel", &logLevelIn) ||
        !astroJSONGetString(jsonIn, "logFilePrefix", logPrefixIn, sizeof(logPrefixIn)) ||
        !astroJSONGetBool(jsonIn, "logToSDCard", &logSDIn) ||
        !astroJSONGetBool(jsonIn, "logToWiFiStorage", &logWiFiIn) ||
        !astroJSONGetString(jsonIn, "dataFilePrefix", dataPrefixIn, sizeof(dataPrefixIn)) ||
        !astroJSONGetBool(jsonIn, "pubToSDCard", &pubSDIn) ||
        !astroJSONGetBool(jsonIn, "pubToWiFiStorage", &pubWiFiIn) ||
        !astroJSONGetBool(jsonIn, "pubToMQTT", &pubMQTTIn)) {
        return false;
    }

    snprintf(systemName, sizeof(systemName), "%s", systemNameIn);
    systemMode = (Astro_SystemMode)systemModeIn;
    measurementMode = (Astro_MeasurementMode)measurementModeIn;
    timeZoneOffset = (int16_t)timeZoneIn;
    pollingInterval = (uint16_t)pollingIn;
    observer = AstroObserver(latitudeIn, longitudeIn, elevationIn);

    scheduler.deploySunAltitudeDegrees = deployIn;
    scheduler.stowSunAltitudeDegrees = stowIn;
    scheduler.alignmentToleranceDegrees = alignIn;
    scheduler.settleSeconds = (uint16_t)settleIn;
    scheduler.reportIntervalSeconds = (uint32_t)reportIn;

    logger.logLevel = (Astro_LogLevel)logLevelIn;
    snprintf(logger.logFilePrefix, sizeof(logger.logFilePrefix), "%s", logPrefixIn);
    logger.logToSDCard = logSDIn;
    logger.logToWiFiStorage = logWiFiIn;

    snprintf(publisher.dataFilePrefix, sizeof(publisher.dataFilePrefix), "%s", dataPrefixIn);
    publisher.pubToSDCard = pubSDIn;
    publisher.pubToWiFiStorage = pubWiFiIn;
    publisher.pubToMQTT = pubMQTTIn;
    return true;
}
