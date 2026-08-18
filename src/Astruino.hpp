/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino System Inlines
*/

#ifndef Astruino_HPP
#define Astruino_HPP

#include <string.h>

inline void Astruino::setSystemName(const char *systemName)
{
    if (!systemName) { return; }
    strncpy(_systemData.systemName, systemName, sizeof(_systemData.systemName) - 1);
    _systemData.systemName[sizeof(_systemData.systemName) - 1] = '\0';
}

inline const char *Astruino::getSystemName() const
{
    return _systemData.systemName;
}

inline void Astruino::setSystemMode(Astro_SystemMode systemMode)
{
    _systemData.systemMode = systemMode;
}

inline Astro_SystemMode Astruino::getSystemMode() const
{
    return _systemData.systemMode;
}

inline void Astruino::setMeasurementMode(Astro_MeasurementMode measurementMode)
{
    _systemData.measurementMode = measurementMode;
}

inline Astro_MeasurementMode Astruino::getMeasurementMode() const
{
    return _systemData.measurementMode;
}

inline void Astruino::setTimeZoneOffset(int16_t timeZoneOffset)
{
    _systemData.timeZoneOffset = timeZoneOffset;
}

inline int16_t Astruino::getTimeZoneOffset() const
{
    return _systemData.timeZoneOffset;
}

inline void Astruino::setPollingInterval(uint16_t pollingInterval)
{
    _systemData.pollingInterval = pollingInterval;
}

inline uint16_t Astruino::getPollingInterval() const
{
    return _systemData.pollingInterval;
}

#endif // /ifndef Astruino_HPP
