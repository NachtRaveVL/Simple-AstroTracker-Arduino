/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Core Logic
*/

#ifndef AstroCoreLogic_H
#define AstroCoreLogic_H

#include "AstroDefines.h"
#include <stdint.h>
#include <stddef.h>
#include <math.h>

// Returns elapsed unsigned time while remaining safe across 32-bit timer rollover.
inline uint32_t astroElapsedTime(uint32_t now, uint32_t start)
{
    return (uint32_t)(now - start);
}

// Returns true once the requested unsigned duration has elapsed.
inline bool astroHasElapsed(uint32_t now, uint32_t start, uint32_t duration)
{
    return astroElapsedTime(now, start) >= duration;
}

// Returns whichever signed value has the greater absolute magnitude.
inline float astroLargerMagnitude(float current, float candidate)
{
    return fabsf(candidate) > fabsf(current) ? candidate : current;
}

// Converts a signed target offset into reverse, stopped, or forward direction.
inline int astroDirectionForOffset(float offset, float epsilon = 0.000001f)
{
    return fabsf(offset) <= epsilon ? 0 : offset > 0.0f ? 1 : -1;
}

// Estimates signed travel distance from actuator speed, powered time, and direction.
inline float astroTravelDistanceForTime(float speedPerMinute, uint32_t timeMillis, int direction)
{
    if (!direction || fabsf(speedPerMinute) <= 0.000001f || !timeMillis) { return 0.0f; }
    return fabsf(speedPerMinute) * (timeMillis / 60000.0f) * (direction > 0 ? 1.0f : -1.0f);
}

// Removes the expected coast interval from a move without shortening very brief moves below zero.
inline uint32_t astroPoweredTravelTime(uint32_t totalTravelTimeMillis, uint32_t coastTimeMillis)
{
    return totalTravelTimeMillis > coastTimeMillis ? totalTravelTimeMillis - coastTimeMillis
                                                    : totalTravelTimeMillis;
}

// Returns true when an incremental motor should hold position for alignment or expected coasting.
inline bool astroShouldHoldIncrementalMotor(float offset, int lastDirection,
                                            float alignedRange, float nearbyRange,
                                            float coastDistance)
{
    const float magnitude = fabsf(offset);
    if (magnitude <= fabsf(alignedRange)) { return true; }

    const int targetDirection = astroDirectionForOffset(offset);
    if (!lastDirection || !targetDirection) { return false; }

    const float coast = fabsf(coastDistance);
    if (targetDirection == lastDirection) {
        return magnitude <= coast;
    }

    return magnitude <= fmaxf(fabsf(nearbyRange), coast);
}

// Returns the shortest signed angular delta while wrapping around the 0/360 boundary.
inline float astroWrappedAngleDelta(float value, float reference)
{
    float delta = fmodf(value - reference, 360.0f);
    if (delta > 180.0f) { delta -= 360.0f; }
    else if (delta < -180.0f) { delta += 360.0f; }
    return delta;
}

// Adds a correction sample to the running arithmetic mean.
inline float astroUpdateRunningCorrection(float correction, float sample, uint16_t sampleCount)
{
    if (!sampleCount) { return correction; }
    return correction + ((sample - correction) / sampleCount);
}

// Applies a minimum stable time before accepting a changed binary sensor state.
inline bool astroUpdateStableBinaryState(bool acceptedState, bool sampledState, uint32_t now,
                                         uint16_t stableTimeMillis, bool &pendingState,
                                         bool &hasPendingState, uint32_t &pendingStateStart)
{
    if (sampledState == acceptedState) {
        hasPendingState = false;
    } else if (!stableTimeMillis) {
        hasPendingState = false;
        return sampledState;
    } else if (!hasPendingState || pendingState != sampledState) {
        pendingState = sampledState;
        pendingStateStart = now;
        hasPendingState = true;
    } else if (astroHasElapsed(now, pendingStateStart, stableTimeMillis)) {
        hasPendingState = false;
        return sampledState;
    }

    return acceptedState;
}

// Binary record copy/skip plan used for append-only serialized data migrations.
struct AstroBinaryDataReadPlan
{
    size_t copyBytes;                                       // Bytes that can be copied into the current data structure
    size_t skipBytes;                                       // Unknown trailing bytes that must be skipped in the serialized record
};

// Builds a safe copy plan for older, current, or newer append-only binary records.
inline AstroBinaryDataReadPlan astroBinaryDataReadPlan(size_t serializedSize, size_t currentSize, size_t baseSize)
{
    if (serializedSize < baseSize || currentSize < baseSize) { return {0, 0}; }

    const size_t serializedRemaining = serializedSize - baseSize;
    const size_t currentRemaining = currentSize - baseSize;
    const size_t copyBytes = serializedRemaining < currentRemaining ? serializedRemaining : currentRemaining;
    return {copyBytes, serializedRemaining - copyBytes};
}

// Equatorial Coordinates
// Right ascension and declination coordinate pair.
struct AstroEquatorialCoordinates {
    double rightAscensionHours;                             // Right ascension, in hours
    double declinationDegrees;                              // Declination, in degrees

    AstroEquatorialCoordinates(double rightAscensionHoursIn = 0.0, double declinationDegreesIn = 0.0)
        : rightAscensionHours(rightAscensionHoursIn), declinationDegrees(declinationDegreesIn)
    { ; }
};

// Horizontal Coordinates
// Local altitude and azimuth coordinate pair.
struct AstroHorizontalCoordinates {
    double altitudeDegrees;                                 // Altitude, in degrees
    double azimuthDegrees;                                  // Azimuth, in degrees

    AstroHorizontalCoordinates(double altitudeDegreesIn = 0.0, double azimuthDegreesIn = 0.0)
        : altitudeDegrees(altitudeDegreesIn), azimuthDegrees(azimuthDegreesIn)
    { ; }
};

// Observer Location
// Fixed geographic observer location used for celestial coordinate conversion.
struct AstroObserver {
    double latitudeDegrees;                                 // Observer latitude, in degrees
    double longitudeDegrees;                                // Observer longitude, in degrees
    double elevationMeters;                                 // Observer elevation above sea level, in meters

    AstroObserver(double latitudeDegreesIn = 0.0, double longitudeDegreesIn = 0.0, double elevationMetersIn = 0.0)
        : latitudeDegrees(latitudeDegreesIn), longitudeDegrees(longitudeDegreesIn), elevationMeters(elevationMetersIn)
    { ; }
};

extern double astroJulianDate(int64_t unixTime);
extern double astroGreenwichSiderealDegrees(int64_t unixTime);
extern double astroLocalSiderealDegrees(int64_t unixTime, double longitudeDegrees);
extern AstroEquatorialCoordinates astroPrecessJ2000(const AstroEquatorialCoordinates &coordinates, int64_t unixTime);
extern AstroHorizontalCoordinates astroEquatorialToHorizontal(const AstroEquatorialCoordinates &coordinates, const AstroObserver &observer, int64_t unixTime);

// Resolves a moving built-in solar-system target into equatorial coordinates for the supplied UTC time.
// Uses lightweight orbital approximations intended for practical DIY pointing/tracking.
extern bool astroResolveSolarSystemTarget(Astro_TargetType targetType, int64_t unixTime,
                                          AstroEquatorialCoordinates *coordinatesOut);

#endif // /ifndef AstroCoreLogic_H
