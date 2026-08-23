/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Utilities
*/

#ifndef AstroUtils_H
#define AstroUtils_H

#include "AstroDefines.h"

#ifdef ARDUINO
// Simple wrapper class for dealing with RTC modules.
// This class is mainly used to abstract which RTC module is used.
template<typename RTCType>
class AstroRTCWrapper : public AstroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) override { return _rtc.begin(wireInstance); }
    virtual void adjust(const DateTime &dt) override { _rtc.adjust(dt); }
    virtual bool lostPower(void) override { return _rtc.lostPower(); }
    virtual DateTime now() override { return _rtc.now(); }
protected:
    RTCType _rtc;
};

// Specialization for older DS1307 that doesn't have lost power tracking.
template<>
class AstroRTCWrapper<RTC_DS1307> : public AstroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) override;
    virtual void adjust(const DateTime &dt) override;
    virtual bool lostPower(void) override;
    virtual DateTime now() override;
protected:
    RTC_DS1307 _rtc;
};
#endif

// UTC/local time helpers. Timezone offsets follow the sibling-library convention.
extern time_t unixNow();
#ifdef ARDUINO
extern DateTime localNow();
extern time_t unixTime(DateTime localTime);
extern DateTime localTime(time_t unixTime);
#endif

// This will return a non-zero millis time value, so that 0 time values can be reserved for other use.
inline millis_t nzMillis() { millis_t now = millis(); return now ? now : 1; }

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

// For wrapping of values to positive-only moduli range [0, +range), e.g. [0,360) [0,2pi) etc, used in horizontal coordinates and as default wrap mode
template<typename T> inline T wrapBy(T value, T range) { value = value % range; return value >= 0 ? value : value + range; }
// For wrapping of values to positive-and-negative-split moduli range [-range/2,+range/2), e.g. [-180,180) [-pi,pi] etc, used in vertical coordinates
template<typename T> inline T wrapBySplit(T value, T range) { return wrapBy<T>(value + (range / 2), range) - (range / 2); }
// For wrapping of degree angle values to [0,360)
template<typename T> inline T wrapBy360(T value) { return wrapBy<T>(value, 360); }
// For wrapping of degree angle values to [-180,180)
template<typename T> inline T wrapBy180Neg180(T value) { return wrapBySplit<T>(value, 360); }


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

#include "AstroUtils.hpp"

#endif // /ifndef AstroUtils_H
