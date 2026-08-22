/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino System Inlines
*/

#ifndef Astruino_HPP
#define Astruino_HPP

#include <string.h>

inline bool Twilight::isDaytime(time_t unixTime) const {
    DateTime time = isUTC ? DateTime((uint32_t)unixTime) : localTime(unixTime);
    double hour = time.hour() + (time.minute() / 60.0) + (time.second() / 3600.0);
    return sunrise <= sunset ? hour >= sunrise && hour <= sunset
                             : hour >= sunrise || hour <= sunset;
}

inline bool Twilight::isDaytime(DateTime localTime) const
{
    DateTime time = isUTC ? DateTime((uint32_t)unixTime(localTime)) : localTime;
    double hour = time.hour() + (time.minute() / 60.0) + (time.second() / 3600.0);
    return sunrise <= sunset ? hour >= sunrise && hour <= sunset
                             : hour >= sunrise || hour <= sunset;
}

inline time_t Twilight::hourToUnixTime(double hour, bool isUTC)
{
    return isUTC ? unixDayStart() + (time_t)(hour * SECS_PER_HOUR)
                 : unixTime(localDayStart() + TimeSpan(hour * SECS_PER_HOUR));
}

inline DateTime Twilight::hourToLocalTime(double hour, bool isUTC)
{
    return isUTC ? localTime(unixDayStart() + (time_t)(hour * SECS_PER_HOUR))
                 : localDayStart() + TimeSpan(hour * SECS_PER_HOUR);
}


#ifdef ASTRO_USE_WIFI

inline WiFiClass *Astroduino::getWiFi(bool begin)
{
    return getWiFi(getWiFiSSID(), getWiFiPassword(), begin);
}

#endif
#ifdef ASTRO_USE_ETHERNET

inline EthernetClass *Astroduino::getEthernet(bool begin)
{
    return getEthernet(getMACAddress(), begin);
}

#endif

inline void Astroduino::broadcastLowMemory()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        iter->second->handleLowMemory();
    }
}

inline void Astroduino::performAutosave()
{
    for (int autosave = 0; autosave < 2; ++autosave) {
        switch (autosave == 0 ? _systemData->autosaveEnabled : _systemData->autosaveFallback) {
            case Astro_Autosave_EnabledToSDCardJson:
                saveToSDCard(JSON);
                break;
            case Astro_Autosave_EnabledToSDCardRaw:
                saveToSDCard(RAW);
                break;
            case Astro_Autosave_EnabledToEEPROMJson:
                saveToEEPROM(JSON);
                break;
            case Astro_Autosave_EnabledToEEPROMRaw:
                saveToEEPROM(RAW);
                break;
            case Astro_Autosave_EnabledToWiFiStorageJson:
                #ifdef ASTRO_USE_WIFI_STORAGE
                    saveToWiFiStorage(JSON);
                #endif
                break;
            case Astro_Autosave_EnabledToWiFiStorageRaw:
                #ifdef ASTRO_USE_WIFI_STORAGE
                    saveToWiFiStorage(RAW);
                #endif
            case Astro_Autosave_Disabled:
                break;
        }
    }

    _lastAutosave = unixNow();
}

inline void Astroduino::notifyRTCTimeUpdated()
{
    _rtcBattFail = false;
}

inline void Astroduino::broadcastDateChanged()
{
    if (getSystemMode() == Astro_SystemMode_Tracking) {
        for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
            if (iter->second->isPanelType()) {
                auto panel = static_pointer_cast<AstroPanel>(iter->second);

                if (panel && panel->isAnyTrackingClass()) {
                    auto trackingPanel = static_pointer_cast<AstroTrackingPanel>(iter->second);
                    trackingPanel->notifyDateChanged();
                }
            }
        }
    }
}

inline void Astroduino::notifySignificantTime(time_t time)
{
    logger.updateInitTracking(time);
    _lastAutosave = isAutosaveEnabled() ? time : 0;
}

inline void Astroduino::notifySignificantLocation(Location loc)
{
    if (_systemData) { _systemData->bumpRevisionIfNeeded(); }
}


inline AstroLoggerSubData *AstroLogger::loggerData() const
{
    return &Astroduino::_activeInstance->_systemData->logger;
}

inline bool AstroLogger::hasLoggerData() const
{
    return Astroduino::_activeInstance && Astroduino::_activeInstance->_systemData;
}

inline bool AstroLogger::isLoggingToSDCard() const
{
    return hasLoggerData() && loggerData()->logLevel != Astro_LogLevel_None && loggerData()->logToSDCard;
}

#ifdef ASTRO_USE_WIFI_STORAGE

inline bool AstroLogger::isLoggingToWiFiStorage() const
{
    return hasLoggerData() && loggerData()->logLevel != Astro_LogLevel_None && loggerData()->logToWiFiStorage;
}

#endif

inline void AstroLogger::logActivation(const AstroActuator *actuator)
{
    if (actuator) { logMessage(actuator->getId().getDisplayString(), SFP(HStr_Log_HasEnabled)); }
}

inline void AstroLogger::logDeactivation(const AstroActuator *actuator)
{
    if (actuator) { logMessage(actuator->getId().getDisplayString(), SFP(HStr_Log_HasDisabled)); }
}

inline void AstroLogger::logProcess(const AstroObjInterface *obj, const String &processString, const String &statusString)
{
    if (obj) { logMessage(obj->getId().getDisplayString(), processString, statusString); }
}

inline void AstroLogger::logStatus(const AstroObjInterface *obj, const String &statusString)
{
    if (obj) { logMessage(obj->getId().getDisplayString(), statusString); }
}

inline Astro_LogLevel AstroLogger::getLogLevel() const
{
    return hasLoggerData() ? loggerData()->logLevel : Astro_LogLevel_None;
}

inline bool AstroLogger::isLoggingEnabled() const
{
    return hasLoggerData() && loggerData()->logLevel != Astro_LogLevel_None && (loggerData()->logToSDCard || loggerData()->logToWiFiStorage);
}


inline AstroPublisherSubData *AstroPublisher::publisherData() const
{
    return &Astroduino::_activeInstance->_systemData->publisher;
}

inline bool AstroPublisher::hasPublisherData() const
{
    return Astroduino::_activeInstance && Astroduino::_activeInstance->_systemData;
}

inline bool AstroPublisher::isPublishingToSDCard() const
{
    return hasPublisherData() && publisherData()->pubToSDCard;
}

#ifdef ASTRO_USE_WIFI_STORAGE

inline bool AstroPublisher::isPublishingToWiFiStorage() const
{
    return hasPublisherData() && publisherData()->pubToWiFiStorage;
}

#endif
#ifdef ASTRO_USE_MQTT

inline bool AstroPublisher::isPublishingToMQTTClient() const
{
    return hasPublisherData() && _mqttClient;
}

#endif

inline bool AstroPublisher::isPublishingEnabled() const
{
    return hasPublisherData() && (publisherData()->pubToSDCard || publisherData()->pubToWiFiStorage
        #ifdef ASTRO_USE_MQTT
            || _mqttClient
        #endif
        );
}

inline void AstroPublisher::setNeedsTabulation()
{
    _needsTabulation = hasPublisherData();
}


inline AstroSchedulerSubData *AstroScheduler::schedulerData() const
{
    return &Astroduino::_activeInstance->_systemData->scheduler;
}

inline bool AstroScheduler::hasSchedulerData() const
{
    return Astroduino::_activeInstance && Astroduino::_activeInstance->_systemData;
}

#endif // /ifndef Astruino_HPP
