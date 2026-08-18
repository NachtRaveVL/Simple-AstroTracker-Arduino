/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Coordinates
*/

#include "AstroCoordinates.h"
#include <math.h>


static inline double radians(double degrees) { return degrees * M_PI / 180.0; }
static inline double degrees(double radiansIn) { return radiansIn * 180.0 / M_PI; }

double astroNormalizeDegrees(double value)
{
    value = fmod(value, 360.0);
    if (value < 0.0) { value += 360.0; }
    return value;
}

double astroNormalizeSignedDegrees(double value)
{
    value = astroNormalizeDegrees(value);
    return value > 180.0 ? value - 360.0 : value;
}

double astroJulianDate(int64_t unixTime)
{
    return 2440587.5 + ((double)unixTime / 86400.0);
}

double astroGreenwichSiderealDegrees(int64_t unixTime)
{
    double jd = astroJulianDate(unixTime);
    double t = (jd - 2451545.0) / 36525.0;
    double gmst = 280.46061837 + 360.98564736629 * (jd - 2451545.0) +
                  0.000387933 * t * t - (t * t * t) / 38710000.0;
    return astroNormalizeDegrees(gmst);
}

double astroLocalSiderealDegrees(int64_t unixTime, double longitudeDegrees)
{
    return astroNormalizeDegrees(astroGreenwichSiderealDegrees(unixTime) + longitudeDegrees);
}

AstroEquatorialCoordinates astroPrecessJ2000(const AstroEquatorialCoordinates &coordinates, int64_t unixTime)
{
    double jd = astroJulianDate(unixTime);
    double t = (jd - 2451545.0) / 36525.0;
    double t2 = t * t;
    double t3 = t2 * t;
    double zeta = radians((2306.2181 * t + 0.30188 * t2 + 0.017998 * t3) / 3600.0);
    double z = radians((2306.2181 * t + 1.09468 * t2 + 0.018203 * t3) / 3600.0);
    double theta = radians((2004.3109 * t - 0.42665 * t2 - 0.041833 * t3) / 3600.0);

    double ra = radians(coordinates.rightAscensionHours * 15.0);
    double dec = radians(coordinates.declinationDegrees);
    double a = cos(dec) * sin(ra + zeta);
    double b = cos(theta) * cos(dec) * cos(ra + zeta) - sin(theta) * sin(dec);
    double c = sin(theta) * cos(dec) * cos(ra + zeta) + cos(theta) * sin(dec);
    double outRa = atan2(a, b) + z;
    double outDec = asin(c);

    return AstroEquatorialCoordinates(astroNormalizeDegrees(degrees(outRa)) / 15.0, degrees(outDec));
}

AstroHorizontalCoordinates astroEquatorialToHorizontal(const AstroEquatorialCoordinates &coordinates, const AstroObserver &observer, int64_t unixTime)
{
    double localSidereal = astroLocalSiderealDegrees(unixTime, observer.longitudeDegrees);
    double hourAngle = radians(astroNormalizeSignedDegrees(localSidereal - coordinates.rightAscensionHours * 15.0));
    double declination = radians(coordinates.declinationDegrees);
    double latitude = radians(observer.latitudeDegrees);

    double altitude = asin(sin(declination) * sin(latitude) + cos(declination) * cos(latitude) * cos(hourAngle));
    double azimuth = atan2(-sin(hourAngle), tan(declination) * cos(latitude) - sin(latitude) * cos(hourAngle));

    return AstroHorizontalCoordinates(degrees(altitude), astroNormalizeDegrees(degrees(azimuth)));
}
