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
    AstroTargetsLibrary library;

    const AstroTargetsLibData *m31a = library.checkoutTargetsData(Astro_TargetType_M31);
    check(m31a != nullptr, "M31 lookup");
    check(std::strcmp(m31a->catalogId, "M31") == 0, "M31 id");
    check(std::strcmp(m31a->targetName, "Andromeda Galaxy") == 0, "M31 name string pool");
    check(std::fabs(m31a->getJ2000Coordinates().rightAscensionHours - 0.7122) < 0.01, "M31 RA");
    check(std::fabs(m31a->getJ2000Coordinates().declinationDegrees - 41.2692) < 0.02, "M31 DEC");

    const AstroTargetsLibData *m31b = library.checkoutTargetsData(Astro_TargetType_M31);
    check(m31a == m31b, "cache returns same checked-out target data");
    library.returnTargetsData(m31a);
    library.returnTargetsData(m31b);

    for (int type = Astro_TargetType_M1; type <= Astro_TargetType_M110; ++type) {
        const AstroTargetsLibData *data = library.checkoutTargetsData((Astro_TargetType)type);
        check(data != nullptr, "all Messier entries load");
        AstroEquatorialCoordinates coordinates = data->getJ2000Coordinates();
        check(coordinates.rightAscensionHours >= 0.0 && coordinates.rightAscensionHours < 24.0, "Messier RA range");
        check(coordinates.declinationDegrees >= -90.0 && coordinates.declinationDegrees <= 90.0, "Messier DEC range");
        library.returnTargetsData(data);
    }

    const AstroTargetsLibData *sirius = library.checkoutTargetsData(Astro_TargetType_Sirius);
    check(sirius && std::strcmp(sirius->targetName, "Sirius") == 0, "bright star name");
    check(sirius->targetClass == Astro_TargetClass_Star, "bright star class");
    library.returnTargetsData(sirius);

    const AstroTargetsLibData *moon = library.checkoutTargetsData(Astro_TargetType_Moon);
    check(moon && moon->movingTarget, "moon moving target");
    AstroEquatorialCoordinates moonNow = moon->getCoordinates(1787011200);
    check(moonNow.rightAscensionHours >= 0.0 && moonNow.rightAscensionHours < 24.0, "moon resolve through target data");
    library.returnTargetsData(moon);

    const AstroTargetsLibData *m42 = library.checkoutTargetsData(Astro_TargetType_M42);
    check(m42 != nullptr, "M42 lookup for serialization");
    StaticJsonDocument<384> doc;
    JsonObject object = doc.to<JsonObject>();
    m42->toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();
    AstroTargetsLibData decoded;
    decoded.fromJSONObject(objectConst);
    check(decoded.targetType == Astro_TargetType_M42 && std::strcmp(decoded.targetName, "Orion Nebula") == 0,
          "target serialization round trip");
    library.returnTargetsData(m42);

    for (int type = Astro_TargetType_Sun; type < Astro_TargetType_Count; ++type) {
        Astro_TargetType targetType = (Astro_TargetType)type;
        String targetString = targetTypeToString(targetType);
        check(targetTypeFromString(targetString) == targetType, "target string round trip");
    }

    AstroTargetsLibData custom;
    custom.targetType = Astro_TargetType_CustomTarget1;
    std::strcpy(custom.catalogId, "GarageStar");
    std::strcpy(custom.targetName, "Garage Star");
    custom.rightAscensionSeconds = 3600;
    custom.declinationArcseconds = 7200;
    custom.targetClass = Astro_TargetClass_Star;
    custom.bumpRevisionIfNeeded();
    check(library.setUserTargetData(&custom), "set custom target");
    const AstroTargetsLibData *customOut = library.checkoutTargetsData(Astro_TargetType_CustomTarget1);
    check(customOut && std::strcmp(customOut->targetName, "Garage Star") == 0, "custom target checkout");
    library.returnTargetsData(customOut);
    check(library.dropUserTargetData(&custom), "drop custom target");

    std::cout << "PASS catalog" << std::endl;
    return 0;
}
