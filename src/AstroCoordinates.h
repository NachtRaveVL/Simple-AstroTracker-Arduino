/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Coordinates
*/

#ifndef AstroCoordinates_H
#define AstroCoordinates_H

#include "AstroCompat.h"

// Equatorial Coordinates
// Right ascension and declination coordinate pair.
struct AstroEquatorialCoordinates {
    double rightAscensionHours;                              // Right ascension, in hours
    double declinationDegrees;                               // Declination, in degrees

    AstroEquatorialCoordinates(double rightAscensionHoursIn = 0.0, double declinationDegreesIn = 0.0)
        : rightAscensionHours(rightAscensionHoursIn), declinationDegrees(declinationDegreesIn)
    { ; }
};

// Horizontal Coordinates
// Local altitude and azimuth coordinate pair.
struct AstroHorizontalCoordinates {
    double altitudeDegrees;                                  // Altitude, in degrees
    double azimuthDegrees;                                   // Azimuth, in degrees

    AstroHorizontalCoordinates(double altitudeDegreesIn = 0.0, double azimuthDegreesIn = 0.0)
        : altitudeDegrees(altitudeDegreesIn), azimuthDegrees(azimuthDegreesIn)
    { ; }
};

// Observer Location
// Fixed geographic observer location used for celestial coordinate conversion.
struct AstroObserver {
    double latitudeDegrees;                                  // Observer latitude, in degrees
    double longitudeDegrees;                                 // Observer longitude, in degrees
    double elevationMeters;                                  // Observer elevation above sea level, in meters

    AstroObserver(double latitudeDegreesIn = 0.0, double longitudeDegreesIn = 0.0, double elevationMetersIn = 0.0)
        : latitudeDegrees(latitudeDegreesIn), longitudeDegrees(longitudeDegreesIn), elevationMeters(elevationMetersIn)
    { ; }
};

extern double astroJulianDate(int64_t unixTime);
extern double astroGreenwichSiderealDegrees(int64_t unixTime);
extern double astroLocalSiderealDegrees(int64_t unixTime, double longitudeDegrees);
extern AstroEquatorialCoordinates astroPrecessJ2000(const AstroEquatorialCoordinates &coordinates, int64_t unixTime);
extern AstroHorizontalCoordinates astroEquatorialToHorizontal(const AstroEquatorialCoordinates &coordinates, const AstroObserver &observer, int64_t unixTime);
extern double astroNormalizeDegrees(double degrees);
extern double astroNormalizeSignedDegrees(double degrees);

#endif // /ifndef AstroCoordinates_H
