// Enum conversion tests script - mainly for dev purposes

#include <Astruino.h>

static int failures = 0;

#define TEST_ENUM_RANGE(prefix, countValue, undefinedValue, toStringFn, fromStringFn) \
    do { \
        for (int typeIndex = -1; typeIndex <= (int)(countValue); ++typeIndex) { \
            prefix value = typeIndex < 0 ? undefinedValue : (prefix)typeIndex; \
            AstroString typeString = toStringFn(value); \
            prefix retValue = fromStringFn(typeString); \
            if (value != retValue) { \
                Serial.print(F("Conversion failure: ")); \
                Serial.print(typeIndex); \
                Serial.print(F(" -> ")); \
                Serial.println(typeString.c_str()); \
                ++failures; \
            } \
        } \
    } while (0)

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("=BEGIN="));

    TEST_ENUM_RANGE(Astro_SystemMode, Astro_SystemMode_Count, Astro_SystemMode_Undefined, systemModeToString, systemModeFromString);
    TEST_ENUM_RANGE(Astro_MeasurementMode, Astro_MeasurementMode_Count, Astro_MeasurementMode_Undefined, measurementModeToString, measurementModeFromString);
    TEST_ENUM_RANGE(Astro_ActuatorType, Astro_ActuatorType_Count, Astro_ActuatorType_Undefined, actuatorTypeToString, actuatorTypeFromString);
    TEST_ENUM_RANGE(Astro_SensorType, Astro_SensorType_Count, Astro_SensorType_Undefined, sensorTypeToString, sensorTypeFromString);
    TEST_ENUM_RANGE(Astro_MountType, Astro_MountType_Count, Astro_MountType_Unknown, mountTypeToString, mountTypeFromString);
    TEST_ENUM_RANGE(Astro_RailType, Astro_RailType_Count, Astro_RailType_Undefined, railTypeToString, railTypeFromString);
    TEST_ENUM_RANGE(Astro_PinMode, Astro_PinMode_Count, Astro_PinMode_Undefined, pinModeToString, pinModeFromString);
    TEST_ENUM_RANGE(Astro_EnableMode, Astro_EnableMode_Count, Astro_EnableMode_Undefined, enableModeToString, enableModeFromString);
    TEST_ENUM_RANGE(Astro_UnitsCategory, Astro_UnitsCategory_Count, Astro_UnitsCategory_Undefined, unitsCategoryToString, unitsCategoryFromString);
    TEST_ENUM_RANGE(Astro_UnitsType, Astro_UnitsType_Count, Astro_UnitsType_Undefined, unitsTypeToSymbol, unitsTypeFromSymbol);
    TEST_ENUM_RANGE(Astro_TargetClass, Astro_TargetClass_Count, Astro_TargetClass_Unknown, targetClassToString, targetClassFromString);
    TEST_ENUM_RANGE(Astro_ThermalMode, Astro_ThermalMode_Count, Astro_ThermalMode_Undefined, thermalModeToString, thermalModeFromString);
    TEST_ENUM_RANGE(Astro_SchedulerStage, Astro_SchedulerStage_Count, Astro_SchedulerStage_Undefined, schedulerStageToString, schedulerStageFromString);

    if (unitsTypeFromSymbol(AstroString("J/s")) != Astro_UnitsType_Power_Wattage) { ++failures; }

    char targetName[ASTRO_TARGET_NAME_MAXSIZE];
    for (unsigned int targetIndex = 0; targetIndex < Astro_Target_Count; ++targetIndex) {
        Astro_TargetId targetId = (Astro_TargetId)targetIndex;
        if (!astroTargetIdToString(targetId, targetName, sizeof(targetName)) ||
            astroTargetIdFromString(targetName) != targetId) {
            Serial.print(F("Target conversion failure: "));
            Serial.println(targetIndex);
            ++failures;
        }
    }

    Serial.print(F("Failures: "));
    Serial.println(failures);
    Serial.println(F("=FINISH="));
}

void loop()
{ ; }
