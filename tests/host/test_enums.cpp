#include "Astruino.h"
#include <cstdlib>
#include <iostream>

#define CHECK(cond) do { if (!(cond)) { std::cerr << "FAIL line " << __LINE__ << ": " #cond << '\n'; return 1; } } while (0)

template<class T, class ToString, class FromString>
static bool testEnumRange(int count, T undefinedValue, ToString toString, FromString fromString)
{
    for (int typeIndex = -1; typeIndex <= count; ++typeIndex) {
        T value = typeIndex < 0 ? undefinedValue : (T)typeIndex;
        String stringValue = toString(value, false);
        if (fromString(stringValue) != value) { return false; }
    }
    return true;
}

int main()
{
    CHECK(testEnumRange(Astro_SystemMode_Count, Astro_SystemMode_Undefined, systemModeToString, systemModeFromString));
    CHECK(testEnumRange(Astro_MeasurementMode_Count, Astro_MeasurementMode_Undefined, measurementModeToString, measurementModeFromString));
    CHECK(testEnumRange(Astro_ActuatorType_Count, Astro_ActuatorType_Undefined, actuatorTypeToString, actuatorTypeFromString));
    CHECK(testEnumRange(Astro_SensorType_Count, Astro_SensorType_Undefined, sensorTypeToString, sensorTypeFromString));
    CHECK(testEnumRange(Astro_MountType_Count, Astro_MountType_Unknown, mountTypeToString, mountTypeFromString));
    CHECK(testEnumRange(Astro_RailType_Count, Astro_RailType_Undefined, railTypeToString, railTypeFromString));
    CHECK(testEnumRange(Astro_PinMode_Count, Astro_PinMode_Undefined, pinModeToString, pinModeFromString));
    CHECK(testEnumRange(Astro_EnableMode_Count, Astro_EnableMode_Undefined, enableModeToString, enableModeFromString));
    CHECK(testEnumRange(Astro_UnitsCategory_Count, Astro_UnitsCategory_Undefined, unitsCategoryToString, unitsCategoryFromString));
    CHECK(testEnumRange(Astro_UnitsType_Count, Astro_UnitsType_Undefined, unitsTypeToSymbol, unitsTypeFromSymbol));
    CHECK(testEnumRange(Astro_TargetClass_Count, Astro_TargetClass_Unknown, targetClassToString, targetClassFromString));
    CHECK(testEnumRange(Astro_TargetType_Count, Astro_TargetType_Undefined, targetTypeToString, targetTypeFromString));
    CHECK(unitsTypeFromSymbol(String("J/s")) == Astro_UnitsType_Power_Wattage);

    CHECK(targetTypeFromString("M0") == Astro_TargetType_Undefined);
    CHECK(targetTypeFromString("M111") == Astro_TargetType_Undefined);

    std::cout << "Enum conversion tests passed\n";
    return 0;
}
