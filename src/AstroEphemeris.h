/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Ephemeris
*/

#ifndef AstroEphemeris_H
#define AstroEphemeris_H

#include "AstroCoordinates.h"
#include "AstroDefines.h"

// Resolves a moving built-in solar-system target into equatorial coordinates for the supplied UTC time.
// Uses lightweight orbital approximations intended for practical DIY pointing/tracking.
extern bool astroResolveSolarSystemTarget(Astro_TargetId targetId, int64_t unixTime,
                                          AstroEquatorialCoordinates *coordinatesOut);

#endif // /ifndef AstroEphemeris_H
