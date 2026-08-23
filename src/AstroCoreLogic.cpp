/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Core Logic
*/

#include "Astruino.h"
#include <math.h>

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
    double zeta = (2306.2181 * t + 0.30188 * t2 + 0.017998 * t3) / 3600.0;
    double z = (2306.2181 * t + 1.09468 * t2 + 0.018203 * t3) / 3600.0;
    double theta = (2004.3109 * t - 0.42665 * t2 - 0.041833 * t3) / 3600.0;
    astroConvertUnits(zeta, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &zeta);
    astroConvertUnits(z, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &z);
    astroConvertUnits(theta, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &theta);

    double ra = coordinates.rightAscensionHours * 15.0;
    double dec = coordinates.declinationDegrees;
    astroConvertUnits(ra, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &ra);
    astroConvertUnits(dec, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &dec);
    double a = cos(dec) * sin(ra + zeta);
    double b = cos(theta) * cos(dec) * cos(ra + zeta) - sin(theta) * sin(dec);
    double c = sin(theta) * cos(dec) * cos(ra + zeta) + cos(theta) * sin(dec);
    double outRa = atan2(a, b) + z;
    double outDec = asin(c);

    astroConvertUnits(outRa, Astro_UnitsType_Angle_Radians_2pi, Astro_UnitsType_Angle_Degrees_360, &outRa);
    astroConvertUnits(outDec, Astro_UnitsType_Angle_Radians_2pi, Astro_UnitsType_Angle_Degrees_360, &outDec);
    return AstroEquatorialCoordinates(astroNormalizeDegrees(outRa) / 15.0, outDec);
}

AstroHorizontalCoordinates astroEquatorialToHorizontal(const AstroEquatorialCoordinates &coordinates, const AstroObserver &observer, int64_t unixTime)
{
    double localSidereal = astroLocalSiderealDegrees(unixTime, observer.longitudeDegrees);
    double hourAngle = astroNormalizeSignedDegrees(localSidereal - coordinates.rightAscensionHours * 15.0);
    double declination = coordinates.declinationDegrees;
    double latitude = observer.latitudeDegrees;
    astroConvertUnits(hourAngle, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &hourAngle);
    astroConvertUnits(declination, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &declination);
    astroConvertUnits(latitude, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &latitude);

    double altitude = asin(sin(declination) * sin(latitude) + cos(declination) * cos(latitude) * cos(hourAngle));
    double azimuth = atan2(-sin(hourAngle), tan(declination) * cos(latitude) - sin(latitude) * cos(hourAngle));

    astroConvertUnits(altitude, Astro_UnitsType_Angle_Radians_2pi, Astro_UnitsType_Angle_Degrees_360, &altitude);
    astroConvertUnits(azimuth, Astro_UnitsType_Angle_Radians_2pi, Astro_UnitsType_Angle_Degrees_360, &azimuth);
    return AstroHorizontalCoordinates(altitude, astroNormalizeDegrees(azimuth));
}

struct AstroOrbitalElements {
    double node;
    double inclination;
    double perihelion;
    double axis;
    double eccentricity;
    double anomaly;
};

static double daysFromJ2000(int64_t unixTime)
{
    return astroJulianDate(unixTime) - 2451543.5;
}

static double solveEccentricAnomaly(double meanAnomalyDegrees, double eccentricity)
{
    double m = astroNormalizeDegrees(meanAnomalyDegrees);
    astroConvertUnits(m, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &m);
    double e = m + eccentricity * sin(m) * (1.0 + eccentricity * cos(m));
    for (int i = 0; i < 6; ++i) {
        e -= (e - eccentricity * sin(e) - m) / (1.0 - eccentricity * cos(e));
    }
    return e;
}

static void orbitalXYZ(const AstroOrbitalElements &elements, double *xOut, double *yOut, double *zOut,
                       double *radiusOut = nullptr, double *longitudeOut = nullptr)
{
    double e = solveEccentricAnomaly(elements.anomaly, elements.eccentricity);
    double xv = cos(e) - elements.eccentricity;
    double yv = sqrt(1.0 - elements.eccentricity * elements.eccentricity) * sin(e);
    double v = atan2(yv, xv);
    double r = elements.axis * sqrt(xv * xv + yv * yv);
    double n = elements.node;
    double i = elements.inclination;
    double perihelion = elements.perihelion;
    astroConvertUnits(n, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &n);
    astroConvertUnits(i, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &i);
    astroConvertUnits(perihelion, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &perihelion);
    double wv = v + perihelion;
    double x = r * (cos(n) * cos(wv) - sin(n) * sin(wv) * cos(i));
    double y = r * (sin(n) * cos(wv) + cos(n) * sin(wv) * cos(i));
    double z = r * sin(wv) * sin(i);
    if (xOut) { *xOut = x; }
    if (yOut) { *yOut = y; }
    if (zOut) { *zOut = z; }
    if (radiusOut) { *radiusOut = r; }
    if (longitudeOut) { *longitudeOut = atan2(y, x); }
}

