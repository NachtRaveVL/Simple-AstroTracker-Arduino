/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Utilities
*/

#ifndef AstroUtils_H
#define AstroUtils_H

#include "AstroDefines.h"

// Debug assertion helpers used by ASTRO_SOFT_ASSERT / ASTRO_HARD_ASSERT when enabled.
extern void astroSoftAssert(bool condition, const AstroString &message, const char *file, const char *function, int line);
extern void astroHardAssert(bool condition, const AstroString &message, const char *file, const char *function, int line);

extern akey_t astroStringHash(const char *stringIn);
extern AstroString astroPositionIndexToString(aposi_t positionIndex);
extern aposi_t astroPositionIndexFromString(const AstroString &positionIndexStr);

extern AstroString systemModeToString(Astro_SystemMode value, bool excludeSpecial = false);
extern Astro_SystemMode systemModeFromString(const AstroString &value);
extern AstroString measurementModeToString(Astro_MeasurementMode value, bool excludeSpecial = false);
extern Astro_MeasurementMode measurementModeFromString(const AstroString &value);
extern AstroString actuatorTypeToString(Astro_ActuatorType value, bool excludeSpecial = false);
extern Astro_ActuatorType actuatorTypeFromString(const AstroString &value);
extern AstroString sensorTypeToString(Astro_SensorType value, bool excludeSpecial = false);
extern Astro_SensorType sensorTypeFromString(const AstroString &value);
extern AstroString mountTypeToString(Astro_MountType value, bool excludeSpecial = false);
extern Astro_MountType mountTypeFromString(const AstroString &value);
extern AstroString railTypeToString(Astro_RailType value, bool excludeSpecial = false);
extern Astro_RailType railTypeFromString(const AstroString &value);
extern AstroString pinModeToString(Astro_PinMode value, bool excludeSpecial = false);
extern Astro_PinMode pinModeFromString(const AstroString &value);
extern AstroString enableModeToString(Astro_EnableMode value, bool excludeSpecial = false);
extern Astro_EnableMode enableModeFromString(const AstroString &value);
extern AstroString unitsCategoryToString(Astro_UnitsCategory value, bool excludeSpecial = false);
extern Astro_UnitsCategory unitsCategoryFromString(const AstroString &value);
extern AstroString unitsTypeToSymbol(Astro_UnitsType value, bool excludeSpecial = false);
extern Astro_UnitsType unitsTypeFromSymbol(const AstroString &value);
extern AstroString targetClassToString(Astro_TargetClass value, bool excludeSpecial = false);
extern Astro_TargetClass targetClassFromString(const AstroString &value);
extern AstroString thermalModeToString(Astro_ThermalMode value, bool excludeSpecial = false);
extern Astro_ThermalMode thermalModeFromString(const AstroString &value);
extern AstroString schedulerStageToString(Astro_SchedulerStage value, bool excludeSpecial = false);
extern Astro_SchedulerStage schedulerStageFromString(const AstroString &value);

extern Astro_UnitsCategory unitsCategoryForType(Astro_UnitsType units);
extern Astro_UnitsType defaultUnits(Astro_UnitsCategory category,
                                    Astro_MeasurementMode measurementMode = Astro_MeasurementMode_Metric);
// Returns the distance/base units associated with a rate unit.
extern Astro_UnitsType baseUnits(Astro_UnitsType units);
// Returns the corresponding rate units for a distance/base unit.
extern Astro_UnitsType rateUnits(Astro_UnitsType units);
extern bool astroConvertUnits(double valueIn, Astro_UnitsType unitsIn, Astro_UnitsType unitsOut,
                              double *valueOut, double convertParam = 0.0);


// Common formatting/string helpers
extern akey_t stringHash(const AstroString &stringIn);
extern AstroString addressToString(uintptr_t address);
extern AstroString charsToString(const char *charsIn, size_t length);
extern AstroString roundToString(double value, unsigned int additionalDecPlaces = 0);
extern AstroString measurementToString(double value, Astro_UnitsType units, unsigned int additionalDecPlaces = 0);
extern int occurrencesInString(const AstroString &stringIn, char singleChar);
extern int occurrencesInString(const AstroString &stringIn, const AstroString &subString);
extern AstroString hexStringFromBytes(const uint8_t *bytesIn, size_t length);
extern bool hexStringToBytes(const AstroString &stringIn, uint8_t *bytesOut, size_t length);

// Pin capability helpers used by factories and pin-data validation.
extern bool checkPinIsDigital(pintype_t pin);
extern bool checkPinIsAnalogInput(pintype_t pin);
extern bool checkPinIsAnalogOutput(pintype_t pin);

// Direction helpers.
extern Astro_DirectionMode directionFromSignedValue(double value);
extern double signedValueFromDirection(Astro_DirectionMode direction, double magnitude = 1.0);

// JSON field helpers
// These helpers perform key-based extraction from the compact JSON emitted by Astruino data objects.
// Field order is intentionally ignored so serialized objects remain stable when fields are rearranged.
extern bool astroJSONGetString(const char *jsonIn, const char *key, char *valueOut, size_t valueSize);
extern bool astroJSONGetLong(const char *jsonIn, const char *key, long *valueOut);
extern bool astroJSONGetUnsignedLong(const char *jsonIn, const char *key, unsigned long *valueOut);
extern bool astroJSONGetDouble(const char *jsonIn, const char *key, double *valueOut);
extern bool astroJSONGetBool(const char *jsonIn, const char *key, bool *valueOut);

#include "AstroUtils.hpp"

#endif // /ifndef AstroUtils_H
