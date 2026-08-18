#include "Astruino.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

static void check(bool condition, const char *message)
{
    if (!condition) { std::cerr << "FAIL: " << message << std::endl; std::exit(1); }
}

int main()
{
    check(astroLib.getLoadedBookCount() == 0, "cache starts empty");

    const AstroTargetData *m31a = astroLib.checkoutTargetData(Astro_Target_M31);
    check(m31a != nullptr, "M31 lookup");
    check(std::strcmp(m31a->id, "M31") == 0, "M31 id");
    check(std::strcmp(m31a->targetName, "Andromeda Galaxy") == 0, "M31 name string pool");
    check(std::fabs(m31a->getJ2000Coordinates().rightAscensionHours - 0.7122) < 0.01, "M31 RA");
    check(std::fabs(m31a->getJ2000Coordinates().declinationDegrees - 41.2692) < 0.02, "M31 DEC");
    check(astroLib.getCheckoutCount(Astro_Target_M31) == 1, "first checkout count");

    const AstroTargetData *m31b = astroLib.checkoutTargetData(Astro_Target_M31);
    check(m31a == m31b, "cache returns same target book");
    check(astroLib.getCheckoutCount(Astro_Target_M31) == 2, "second checkout count");
    astroLib.returnTargetData(m31a);
    check(astroLib.getCheckoutCount(Astro_Target_M31) == 1, "return decrements count");
    astroLib.returnTargetData(m31b);
    check(astroLib.getCheckoutCount(Astro_Target_M31) == 0, "final return unloads book");

    for (int i = Astro_Target_M1; i <= Astro_Target_M110; ++i) {
        const AstroTargetData *data = astroLib.checkoutTargetData((Astro_TargetId)i);
        check(data != nullptr, "all Messier entries load");
        AstroEquatorialCoordinates coordinates = data->getJ2000Coordinates();
        check(coordinates.rightAscensionHours >= 0.0 && coordinates.rightAscensionHours < 24.0, "Messier RA range");
        check(coordinates.declinationDegrees >= -90.0 && coordinates.declinationDegrees <= 90.0, "Messier DEC range");
        astroLib.returnTargetData(data);
    }

    const AstroTargetData *sirius = astroLib.checkoutTargetData(Astro_Target_Sirius);
    check(sirius && std::strcmp(sirius->targetName, "Sirius") == 0, "bright star name");
    check(sirius->targetClass == Astro_TargetClass_Star, "bright star class");
    astroLib.returnTargetData(sirius);

    const AstroTargetData *moon = astroLib.checkoutTargetData(Astro_Target_Moon);
    check(moon && moon->movingTarget, "moon moving target");
    AstroEquatorialCoordinates moonNow = moon->getCoordinates(1787011200);
    check(moonNow.rightAscensionHours >= 0.0 && moonNow.rightAscensionHours < 24.0, "moon resolve through target object");
    astroLib.returnTargetData(moon);

    const AstroTargetData *m42 = astroLib.checkoutTargetData(Astro_Target_M42);
    char json[256];
    check(m42 && m42->toJSON(json, sizeof(json)), "target serialization");
    check(std::strstr(json, "Orion Nebula") != nullptr, "serialization contains name");
    AstroTargetData decoded;
    check(decoded.fromJSON(json), "target deserialization");
    check(decoded.targetId == Astro_Target_M42 && std::strcmp(decoded.targetName, "Orion Nebula") == 0, "serialization round trip");
    astroLib.returnTargetData(m42);

    char targetString[ASTRO_TARGET_NAME_MAXSIZE];
    for (int id = Astro_Target_Sun; id <= Astro_Target_Dubhe; ++id) {
        check(astroTargetIdToString((Astro_TargetId)id, targetString, sizeof(targetString)), "target string export");
        check(astroTargetIdFromString(targetString) == id, "target string optimized round trip");
    }

    AstroTargetData custom;
    custom.targetId = Astro_Target_Custom1;
    std::strcpy(custom.id, "GarageStar");
    std::strcpy(custom.targetName, "Garage Star");
    custom.rightAscensionSeconds = 3600;
    custom.declinationArcseconds = 7200;
    custom.targetClass = Astro_TargetClass_Star;
    custom.modified = true;
    check(astroLib.setUserTargetData(&custom), "set custom target");
    const AstroTargetData *customOut = astroLib.checkoutTargetData(Astro_Target_Custom1);
    check(customOut && std::strcmp(customOut->targetName, "Garage Star") == 0, "custom target checkout");
    astroLib.returnTargetData(customOut);
    check(astroLib.dropUserTargetData(&custom), "drop custom target");

    std::cout << "PASS catalog" << std::endl;
    return 0;
}