static AstroOrbitalElements sunElements(double d)
{
    return {0.0, 0.0, 282.9404 + 4.70935e-5 * d, 1.0,
            0.016709 - 1.151e-9 * d, 356.0470 + 0.9856002585 * d};
}

static AstroOrbitalElements planetElements(Astro_TargetType targetType, double d)
{
    switch (targetType) {
        case Astro_TargetType_Mercury: return {48.3313 + 3.24587e-5*d, 7.0047 + 5.00e-8*d, 29.1241 + 1.01444e-5*d, 0.387098, 0.205635 + 5.59e-10*d, 168.6562 + 4.0923344368*d};
        case Astro_TargetType_Venus:   return {76.6799 + 2.46590e-5*d, 3.3946 + 2.75e-8*d, 54.8910 + 1.38374e-5*d, 0.723330, 0.006773 - 1.302e-9*d, 48.0052 + 1.6021302244*d};
        case Astro_TargetType_Mars:    return {49.5574 + 2.11081e-5*d, 1.8497 - 1.78e-8*d, 286.5016 + 2.92961e-5*d, 1.523688, 0.093405 + 2.516e-9*d, 18.6021 + 0.5240207766*d};
        case Astro_TargetType_Jupiter: return {100.4542 + 2.76854e-5*d, 1.3030 - 1.557e-7*d, 273.8777 + 1.64505e-5*d, 5.20256, 0.048498 + 4.469e-9*d, 19.8950 + 0.0830853001*d};
        case Astro_TargetType_Saturn:  return {113.6634 + 2.38980e-5*d, 2.4886 - 1.081e-7*d, 339.3939 + 2.97661e-5*d, 9.55475, 0.055546 - 9.499e-9*d, 316.9670 + 0.0334442282*d};
        case Astro_TargetType_Uranus:  return {74.0005 + 1.3978e-5*d, 0.7733 + 1.9e-8*d, 96.6612 + 3.0565e-5*d, 19.18171 - 1.55e-8*d, 0.047318 + 7.45e-9*d, 142.5905 + 0.011725806*d};
        case Astro_TargetType_Neptune: return {131.7806 + 3.0173e-5*d, 1.7700 - 2.55e-7*d, 272.8461 - 6.027e-6*d, 30.05826 + 3.313e-8*d, 0.008606 + 2.15e-9*d, 260.2471 + 0.005995147*d};
        default: return {0,0,0,0,0,0};
    }
}

static AstroEquatorialCoordinates eclipticToEquatorial(double x, double y, double z, double d)
{
    double obliquity = 23.4393 - 3.563e-7 * d;
    astroConvertUnits(obliquity, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &obliquity);
    double xe = x;
    double ye = y * cos(obliquity) - z * sin(obliquity);
    double ze = y * sin(obliquity) + z * cos(obliquity);
    double ra = atan2(ye, xe);
    double dec = atan2(ze, sqrt(xe * xe + ye * ye));
    astroConvertUnits(ra, Astro_UnitsType_Angle_Radians_2pi, Astro_UnitsType_Angle_Degrees_360, &ra);
    astroConvertUnits(dec, Astro_UnitsType_Angle_Radians_2pi, Astro_UnitsType_Angle_Degrees_360, &dec);
    return AstroEquatorialCoordinates(astroNormalizeDegrees(ra) / 15.0, dec);
}

static AstroEquatorialCoordinates resolveSun(double d)
{
    AstroOrbitalElements sun = sunElements(d);
    double e = solveEccentricAnomaly(sun.anomaly, sun.eccentricity);
    double xv = cos(e) - sun.eccentricity;
    double yv = sqrt(1.0 - sun.eccentricity * sun.eccentricity) * sin(e);
    double v = atan2(yv, xv);
    double r = sqrt(xv*xv + yv*yv);
    double perihelion = sun.perihelion;
    astroConvertUnits(perihelion, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &perihelion);
    double lon = v + perihelion;
    return eclipticToEquatorial(r * cos(lon), r * sin(lon), 0.0, d);
}

