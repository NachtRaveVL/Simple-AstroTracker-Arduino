#include "Astruino.h"
#include <cassert>
#include <cmath>
#include <cstring>

static void testUserTargetCheckoutLifetime()
{
    AstroTargetsLibrary library;
    AstroTargetsLibData custom;
    custom.targetType = Astro_TargetType_CustomTarget1;
    std::strcpy(custom.catalogId, "ShopStar");
    std::strcpy(custom.targetName, "Shop Star");
    custom.targetClass = Astro_TargetClass_Star;
    custom.bumpRevisionIfNeeded();

    assert(library.setUserTargetData(&custom));
    const AstroTargetsLibData *checkedOut = library.checkoutTargetsData(Astro_TargetType_CustomTarget1);
    assert(checkedOut != nullptr);

    assert(!library.dropUserTargetData(&custom));
    assert(std::strcmp(checkedOut->targetName, "Shop Star") == 0);

    library.returnTargetsData(checkedOut);
    assert(library.dropUserTargetData(&custom));
    assert(!library.hasUserTargets());
}

static void testJ2000IdentityAndClockStep()
{
    AstroEquatorialCoordinates input(5.25, -12.5);
    AstroEquatorialCoordinates output = astroPrecessJ2000(input, 946728000);
    assert(isFPEqual(output.rightAscensionHours, input.rightAscensionHours));
    assert(isFPEqual(output.declinationDegrees, input.declinationDegrees));

    const double oneSecondDays = astroJulianDate(946728001) - astroJulianDate(946728000);
    assert(std::fabs(oneSecondDays - (1.0 / 86400.0)) < 1.0e-9);
}

static void testCalibrationDegenerateSpan()
{
    AstroCalibrationData calibration;
    calibration.setFromTwoPoints(0.5, 10.0, 0.5, 20.0);
    assert(isFPEqual(calibration.multiplier, 1.0f));
    assert(isFPEqual(calibration.offset, 0.0f));
    assert(isFPEqual(calibration.transform(3.0f), 3.0f));
}

int main()
{
    testUserTargetCheckoutLifetime();
    testJ2000IdentityAndClockStep();
    testCalibrationDegenerateSpan();
    return 0;
}
