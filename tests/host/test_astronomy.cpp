#include "Astruino.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

static void check(bool condition, const char *message)
{
    if (!condition) { std::cerr << "FAIL: " << message << std::endl; std::exit(1); }
}

int main()
{
    check(std::fabs(astroJulianDate(946728000) - 2451545.0) < 0.00001, "J2000 Julian date");

    AstroObserver observer(49.2827, -123.1207);
    AstroEquatorialCoordinates polaris(2.5303, 89.2641);
    AstroHorizontalCoordinates horizontal = astroEquatorialToHorizontal(polaris, observer, 1787011200);
    check(horizontal.altitudeDegrees > 45.0 && horizontal.altitudeDegrees < 55.0, "Polaris altitude near latitude");
    check(horizontal.azimuthDegrees >= 0.0 && horizontal.azimuthDegrees < 360.0, "Azimuth normalized");

    AstroEquatorialCoordinates m31J2000(2564.0 / 3600.0, 148569.0 / 3600.0);
    AstroEquatorialCoordinates m31Date = astroPrecessJ2000(m31J2000, 1787011200);
    check(std::fabs(m31Date.rightAscensionHours - m31J2000.rightAscensionHours) < 0.1, "Precession RA sane");
    check(std::fabs(m31Date.declinationDegrees - m31J2000.declinationDegrees) < 1.0, "Precession DEC sane");

    for (int id = Astro_Target_Sun; id <= Astro_Target_Neptune; ++id) {
        AstroEquatorialCoordinates coords;
        check(astroResolveSolarSystemTarget((Astro_TargetId)id, 1787011200, &coords), "moving target resolves");
        check(coords.rightAscensionHours >= 0.0 && coords.rightAscensionHours < 24.0, "moving RA range");
        check(coords.declinationDegrees >= -90.0 && coords.declinationDegrees <= 90.0, "moving DEC range");
    }

    AstroEquatorialCoordinates sun;
    check(astroResolveSolarSystemTarget(Astro_Target_Sun, 946728000, &sun), "J2000 sun resolves");
    check(sun.rightAscensionHours > 18.0 && sun.rightAscensionHours < 19.5, "J2000 sun RA plausible");
    check(sun.declinationDegrees < -20.0 && sun.declinationDegrees > -25.0, "J2000 sun DEC plausible");

    std::cout << "PASS astronomy" << std::endl;
    return 0;
}