static AstroEquatorialCoordinates resolveMoon(double d)
{
    AstroOrbitalElements moon = {125.1228 - 0.0529538083*d, 5.1454, 318.0634 + 0.1643573223*d,
                                 60.2666, 0.054900, 115.3654 + 13.0649929509*d};
    double x, y, z, r, lon;
    orbitalXYZ(moon, &x, &y, &z, &r, &lon);

    double lMoon = astroNormalizeDegrees(moon.node + moon.perihelion + moon.anomaly);
    AstroOrbitalElements sun = sunElements(d);
    double lSun = astroNormalizeDegrees(sun.perihelion + sun.anomaly);
    double mm = astroNormalizeDegrees(moon.anomaly);
    double ms = astroNormalizeDegrees(sun.anomaly);
    double dd = astroNormalizeDegrees(lMoon - lSun);
    double ff = astroNormalizeDegrees(lMoon - moon.node);
    astroConvertUnits(mm, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &mm);
    astroConvertUnits(ms, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &ms);
    astroConvertUnits(dd, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &dd);
    astroConvertUnits(ff, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &ff);

    double lonDeg = lon;
    astroConvertUnits(lonDeg, Astro_UnitsType_Angle_Radians_2pi, Astro_UnitsType_Angle_Degrees_360, &lonDeg);
    lonDeg += -1.274 * sin(mm - 2.0*dd) + 0.658 * sin(2.0*dd) - 0.186 * sin(ms)
              -0.059 * sin(2.0*mm - 2.0*dd) - 0.057 * sin(mm - 2.0*dd + ms)
              +0.053 * sin(mm + 2.0*dd) + 0.046 * sin(2.0*dd - ms)
              +0.041 * sin(mm - ms) - 0.035 * sin(dd) - 0.031 * sin(mm + ms)
              -0.015 * sin(2.0*ff - 2.0*dd) + 0.011 * sin(mm - 4.0*dd);

    double latDeg = atan2(z, sqrt(x*x + y*y));
    astroConvertUnits(latDeg, Astro_UnitsType_Angle_Radians_2pi, Astro_UnitsType_Angle_Degrees_360, &latDeg);
    latDeg += -0.173 * sin(ff - 2.0*dd) - 0.055 * sin(mm - ff - 2.0*dd)
              -0.046 * sin(mm + ff - 2.0*dd) + 0.033 * sin(ff + 2.0*dd)
              +0.017 * sin(2.0*mm + ff);
    r += -0.58 * cos(mm - 2.0*dd) - 0.46 * cos(2.0*dd);

    double lonRad = lonDeg;
    double latRad = latDeg;
    astroConvertUnits(lonRad, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &lonRad);
    astroConvertUnits(latRad, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &latRad);
    double xy = r * cos(latRad);
    return eclipticToEquatorial(xy*cos(lonRad), xy*sin(lonRad), r*sin(latRad), d);
}

bool astroResolveSolarSystemTarget(Astro_TargetType targetType, int64_t unixTime, AstroEquatorialCoordinates *coordinatesOut)
{
    if (!coordinatesOut) { return false; }
    double d = daysFromJ2000(unixTime);
    if (targetType == Astro_TargetType_Sun) {
        *coordinatesOut = resolveSun(d);
        return true;
    }
    if (targetType == Astro_TargetType_Moon) {
        *coordinatesOut = resolveMoon(d);
        return true;
    }
    if (targetType < Astro_TargetType_Mercury || targetType > Astro_TargetType_Neptune) { return false; }

    AstroOrbitalElements sun = sunElements(d);
    double e = solveEccentricAnomaly(sun.anomaly, sun.eccentricity);
    double xv = cos(e) - sun.eccentricity;
    double yv = sqrt(1.0 - sun.eccentricity * sun.eccentricity) * sin(e);
    double v = atan2(yv, xv);
    double r = sqrt(xv*xv + yv*yv);
    double perihelion = sun.perihelion;
    astroConvertUnits(perihelion, Astro_UnitsType_Angle_Degrees_360, Astro_UnitsType_Angle_Radians_2pi, &perihelion);
    double lon = v + perihelion;
    double xs = r * cos(lon);
    double ys = r * sin(lon);

    double xp, yp, zp;
    orbitalXYZ(planetElements(targetType, d), &xp, &yp, &zp);
    *coordinatesOut = eclipticToEquatorial(xp + xs, yp + ys, zp, d);
    return true;
}
