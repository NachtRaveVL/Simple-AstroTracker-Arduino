/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Utilities
*/

#include "Astruino.h"
#include "AstroStrings.h"
#include "AstroLogger.h"
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


extern AstroLogger *getLogger();

#ifdef ARDUINO

bool AstroRTCWrapper<RTC_DS1307>::begin(TwoWire *wireInstance) { return _rtc.begin(wireInstance); }
void AstroRTCWrapper<RTC_DS1307>::adjust(const DateTime &dt) { _rtc.adjust(dt); }
bool AstroRTCWrapper<RTC_DS1307>::lostPower(void) { return false; }
DateTime AstroRTCWrapper<RTC_DS1307>::now() { return _rtc.now(); }

#endif

time_t unixNow()
{
#ifdef ARDUINO
    return now();
#else
    return time(nullptr);
#endif
}

#ifdef ARDUINO

time_t unixTime(DateTime localTimeIn)
{
    return localTimeIn.unixtime() - (getController() ? getController()->getTimeZoneOffset() : 0);
}

DateTime localTime(time_t unixTimeIn)
{
    return DateTime((uint32_t)(unixTimeIn + (getController() ? getController()->getTimeZoneOffset() : 0)));
}

DateTime localNow()
{
    return localTime(unixNow());
}

#endif

static AstroString astroAssertLocation(const char *file, const char *function, int line)
{
    char buffer[160];
    snprintf(buffer, sizeof(buffer), "%s:%d in %s", file ? file : "?", line, function ? function : "?");
    return AstroString(buffer);
}

void astroSoftAssert(bool condition, const AstroString &message, const char *file, const char *function, int line)
{
    if (condition) { return; }
    AstroString output = AstroString("Assertion failure: ") + astroAssertLocation(file, function, line);
    #ifdef ARDUINO
    if (message.length()) { output += AstroString(": ") + message; }
#else
    if (!message.empty()) { output += AstroString(": ") + message; }
#endif
    if (getLogger()) { getLogger()->logWarning((int64_t)time(nullptr), output.c_str()); }
}

void astroHardAssert(bool condition, const AstroString &message, const char *file, const char *function, int line)
{
    if (condition) { return; }
    AstroString output = AstroString("Assertion failure HARD: ") + astroAssertLocation(file, function, line);
    #ifdef ARDUINO
    if (message.length()) { output += AstroString(": ") + message; }
#else
    if (!message.empty()) { output += AstroString(": ") + message; }
#endif
    if (getLogger()) { getLogger()->logError((int64_t)time(nullptr), output.c_str()); }
    abort();
}

akey_t astroStringHash(const char *stringIn)
{
    if (!stringIn) { return akey_none; }

    akey_t hash = 5381;
    for (const char *ch = stringIn; *ch; ++ch) {
        hash = (akey_t)(((hash << 5) + hash) + (akey_t)*ch);
    }
    return hash != akey_none ? hash : 5381;
}

AstroString astroPositionIndexToString(aposi_t positionIndex)
{
#ifdef ARDUINO
    return String((int)positionIndex);
#else
    return std::to_string((int)positionIndex);
#endif
}

aposi_t astroPositionIndexFromString(const AstroString &positionIndexStr)
{
#ifdef ARDUINO
    return (aposi_t)positionIndexStr.toInt();
#else
    return (aposi_t)strtol(positionIndexStr.c_str(), nullptr, 10);
#endif
}


