/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Data
*/

#ifndef AstroData_H
#define AstroData_H

#include "AstroLogger.h"
#include "AstroPublisher.h"
#include "AstroScheduler.h"

// User System Setup Data
// id: ASYS. Stores the controller-level settings that are needed to reconstruct an
// Astruino system without tying operation to networking or any particular storage device.
struct AstroSystemData {
    char systemName[ASTRO_NAME_MAXSIZE];                    // System display name
    Astro_SystemMode systemMode;                            // System operating mode
    Astro_MeasurementMode measurementMode;                  // Preferred measurement units mode
    int16_t timeZoneOffset;                                 // Timezone offset from UTC, in whole hours
    uint16_t pollingInterval;                               // Sensor polling interval, in milliseconds
    AstroObserver observer;                                 // Fixed observer/location data

    AstroSchedulerSubData scheduler;                        // Scheduler setup data
    AstroLoggerSubData logger;                              // Logger setup data
    AstroPublisherSubData publisher;                        // Publisher setup data

    AstroSystemData();
    bool toJSON(char *bufferOut, size_t bufferSize) const;
    bool fromJSON(const char *jsonIn);
};

#endif // /ifndef AstroData_H