AstroString systemModeToString(Astro_SystemMode value, bool excludeSpecial)
{
    switch (value) {
        case Astro_SystemMode_Tracking:
            return SFP(AStr_Tracking);
        case Astro_SystemMode_Balancing:
            return SFP(AStr_Balancing);
        case Astro_SystemMode_Manual:
            return SFP(AStr_Manual);
        case Astro_SystemMode_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_SystemMode_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

// All remaining methods generated from minimum spanning trie

Astro_SystemMode systemModeFromString(const AstroString &value)
{
    switch (value.length() > 0 ? value[0] : '\0') {
        case 'B':
            return Astro_SystemMode_Balancing;
        case 'C':
            return Astro_SystemMode_Count;
        case 'M':
            return Astro_SystemMode_Manual;
        case 'T':
            return Astro_SystemMode_Tracking;
        case 'U':
            return Astro_SystemMode_Undefined;
    }
    return Astro_SystemMode_Undefined;
}

AstroString measurementModeToString(Astro_MeasurementMode value, bool excludeSpecial)
{
    switch (value) {
        case Astro_MeasurementMode_Imperial: return SFP(AStr_Imperial);
        case Astro_MeasurementMode_Metric: return SFP(AStr_Metric);
        case Astro_MeasurementMode_Scientific: return SFP(AStr_Scientific);
        case Astro_MeasurementMode_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_MeasurementMode_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_MeasurementMode measurementModeFromString(const AstroString &value)
{
    switch (value.length() > 0 ? value[0] : '\0') {
        case 'C':
            return Astro_MeasurementMode_Count;
        case 'I':
            return Astro_MeasurementMode_Imperial;
        case 'M':
            return Astro_MeasurementMode_Metric;
        case 'S':
            return Astro_MeasurementMode_Scientific;
        case 'U':
            return Astro_MeasurementMode_Undefined;
    }
    return Astro_MeasurementMode_Undefined;
}

AstroString actuatorTypeToString(Astro_ActuatorType value, bool excludeSpecial)
{
    switch (value) {
        case Astro_ActuatorType_MountAxis: return SFP(AStr_MountAxis);
        case Astro_ActuatorType_Cover: return SFP(AStr_Cover);
        case Astro_ActuatorType_DewHeater: return SFP(AStr_DewHeater);
        case Astro_ActuatorType_CameraCooler: return SFP(AStr_CameraCooler);
        case Astro_ActuatorType_Fan: return SFP(AStr_Fan);
        case Astro_ActuatorType_Focuser: return SFP(AStr_Focuser);
        case Astro_ActuatorType_Generic: return SFP(AStr_Generic);
        case Astro_ActuatorType_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_ActuatorType_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_ActuatorType actuatorTypeFromString(const AstroString &value)
{
    switch (value.length() > 6 ? value[6] : '\0') {
        case '\0':
            switch (value.length() > 2 ? value[2] : '\0') {
                case 'n':
                    return Astro_ActuatorType_Fan;
                case 'u':
                    return Astro_ActuatorType_Count;
                case 'v':
                    return Astro_ActuatorType_Cover;
            }
            return Astro_ActuatorType_Undefined;
        case 'C':
            return Astro_ActuatorType_CameraCooler;
        case 'c':
            return Astro_ActuatorType_Generic;
        case 'n':
            return Astro_ActuatorType_Undefined;
        case 'r':
            return Astro_ActuatorType_Focuser;
        case 't':
            return Astro_ActuatorType_DewHeater;
        case 'x':
            return Astro_ActuatorType_MountAxis;
    }
    return Astro_ActuatorType_Undefined;
}

AstroString sensorTypeToString(Astro_SensorType value, bool excludeSpecial)
{
    switch (value) {
        case Astro_SensorType_Temperature: return SFP(AStr_Temperature);
        case Astro_SensorType_Humidity: return SFP(AStr_Humidity);
        case Astro_SensorType_Position: return SFP(AStr_Position);
        case Astro_SensorType_LimitSwitch: return SFP(AStr_LimitSwitch);
        case Astro_SensorType_Rain: return SFP(AStr_Rain);
        case Astro_SensorType_WindSpeed: return SFP(AStr_WindSpeed);
        case Astro_SensorType_Light: return SFP(AStr_Light);
        case Astro_SensorType_Voltage: return SFP(AStr_Voltage);
        case Astro_SensorType_Current: return SFP(AStr_Current);
        case Astro_SensorType_CameraTemperature: return SFP(AStr_CameraTemperature);
        case Astro_SensorType_Generic: return SFP(AStr_Generic);
        case Astro_SensorType_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_SensorType_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_SensorType sensorTypeFromString(const AstroString &value)
{
    switch (value.length() > 0 ? value[0] : '\0') {
        case 'C':
            switch (value.length() > 1 ? value[1] : '\0') {
                case 'a':
                    return Astro_SensorType_CameraTemperature;
                case 'o':
                    return Astro_SensorType_Count;
                case 'u':
                    return Astro_SensorType_Current;
            }
            return Astro_SensorType_Undefined;
        case 'G':
            return Astro_SensorType_Generic;
        case 'H':
            return Astro_SensorType_Humidity;
        case 'L':
            switch (value.length() > 2 ? value[2] : '\0') {
                case 'g':
                    return Astro_SensorType_Light;
                case 'm':
                    return Astro_SensorType_LimitSwitch;
            }
            return Astro_SensorType_Undefined;
        case 'P':
            return Astro_SensorType_Position;
        case 'R':
            return Astro_SensorType_Rain;
        case 'T':
            return Astro_SensorType_Temperature;
        case 'U':
            return Astro_SensorType_Undefined;
        case 'V':
            return Astro_SensorType_Voltage;
        case 'W':
            return Astro_SensorType_WindSpeed;
    }
    return Astro_SensorType_Undefined;
}

AstroString mountTypeToString(Astro_MountType value, bool excludeSpecial)
{
    switch (value) {
        case Astro_MountType_Equatorial: return SFP(AStr_Equatorial);
        case Astro_MountType_AltAz: return SFP(AStr_AltAz);
        case Astro_MountType_SingleAxis: return SFP(AStr_SingleAxis);
        case Astro_MountType_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_MountType_Unknown: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_MountType mountTypeFromString(const AstroString &value)
{
    switch (value.length() > 0 ? value[0] : '\0') {
        case 'A':
            return Astro_MountType_AltAz;
        case 'C':
            return Astro_MountType_Count;
        case 'E':
            return Astro_MountType_Equatorial;
        case 'S':
            return Astro_MountType_SingleAxis;
        case 'U':
            return Astro_MountType_Unknown;
    }
    return Astro_MountType_Unknown;
}

AstroString railTypeToString(Astro_RailType value, bool excludeSpecial)
{
    switch (value) {
        case Astro_RailType_DC3V3: return SFP(AStr_DC3V3);
        case Astro_RailType_DC5V: return SFP(AStr_DC5V);
        case Astro_RailType_DC12V: return SFP(AStr_DC12V);
        case Astro_RailType_DC24V: return SFP(AStr_DC24V);
        case Astro_RailType_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_RailType_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_RailType railTypeFromString(const AstroString &value)
{
    switch (value.length() > 2 ? value[2] : '\0') {
        case '1':
            return Astro_RailType_DC12V;
        case '2':
            return Astro_RailType_DC24V;
        case '3':
            return Astro_RailType_DC3V3;
        case '5':
            return Astro_RailType_DC5V;
        case 'd':
            return Astro_RailType_Undefined;
        case 'u':
            return Astro_RailType_Count;
    }
    return Astro_RailType_Undefined;
}

AstroString pinModeToString(Astro_PinMode value, bool excludeSpecial)
{
    switch (value) {
        case Astro_PinMode_Digital_Input: return SFP(AStr_DigitalInput);
        case Astro_PinMode_Digital_Input_PullUp: return SFP(AStr_DigitalInputPullUp);
        case Astro_PinMode_Digital_Input_PullDown: return SFP(AStr_DigitalInputPullDown);
        case Astro_PinMode_Digital_Output: return SFP(AStr_DigitalOutput);
        case Astro_PinMode_Digital_Output_PushPull: return SFP(AStr_DigitalOutputPushPull);
        case Astro_PinMode_Analog_Input: return SFP(AStr_AnalogInput);
        case Astro_PinMode_Analog_Output: return SFP(AStr_AnalogOutput);
        case Astro_PinMode_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_PinMode_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_PinMode pinModeFromString(const AstroString &value)
{
    switch (value.length() > 16 ? value[16] : '\0') {
        case '\0':
            switch (value.length() > 7 ? value[7] : '\0') {
                case '\0':
                    return Astro_PinMode_Count;
                case 'I':
                    return Astro_PinMode_Digital_Input;
                case 'O':
                    return Astro_PinMode_Digital_Output;
                case 'e':
                    return Astro_PinMode_Undefined;
                case 'n':
                    return Astro_PinMode_Analog_Input;
                case 'u':
                    return Astro_PinMode_Analog_Output;
            }
            return Astro_PinMode_Undefined;
        case 'D':
            return Astro_PinMode_Digital_Input_PullDown;
        case 'U':
            return Astro_PinMode_Digital_Input_PullUp;
        case 'h':
            return Astro_PinMode_Digital_Output_PushPull;
    }
    return Astro_PinMode_Undefined;
}

AstroString enableModeToString(Astro_EnableMode value, bool excludeSpecial)
{
    switch (value) {
        case Astro_EnableMode_Highest: return SFP(AStr_Highest);
        case Astro_EnableMode_Lowest: return SFP(AStr_Lowest);
        case Astro_EnableMode_Average: return SFP(AStr_Average);
        case Astro_EnableMode_Multiply: return SFP(AStr_Multiply);
        case Astro_EnableMode_InOrder: return SFP(AStr_InOrder);
        case Astro_EnableMode_RevOrder: return SFP(AStr_RevOrder);
        case Astro_EnableMode_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_EnableMode_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_EnableMode enableModeFromString(const AstroString &value)
{
    switch (value.length() > 0 ? value[0] : '\0') {
        case 'A':
            return Astro_EnableMode_Average;
        case 'C':
            return Astro_EnableMode_Count;
        case 'H':
            return Astro_EnableMode_Highest;
        case 'I':
            return Astro_EnableMode_InOrder;
        case 'L':
            return Astro_EnableMode_Lowest;
        case 'M':
            return Astro_EnableMode_Multiply;
        case 'R':
            return Astro_EnableMode_RevOrder;
        case 'U':
            return Astro_EnableMode_Undefined;
    }
    return Astro_EnableMode_Undefined;
}

AstroString unitsCategoryToString(Astro_UnitsCategory value, bool excludeSpecial)
{
    switch (value) {
        case Astro_UnitsCategory_Raw: return SFP(AStr_Raw);
        case Astro_UnitsCategory_Angle: return SFP(AStr_Angle);
        case Astro_UnitsCategory_Distance: return SFP(AStr_Distance);
        case Astro_UnitsCategory_Percentile: return SFP(AStr_Percentile);
        case Astro_UnitsCategory_Speed: return SFP(AStr_Speed);
        case Astro_UnitsCategory_Temperature: return SFP(AStr_Temperature);
        case Astro_UnitsCategory_Humidity: return SFP(AStr_Humidity);
        case Astro_UnitsCategory_Power: return SFP(AStr_Power);
        case Astro_UnitsCategory_Voltage: return SFP(AStr_Voltage);
        case Astro_UnitsCategory_Current: return SFP(AStr_Current);
        case Astro_UnitsCategory_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_UnitsCategory_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_UnitsCategory unitsCategoryFromString(const AstroString &value)
{
    switch (value.length() > 0 ? value[0] : '\0') {
        case 'A':
            return Astro_UnitsCategory_Angle;
        case 'C':
            switch (value.length() > 1 ? value[1] : '\0') {
                case 'o':
                    return Astro_UnitsCategory_Count;
                case 'u':
                    return Astro_UnitsCategory_Current;
            }
            return Astro_UnitsCategory_Undefined;
        case 'D':
            return Astro_UnitsCategory_Distance;
        case 'H':
            return Astro_UnitsCategory_Humidity;
        case 'P':
            switch (value.length() > 1 ? value[1] : '\0') {
                case 'e':
                    return Astro_UnitsCategory_Percentile;
                case 'o':
                    return Astro_UnitsCategory_Power;
            }
            return Astro_UnitsCategory_Undefined;
        case 'R':
            return Astro_UnitsCategory_Raw;
        case 'S':
            return Astro_UnitsCategory_Speed;
        case 'T':
            return Astro_UnitsCategory_Temperature;
        case 'U':
            return Astro_UnitsCategory_Undefined;
        case 'V':
            return Astro_UnitsCategory_Voltage;
    }
    return Astro_UnitsCategory_Undefined;
}

AstroString unitsTypeToSymbol(Astro_UnitsType value, bool excludeSpecial)
{
    switch (value) {
        case Astro_UnitsType_Raw_1: return SFP(AStr_N1);
        case Astro_UnitsType_Angle_Degrees_360: return SFP(AStr_deg);
        case Astro_UnitsType_Angle_Radians_2pi: return SFP(AStr_rad);
        case Astro_UnitsType_Distance_Meters: return SFP(AStr_m);
        case Astro_UnitsType_Distance_Feet: return SFP(AStr_ft);
        case Astro_UnitsType_Percentile_100: return SFP(AStr_Percent);
        case Astro_UnitsType_Speed_MetersPerSec: return SFP(AStr_mPers);
        case Astro_UnitsType_Speed_FeetPerSec: return SFP(AStr_ftPers);
        case Astro_UnitsType_Temperature_Celsius: return SFP(AStr_C);
        case Astro_UnitsType_Temperature_Fahrenheit: return SFP(AStr_F);
        case Astro_UnitsType_Temperature_Kelvin: return SFP(AStr_K);
        case Astro_UnitsType_Humidity_RH: return SFP(AStr_PercentRH);
        case Astro_UnitsType_Power_Wattage: return SFP(AStr_W);
        case Astro_UnitsType_Voltage_Volts: return SFP(AStr_V);
        case Astro_UnitsType_Current_Amperage: return SFP(AStr_A);
        case Astro_UnitsType_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_UnitsType_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_UnitsType unitsTypeFromSymbol(const AstroString &value)
{
    switch (value.length() > 1 ? value[1] : '\0') {
        case '\0':
            switch (value.length() > 0 ? value[0] : '\0') {
                case '%':
                    return Astro_UnitsType_Percentile_100;
                case '1':
                    return Astro_UnitsType_Raw_1;
                case 'A':
                    return Astro_UnitsType_Current_Amperage;
                case 'C':
                    return Astro_UnitsType_Temperature_Celsius;
                case 'F':
                    return Astro_UnitsType_Temperature_Fahrenheit;
                case 'K':
                    return Astro_UnitsType_Temperature_Kelvin;
                case 'V':
                    return Astro_UnitsType_Voltage_Volts;
                case 'W':
                    return Astro_UnitsType_Power_Wattage;
                case 'm':
                    return Astro_UnitsType_Distance_Meters;
            }
            return Astro_UnitsType_Undefined;
        case '/':
            switch (value.length() > 0 ? value[0] : '\0') {
                case 'J':
                    return Astro_UnitsType_Power_Wattage;
                case 'm':
                    return Astro_UnitsType_Speed_MetersPerSec;
            }
            return Astro_UnitsType_Undefined;
        case 'R':
            return Astro_UnitsType_Humidity_RH;
        case 'a':
            return Astro_UnitsType_Angle_Radians_2pi;
        case 'e':
            return Astro_UnitsType_Angle_Degrees_360;
        case 'n':
            return Astro_UnitsType_Undefined;
        case 'o':
            return Astro_UnitsType_Count;
        case 't':
            switch (value.length() > 2 ? value[2] : '\0') {
                case '\0':
                    return Astro_UnitsType_Distance_Feet;
                case '/':
                    return Astro_UnitsType_Speed_FeetPerSec;
            }
            return Astro_UnitsType_Undefined;
    }
    return Astro_UnitsType_Undefined;
}

AstroString targetClassToString(Astro_TargetClass value, bool excludeSpecial)
{
    switch (value) {
        case Astro_TargetClass_Star: return SFP(AStr_Star);
        case Astro_TargetClass_OpenCluster: return SFP(AStr_OpenCluster);
        case Astro_TargetClass_GlobularCluster: return SFP(AStr_GlobularCluster);
        case Astro_TargetClass_Nebula: return SFP(AStr_Nebula);
        case Astro_TargetClass_PlanetaryNebula: return SFP(AStr_PlanetaryNebula);
        case Astro_TargetClass_Galaxy: return SFP(AStr_Galaxy);
        case Astro_TargetClass_SolarSystem: return SFP(AStr_SolarSystem);
        case Astro_TargetClass_Other: return SFP(AStr_Other);
        case Astro_TargetClass_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_TargetClass_Unknown: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_TargetClass targetClassFromString(const AstroString &value)
{
    switch (value.length() > 4 ? value[4] : '\0') {
        case '\0':
            return Astro_TargetClass_Star;
        case 'C':
            return Astro_TargetClass_OpenCluster;
        case 'e':
            return Astro_TargetClass_PlanetaryNebula;
        case 'f':
            return Astro_TargetClass_Unknown;
        case 'l':
            return Astro_TargetClass_Nebula;
        case 'r':
            switch (value.length() > 0 ? value[0] : '\0') {
                case 'O':
                    return Astro_TargetClass_Other;
                case 'S':
                    return Astro_TargetClass_SolarSystem;
            }
            return Astro_TargetClass_Unknown;
        case 't':
            return Astro_TargetClass_Count;
        case 'u':
            return Astro_TargetClass_GlobularCluster;
        case 'x':
            return Astro_TargetClass_Galaxy;
    }
    return Astro_TargetClass_Unknown;
}

AstroString thermalModeToString(Astro_ThermalMode value, bool excludeSpecial)
{
    switch (value) {
        case Astro_ThermalMode_DayStorage: return SFP(AStr_DayStorage);
        case Astro_ThermalMode_NightObserving: return SFP(AStr_NightObserving);
        case Astro_ThermalMode_SafeStowed: return SFP(AStr_SafeStowed);
        case Astro_ThermalMode_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_ThermalMode_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_ThermalMode thermalModeFromString(const AstroString &value)
{
    switch (value.length() > 0 ? value[0] : '\0') {
        case 'C':
            return Astro_ThermalMode_Count;
        case 'D':
            return Astro_ThermalMode_DayStorage;
        case 'N':
            return Astro_ThermalMode_NightObserving;
        case 'S':
            return Astro_ThermalMode_SafeStowed;
        case 'U':
            return Astro_ThermalMode_Undefined;
    }
    return Astro_ThermalMode_Undefined;
}

AstroString schedulerStageToString(Astro_SchedulerStage value, bool excludeSpecial)
{
    switch (value) {
        case Astro_SchedulerStage_DayStowed: return SFP(AStr_DayStowed);
        case Astro_SchedulerStage_Deploying: return SFP(AStr_Deploying);
        case Astro_SchedulerStage_Cooling: return SFP(AStr_Cooling);
        case Astro_SchedulerStage_Slewing: return SFP(AStr_Slewing);
        case Astro_SchedulerStage_Settling: return SFP(AStr_Settling);
        case Astro_SchedulerStage_Observing: return SFP(AStr_Observing);
        case Astro_SchedulerStage_Warming: return SFP(AStr_Warming);
        case Astro_SchedulerStage_Stowing: return SFP(AStr_Stowing);
        case Astro_SchedulerStage_SafeStowed: return SFP(AStr_SafeStowed);
        case Astro_SchedulerStage_Fault: return SFP(AStr_Fault);
        case Astro_SchedulerStage_Count: return !excludeSpecial ? SFP(AStr_Enum_Count) : AstroString();
        case Astro_SchedulerStage_Undefined: break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : AstroString();
}

Astro_SchedulerStage schedulerStageFromString(const AstroString &value)
{
    switch (value.length() > 2 ? value[2] : '\0') {
        case 'd':
            return Astro_SchedulerStage_Undefined;
        case 'e':
            return Astro_SchedulerStage_Slewing;
        case 'f':
            return Astro_SchedulerStage_SafeStowed;
        case 'o':
            switch (value.length() > 0 ? value[0] : '\0') {
                case 'C':
                    return Astro_SchedulerStage_Cooling;
                case 'S':
                    return Astro_SchedulerStage_Stowing;
            }
            return Astro_SchedulerStage_Undefined;
        case 'p':
            return Astro_SchedulerStage_Deploying;
        case 'r':
            return Astro_SchedulerStage_Warming;
        case 's':
            return Astro_SchedulerStage_Observing;
        case 't':
            return Astro_SchedulerStage_Settling;
        case 'u':
            switch (value.length() > 0 ? value[0] : '\0') {
                case 'C':
                    return Astro_SchedulerStage_Count;
                case 'F':
                    return Astro_SchedulerStage_Fault;
            }
            return Astro_SchedulerStage_Undefined;
        case 'y':
            return Astro_SchedulerStage_DayStowed;
    }
    return Astro_SchedulerStage_Undefined;
}

Astro_UnitsCategory unitsCategoryForType(Astro_UnitsType units)
{
    switch (units) {
        case Astro_UnitsType_Raw_1: return Astro_UnitsCategory_Raw;
        case Astro_UnitsType_Angle_Degrees_360:
        case Astro_UnitsType_Angle_Radians_2pi: return Astro_UnitsCategory_Angle;
        case Astro_UnitsType_Distance_Meters:
        case Astro_UnitsType_Distance_Feet: return Astro_UnitsCategory_Distance;
        case Astro_UnitsType_Percentile_100: return Astro_UnitsCategory_Percentile;
        case Astro_UnitsType_Speed_MetersPerSec:
        case Astro_UnitsType_Speed_FeetPerSec: return Astro_UnitsCategory_Speed;
        case Astro_UnitsType_Temperature_Celsius:
        case Astro_UnitsType_Temperature_Fahrenheit:
        case Astro_UnitsType_Temperature_Kelvin: return Astro_UnitsCategory_Temperature;
        case Astro_UnitsType_Humidity_RH: return Astro_UnitsCategory_Humidity;
        case Astro_UnitsType_Power_Wattage: return Astro_UnitsCategory_Power;
        case Astro_UnitsType_Voltage_Volts: return Astro_UnitsCategory_Voltage;
        case Astro_UnitsType_Current_Amperage: return Astro_UnitsCategory_Current;
        default: return Astro_UnitsCategory_Undefined;
    }
}

Astro_UnitsType defaultUnits(Astro_UnitsCategory category, Astro_MeasurementMode measurementMode)
{
    switch (category) {
        case Astro_UnitsCategory_Raw: return Astro_UnitsType_Raw_1;
        case Astro_UnitsCategory_Angle:
            return measurementMode == Astro_MeasurementMode_Scientific ? Astro_UnitsType_Angle_Radians_2pi : Astro_UnitsType_Angle_Degrees_360;
        case Astro_UnitsCategory_Distance:
            return measurementMode == Astro_MeasurementMode_Imperial ? Astro_UnitsType_Distance_Feet : Astro_UnitsType_Distance_Meters;
        case Astro_UnitsCategory_Percentile: return Astro_UnitsType_Percentile_100;
        case Astro_UnitsCategory_Speed:
            return measurementMode == Astro_MeasurementMode_Imperial ? Astro_UnitsType_Speed_FeetPerSec : Astro_UnitsType_Speed_MetersPerSec;
        case Astro_UnitsCategory_Temperature:
            return measurementMode == Astro_MeasurementMode_Imperial ? Astro_UnitsType_Temperature_Fahrenheit :
                   measurementMode == Astro_MeasurementMode_Scientific ? Astro_UnitsType_Temperature_Kelvin : Astro_UnitsType_Temperature_Celsius;
        case Astro_UnitsCategory_Humidity: return Astro_UnitsType_Humidity_RH;
        case Astro_UnitsCategory_Power: return Astro_UnitsType_Power_Wattage;
        case Astro_UnitsCategory_Voltage: return Astro_UnitsType_Voltage_Volts;
        case Astro_UnitsCategory_Current: return Astro_UnitsType_Current_Amperage;
        default: return Astro_UnitsType_Undefined;
    }
}

bool astroConvertUnits(double valueIn, Astro_UnitsType unitsIn, Astro_UnitsType unitsOut,
                       double *valueOut, double convertParam)
{
    if (!valueOut) { return false; }
    if (unitsIn == unitsOut) { *valueOut = valueIn; return true; }
    if (unitsCategoryForType(unitsIn) != unitsCategoryForType(unitsOut)) {
        if (unitsIn == Astro_UnitsType_Power_Wattage && unitsOut == Astro_UnitsType_Current_Amperage && !isFPEqual(convertParam, 0.0)) {
            *valueOut = valueIn / convertParam;
            return true;
        }
        if (unitsIn == Astro_UnitsType_Current_Amperage && unitsOut == Astro_UnitsType_Power_Wattage && !isFPEqual(convertParam, 0.0)) {
            *valueOut = valueIn * convertParam;
            return true;
        }
        return false;
    }

    switch (unitsIn) {
        case Astro_UnitsType_Angle_Degrees_360:
            if (unitsOut == Astro_UnitsType_Angle_Radians_2pi) { *valueOut = valueIn * M_PI / 180.0; return true; }
            break;
        case Astro_UnitsType_Angle_Radians_2pi:
            if (unitsOut == Astro_UnitsType_Angle_Degrees_360) { *valueOut = valueIn * 180.0 / M_PI; return true; }
            break;
        case Astro_UnitsType_Distance_Meters:
            if (unitsOut == Astro_UnitsType_Distance_Feet) { *valueOut = valueIn * 3.28083989501312; return true; }
            break;
        case Astro_UnitsType_Distance_Feet:
            if (unitsOut == Astro_UnitsType_Distance_Meters) { *valueOut = valueIn * 0.3048; return true; }
            break;
        case Astro_UnitsType_Speed_MetersPerSec:
            if (unitsOut == Astro_UnitsType_Speed_FeetPerSec) { *valueOut = valueIn * 3.28083989501312; return true; }
            break;
        case Astro_UnitsType_Speed_FeetPerSec:
            if (unitsOut == Astro_UnitsType_Speed_MetersPerSec) { *valueOut = valueIn * 0.3048; return true; }
            break;
        case Astro_UnitsType_Temperature_Celsius:
            if (unitsOut == Astro_UnitsType_Temperature_Fahrenheit) { *valueOut = valueIn * 1.8 + 32.0; return true; }
            if (unitsOut == Astro_UnitsType_Temperature_Kelvin) { *valueOut = valueIn + 273.15; return true; }
            break;
        case Astro_UnitsType_Temperature_Fahrenheit:
            if (unitsOut == Astro_UnitsType_Temperature_Celsius) { *valueOut = (valueIn - 32.0) / 1.8; return true; }
            if (unitsOut == Astro_UnitsType_Temperature_Kelvin) { *valueOut = (valueIn + 459.67) * 5.0 / 9.0; return true; }
            break;
        case Astro_UnitsType_Temperature_Kelvin:
            if (unitsOut == Astro_UnitsType_Temperature_Celsius) { *valueOut = valueIn - 273.15; return true; }
            if (unitsOut == Astro_UnitsType_Temperature_Fahrenheit) { *valueOut = valueIn * 9.0 / 5.0 - 459.67; return true; }
            break;
        default:
            break;
    }

    return false;
}

static const char *astroJSONFindValue(const char *jsonIn, const char *key)
{
    if (!jsonIn || !key || !*key) { return nullptr; }

    const size_t keyLength = strlen(key);
    const char *cursor = jsonIn;
    while ((cursor = strchr(cursor, '"'))) {
        ++cursor;
        if (strncmp(cursor, key, keyLength) == 0 && cursor[keyLength] == '"') {
            cursor += keyLength + 1;
            while (*cursor && isspace((unsigned char)*cursor)) { ++cursor; }
            if (*cursor != ':') { continue; }
            ++cursor;
            while (*cursor && isspace((unsigned char)*cursor)) { ++cursor; }
            return cursor;
        }
        while (*cursor && *cursor != '"') { ++cursor; }
        if (*cursor) { ++cursor; }
    }
    return nullptr;
}

bool astroJSONGetString(const char *jsonIn, const char *key, char *valueOut, size_t valueSize)
{
    if (!valueOut || !valueSize) { return false; }
    valueOut[0] = '\0';

    const char *value = astroJSONFindValue(jsonIn, key);
    if (!value || *value != '"') { return false; }
    ++value;

    size_t outIndex = 0;
    while (*value && *value != '"') {
        char next = *value++;
        if (next == '\\' && *value) {
            switch (*value++) {
                case '"': next = '"'; break;
                case '\\': next = '\\'; break;
                case 'n': next = '\n'; break;
                case 'r': next = '\r'; break;
                case 't': next = '\t'; break;
                default: return false;
            }
        }
        if (outIndex + 1 >= valueSize) { return false; }
        valueOut[outIndex++] = next;
    }
    if (*value != '"') { return false; }
    valueOut[outIndex] = '\0';
    return true;
}

bool astroJSONGetLong(const char *jsonIn, const char *key, long *valueOut)
{
    if (!valueOut) { return false; }
    const char *value = astroJSONFindValue(jsonIn, key);
    if (!value) { return false; }
    char *end = nullptr;
    long parsed = strtol(value, &end, 10);
    if (end == value) { return false; }
    *valueOut = parsed;
    return true;
}

bool astroJSONGetUnsignedLong(const char *jsonIn, const char *key, unsigned long *valueOut)
{
    if (!valueOut) { return false; }
    const char *value = astroJSONFindValue(jsonIn, key);
    if (!value || *value == '-') { return false; }
    char *end = nullptr;
    unsigned long parsed = strtoul(value, &end, 10);
    if (end == value) { return false; }
    *valueOut = parsed;
    return true;
}

bool astroJSONGetDouble(const char *jsonIn, const char *key, double *valueOut)
{
    if (!valueOut) { return false; }
    const char *value = astroJSONFindValue(jsonIn, key);
    if (!value) { return false; }
    char *end = nullptr;
    double parsed = strtod(value, &end);
    if (end == value) { return false; }
    *valueOut = parsed;
    return true;
}

bool astroJSONGetBool(const char *jsonIn, const char *key, bool *valueOut)
{
    if (!valueOut) { return false; }
    const char *value = astroJSONFindValue(jsonIn, key);
    if (!value) { return false; }
    if (strncmp(value, "true", 4) == 0 || *value == '1') { *valueOut = true; return true; }
    if (strncmp(value, "false", 5) == 0 || *value == '0') { *valueOut = false; return true; }
    return false;
}

Astro_UnitsType baseUnits(Astro_UnitsType units)
{
    switch (units) {
        case Astro_UnitsType_Speed_MetersPerSec: return Astro_UnitsType_Distance_Meters;
        case Astro_UnitsType_Speed_FeetPerSec: return Astro_UnitsType_Distance_Feet;
        default: return Astro_UnitsType_Undefined;
    }
}

Astro_UnitsType rateUnits(Astro_UnitsType units)
{
    switch (units) {
        case Astro_UnitsType_Distance_Meters: return Astro_UnitsType_Speed_MetersPerSec;
        case Astro_UnitsType_Distance_Feet: return Astro_UnitsType_Speed_FeetPerSec;
        default: return Astro_UnitsType_Undefined;
    }
}

akey_t stringHash(const AstroString &stringIn)
{
    return astroStringHash(stringIn.c_str());
}

AstroString addressToString(uintptr_t address)
{
    char buffer[(sizeof(uintptr_t) * 2) + 3];
    snprintf(buffer, sizeof(buffer), "0x%0*llx", (int)(sizeof(uintptr_t) * 2), (unsigned long long)address);
    return AstroString(buffer);
}

AstroString charsToString(const char *charsIn, size_t length)
{
    if (!charsIn || !length) { return AstroString(); }
#ifdef ARDUINO
    AstroString retVal;
    retVal.reserve(length + 1);
    for (size_t index = 0; index < length && charsIn[index]; ++index) { retVal += charsIn[index]; }
    return retVal;
#else
    size_t actualLength = 0;
    while (actualLength < length && charsIn[actualLength]) { ++actualLength; }
    return AstroString(charsIn, actualLength);
#endif
}

AstroString roundToString(double value, unsigned int additionalDecPlaces)
{
    const unsigned int decimals = 2U + additionalDecPlaces;
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.*f", (int)decimals, value);
    char *end = buffer + strlen(buffer) - 1;
    while (end > buffer && *end == '0') { *end-- = '\0'; }
    if (end > buffer && *end == '.') { *end = '\0'; }
    return AstroString(buffer);
}

AstroString measurementToString(double value, Astro_UnitsType units, unsigned int additionalDecPlaces)
{
    AstroString retVal = roundToString(value, additionalDecPlaces);
    AstroString symbol = unitsTypeToSymbol(units, true);
    if (symbol.length()) {
#ifdef ARDUINO
        retVal += ' ';
        retVal += symbol;
#else
        retVal += " ";
        retVal += symbol;
#endif
    }
    return retVal;
}

int occurrencesInString(const AstroString &stringIn, char singleChar)
{
    int count = 0;
    for (size_t index = 0; index < stringIn.length(); ++index) {
        if (stringIn[index] == singleChar) { ++count; }
    }
    return count;
}

int occurrencesInString(const AstroString &stringIn, const AstroString &subString)
{
    if (subString.length() == 0) { return 0; }
    int count = 0;
#ifdef ARDUINO
    int position = stringIn.indexOf(subString);
    while (position >= 0) {
        ++count;
        position = stringIn.indexOf(subString, position + subString.length());
    }
#else
    size_t position = stringIn.find(subString);
    while (position != AstroString::npos) {
        ++count;
        position = stringIn.find(subString, position + subString.length());
    }
#endif
    return count;
}

AstroString hexStringFromBytes(const uint8_t *bytesIn, size_t length)
{
    if (!bytesIn || !length) { return AstroString(); }
    static const char hexChars[] = "0123456789abcdef";
#ifdef ARDUINO
    AstroString retVal;
    retVal.reserve((length * 2) + 1);
    for (size_t index = 0; index < length; ++index) {
        retVal += hexChars[(bytesIn[index] >> 4) & 0x0f];
        retVal += hexChars[bytesIn[index] & 0x0f];
    }
    return retVal;
#else
    AstroString retVal;
    retVal.reserve(length * 2);
    for (size_t index = 0; index < length; ++index) {
        retVal.push_back(hexChars[(bytesIn[index] >> 4) & 0x0f]);
        retVal.push_back(hexChars[bytesIn[index] & 0x0f]);
    }
    return retVal;
#endif
}

bool hexStringToBytes(const AstroString &stringIn, uint8_t *bytesOut, size_t length)
{
    if (!bytesOut || stringIn.length() < length * 2) { return false; }
    for (size_t index = 0; index < length; ++index) {
        char byteChars[3] = {stringIn[index * 2], stringIn[index * 2 + 1], '\0'};
        char *end = nullptr;
        unsigned long value = strtoul(byteChars, &end, 16);
        if (!end || *end) { return false; }
        bytesOut[index] = (uint8_t)value;
    }
    return true;
}

bool checkPinIsDigital(pintype_t pin)
{
    return pin >= 0;
}

bool checkPinIsAnalogInput(pintype_t pin)
{
#ifdef ARDUINO
#ifdef NUM_ANALOG_INPUTS
    if (pin < 0) { return false; }
#if defined(A0)
    return pin >= (pintype_t)A0 && pin < (pintype_t)(A0 + NUM_ANALOG_INPUTS);
#else
    return checkPinIsDigital(pin);
#endif
#else
    return checkPinIsDigital(pin);
#endif
#else
    return pin >= 0;
#endif
}

bool checkPinIsAnalogOutput(pintype_t pin)
{
#ifdef ARDUINO
#ifdef NUM_ANALOG_OUTPUTS
    return NUM_ANALOG_OUTPUTS > 0 && pin >= 0;
#else
    return checkPinIsDigital(pin);
#endif
#else
    return pin >= 0;
#endif
}

Astro_DirectionMode directionFromSignedValue(double value)
{
    return isFPEqual(value, 0.0) ? Astro_DirectionMode_Stop :
           value > 0.0 ? Astro_DirectionMode_Forward : Astro_DirectionMode_Reverse;
}

double signedValueFromDirection(Astro_DirectionMode direction, double magnitude)
{
    magnitude = fabs(magnitude);
    return direction == Astro_DirectionMode_Forward ? magnitude :
           direction == Astro_DirectionMode_Reverse ? -magnitude : 0.0;
}
