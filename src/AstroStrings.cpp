/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Strings
*/

#include "AstroStrings.h"
#include <string.h>

static char _blank = '\000';
const char *AStr_Blank = &_blank;

static uint16_t _strDataAddress = (uint16_t)-1;
static AstroString _strDataFilePrefix;

void beginStringsFromEEPROM(uint16_t dataAddress)
{
    _strDataAddress = dataAddress;
}

void beginStringsFromSDCard(const AstroString &dataFilePrefix)
{
    _strDataFilePrefix = dataFilePrefix;
}

AstroString stringFromPGM(Astro_String strNum)
{
    // External-data addresses are retained here to mirror the sibling libraries.
    // Platform storage adapters may override these sources without changing callers.
    (void)_strDataAddress;
    (void)_strDataFilePrefix;

#ifndef ASTRO_DISABLE_BUILTIN_DATA
    return stringFromPGMAddr(pgmAddrForStr(strNum));
#else
    (void)strNum;
    return AstroString();
#endif
}

AstroString stringFromPGMAddr(const char *flashStr)
{
    if (!flashStr) { return AstroString(); }
#ifdef ARDUINO
    AstroString retVal;
    retVal.reserve(strlen_P(flashStr) + 1);
    char buffer[ASTRO_STRING_BUFFER_SIZE] = {0};
    strncpy_P(buffer, flashStr, sizeof(buffer) - 1);
    retVal += buffer;
    while (strlen_P(flashStr) >= sizeof(buffer)) {
        flashStr += sizeof(buffer) - 1;
        memset(buffer, 0, sizeof(buffer));
        strncpy_P(buffer, flashStr, sizeof(buffer) - 1);
        retVal += buffer;
    }
    return retVal;
#else
    return AstroString(flashStr);
#endif
}

#ifndef ASTRO_DISABLE_BUILTIN_DATA

const char *pgmAddrForStr(Astro_String strNum)
{
    switch (strNum) {
        case AStr_Tracking: {
            static const char flashStr[] PROGMEM = "Tracking";
            return flashStr;
        }
        case AStr_Balancing: {
            static const char flashStr[] PROGMEM = "Balancing";
            return flashStr;
        }
        case AStr_Manual: {
            static const char flashStr[] PROGMEM = "Manual";
            return flashStr;
        }
        case AStr_Enum_Count: {
            static const char flashStr[] PROGMEM = "Count";
            return flashStr;
        }
        case AStr_Undefined: {
            static const char flashStr[] PROGMEM = "Undefined";
            return flashStr;
        }
        case AStr_Imperial: {
            static const char flashStr[] PROGMEM = "Imperial";
            return flashStr;
        }
        case AStr_Metric: {
            static const char flashStr[] PROGMEM = "Metric";
            return flashStr;
        }
        case AStr_Scientific: {
            static const char flashStr[] PROGMEM = "Scientific";
            return flashStr;
        }
        case AStr_MountAxis: {
            static const char flashStr[] PROGMEM = "MountAxis";
            return flashStr;
        }
        case AStr_Cover: {
            static const char flashStr[] PROGMEM = "Cover";
            return flashStr;
        }
        case AStr_DewHeater: {
            static const char flashStr[] PROGMEM = "DewHeater";
            return flashStr;
        }
        case AStr_CameraCooler: {
            static const char flashStr[] PROGMEM = "CameraCooler";
            return flashStr;
        }
        case AStr_Fan: {
            static const char flashStr[] PROGMEM = "Fan";
            return flashStr;
        }
        case AStr_Focuser: {
            static const char flashStr[] PROGMEM = "Focuser";
            return flashStr;
        }
        case AStr_Generic: {
            static const char flashStr[] PROGMEM = "Generic";
            return flashStr;
        }
        case AStr_Temperature: {
            static const char flashStr[] PROGMEM = "Temperature";
            return flashStr;
        }
        case AStr_Humidity: {
            static const char flashStr[] PROGMEM = "Humidity";
            return flashStr;
        }
        case AStr_Position: {
            static const char flashStr[] PROGMEM = "Position";
            return flashStr;
        }
        case AStr_LimitSwitch: {
            static const char flashStr[] PROGMEM = "LimitSwitch";
            return flashStr;
        }
        case AStr_Rain: {
            static const char flashStr[] PROGMEM = "Rain";
            return flashStr;
        }
        case AStr_WindSpeed: {
            static const char flashStr[] PROGMEM = "WindSpeed";
            return flashStr;
        }
        case AStr_Light: {
            static const char flashStr[] PROGMEM = "Light";
            return flashStr;
        }
        case AStr_Voltage: {
            static const char flashStr[] PROGMEM = "Voltage";
            return flashStr;
        }
        case AStr_Current: {
            static const char flashStr[] PROGMEM = "Current";
            return flashStr;
        }
        case AStr_CameraTemperature: {
            static const char flashStr[] PROGMEM = "CameraTemperature";
            return flashStr;
        }
        case AStr_Equatorial: {
            static const char flashStr[] PROGMEM = "Equatorial";
            return flashStr;
        }
        case AStr_AltAzimuth: {
            static const char flashStr[] PROGMEM = "AltAzimuth";
            return flashStr;
        }
        case AStr_SingleAxis: {
            static const char flashStr[] PROGMEM = "SingleAxis";
            return flashStr;
        }
        case AStr_DC3V3: {
            static const char flashStr[] PROGMEM = "DC3V3";
            return flashStr;
        }
        case AStr_DC5V: {
            static const char flashStr[] PROGMEM = "DC5V";
            return flashStr;
        }
        case AStr_DC12V: {
            static const char flashStr[] PROGMEM = "DC12V";
            return flashStr;
        }
        case AStr_DC24V: {
            static const char flashStr[] PROGMEM = "DC24V";
            return flashStr;
        }
        case AStr_DigitalInput: {
            static const char flashStr[] PROGMEM = "DigitalInput";
            return flashStr;
        }
        case AStr_DigitalInputPullUp: {
            static const char flashStr[] PROGMEM = "DigitalInputPullUp";
            return flashStr;
        }
        case AStr_DigitalInputPullDown: {
            static const char flashStr[] PROGMEM = "DigitalInputPullDown";
            return flashStr;
        }
        case AStr_DigitalOutput: {
            static const char flashStr[] PROGMEM = "DigitalOutput";
            return flashStr;
        }
        case AStr_DigitalOutputPushPull: {
            static const char flashStr[] PROGMEM = "DigitalOutputPushPull";
            return flashStr;
        }
        case AStr_AnalogInput: {
            static const char flashStr[] PROGMEM = "AnalogInput";
            return flashStr;
        }
        case AStr_AnalogOutput: {
            static const char flashStr[] PROGMEM = "AnalogOutput";
            return flashStr;
        }
        case AStr_Highest: {
            static const char flashStr[] PROGMEM = "Highest";
            return flashStr;
        }
        case AStr_Lowest: {
            static const char flashStr[] PROGMEM = "Lowest";
            return flashStr;
        }
        case AStr_Average: {
            static const char flashStr[] PROGMEM = "Average";
            return flashStr;
        }
        case AStr_Multiply: {
            static const char flashStr[] PROGMEM = "Multiply";
            return flashStr;
        }
        case AStr_InOrder: {
            static const char flashStr[] PROGMEM = "InOrder";
            return flashStr;
        }
        case AStr_RevOrder: {
            static const char flashStr[] PROGMEM = "RevOrder";
            return flashStr;
        }
        case AStr_Raw: {
            static const char flashStr[] PROGMEM = "Raw";
            return flashStr;
        }
        case AStr_Angle: {
            static const char flashStr[] PROGMEM = "Angle";
            return flashStr;
        }
        case AStr_Distance: {
            static const char flashStr[] PROGMEM = "Distance";
            return flashStr;
        }
        case AStr_Percentile: {
            static const char flashStr[] PROGMEM = "Percentile";
            return flashStr;
        }
        case AStr_Speed: {
            static const char flashStr[] PROGMEM = "Speed";
            return flashStr;
        }
        case AStr_Power: {
            static const char flashStr[] PROGMEM = "Power";
            return flashStr;
        }
        case AStr_N1: {
            static const char flashStr[] PROGMEM = "1";
            return flashStr;
        }
        case AStr_deg: {
            static const char flashStr[] PROGMEM = "deg";
            return flashStr;
        }
        case AStr_rad: {
            static const char flashStr[] PROGMEM = "rad";
            return flashStr;
        }
        case AStr_m: {
            static const char flashStr[] PROGMEM = "m";
            return flashStr;
        }
        case AStr_ft: {
            static const char flashStr[] PROGMEM = "ft";
            return flashStr;
        }
        case AStr_Percent: {
            static const char flashStr[] PROGMEM = "%";
            return flashStr;
        }
        case AStr_mPers: {
            static const char flashStr[] PROGMEM = "m/s";
            return flashStr;
        }
        case AStr_ftPers: {
            static const char flashStr[] PROGMEM = "ft/s";
            return flashStr;
        }
        case AStr_C: {
            static const char flashStr[] PROGMEM = "C";
            return flashStr;
        }
        case AStr_F: {
            static const char flashStr[] PROGMEM = "F";
            return flashStr;
        }
        case AStr_K: {
            static const char flashStr[] PROGMEM = "K";
            return flashStr;
        }
        case AStr_PercentRH: {
            static const char flashStr[] PROGMEM = "%RH";
            return flashStr;
        }
        case AStr_W: {
            static const char flashStr[] PROGMEM = "W";
            return flashStr;
        }
        case AStr_V: {
            static const char flashStr[] PROGMEM = "V";
            return flashStr;
        }
        case AStr_A: {
            static const char flashStr[] PROGMEM = "A";
            return flashStr;
        }
        case AStr_Star: {
            static const char flashStr[] PROGMEM = "Star";
            return flashStr;
        }
        case AStr_OpenCluster: {
            static const char flashStr[] PROGMEM = "OpenCluster";
            return flashStr;
        }
        case AStr_GlobularCluster: {
            static const char flashStr[] PROGMEM = "GlobularCluster";
            return flashStr;
        }
        case AStr_Nebula: {
            static const char flashStr[] PROGMEM = "Nebula";
            return flashStr;
        }
        case AStr_PlanetaryNebula: {
            static const char flashStr[] PROGMEM = "PlanetaryNebula";
            return flashStr;
        }
        case AStr_Galaxy: {
            static const char flashStr[] PROGMEM = "Galaxy";
            return flashStr;
        }
        case AStr_SolarSystem: {
            static const char flashStr[] PROGMEM = "SolarSystem";
            return flashStr;
        }
        case AStr_Other: {
            static const char flashStr[] PROGMEM = "Other";
            return flashStr;
        }
        case AStr_DayStorage: {
            static const char flashStr[] PROGMEM = "DayStorage";
            return flashStr;
        }
        case AStr_NightObserving: {
            static const char flashStr[] PROGMEM = "NightObserving";
            return flashStr;
        }
        case AStr_SafeStowed: {
            static const char flashStr[] PROGMEM = "SafeStowed";
            return flashStr;
        }
        case AStr_DayStowed: {
            static const char flashStr[] PROGMEM = "DayStowed";
            return flashStr;
        }
        case AStr_Deploying: {
            static const char flashStr[] PROGMEM = "Deploying";
            return flashStr;
        }
        case AStr_Cooling: {
            static const char flashStr[] PROGMEM = "Cooling";
            return flashStr;
        }
        case AStr_Slewing: {
            static const char flashStr[] PROGMEM = "Slewing";
            return flashStr;
        }
        case AStr_Settling: {
            static const char flashStr[] PROGMEM = "Settling";
            return flashStr;
        }
        case AStr_Observing: {
            static const char flashStr[] PROGMEM = "Observing";
            return flashStr;
        }
        case AStr_Warming: {
            static const char flashStr[] PROGMEM = "Warming";
            return flashStr;
        }
        case AStr_Stowing: {
            static const char flashStr[] PROGMEM = "Stowing";
            return flashStr;
        }
        case AStr_Fault: {
            static const char flashStr[] PROGMEM = "Fault";
            return flashStr;
        }
        case AStr_Empty: {
            static const char flashStr[] PROGMEM = "";
            return flashStr;
        }
        case AStr_Astruino: {
            static const char flashStr[] PROGMEM = "Astruino";
            return flashStr;
        }
        case AStr_AstruinoDotCfg: {
            static const char flashStr[] PROGMEM = "Astruino.cfg";
            return flashStr;
        }
        case AStr_EnvironmentReport: {
            static const char flashStr[] PROGMEM = "Environment report";
            return flashStr;
        }
        case AStr_ObservationStarted: {
            static const char flashStr[] PROGMEM = "Observation started";
            return flashStr;
        }
        case AStr_ObservationStopped: {
            static const char flashStr[] PROGMEM = "Observation stopped";
            return flashStr;
        }
        case AStr_CoverOpening: {
            static const char flashStr[] PROGMEM = "Cover opening";
            return flashStr;
        }
        case AStr_CoverClosing: {
            static const char flashStr[] PROGMEM = "Cover closing";
            return flashStr;
        }
        case AStr_SystemDataSaved: {
            static const char flashStr[] PROGMEM = "System data saved";
            return flashStr;
        }
        case AStr_SystemUptimeColon: {
            static const char flashStr[] PROGMEM = "System uptime: ";
            return flashStr;
        }
        case AStr_AllocationFailure: {
            static const char flashStr[] PROGMEM = "Allocation failure";
            return flashStr;
        }
        case AStr_InvalidParameter: {
            static const char flashStr[] PROGMEM = "Invalid parameter";
            return flashStr;
        }
        case AStr_ImportFailure: {
            static const char flashStr[] PROGMEM = "Import failure";
            return flashStr;
        }
        case AStr_ExportFailure: {
            static const char flashStr[] PROGMEM = "Export failure";
            return flashStr;
        }
        case AStr_OperationFailure: {
            static const char flashStr[] PROGMEM = "Operation failure";
            return flashStr;
        }
        case AStr_AlreadyInitialized: {
            static const char flashStr[] PROGMEM = "Already initialized";
            return flashStr;
        }
        case AStr_NotYetInitialized: {
            static const char flashStr[] PROGMEM = "Not yet initialized";
            return flashStr;
        }
        case AStr_null: {
            static const char flashStr[] PROGMEM = "null";
            return flashStr;
        }
        case AStr_csv: {
            static const char flashStr[] PROGMEM = "csv";
            return flashStr;
        }
        case AStr_dat: {
            static const char flashStr[] PROGMEM = "dat";
            return flashStr;
        }
        case AStr_txt: {
            static const char flashStr[] PROGMEM = "txt";
            return flashStr;
        }
        case AStr_Colon: {
            static const char flashStr[] PROGMEM = ": ";
            return flashStr;
        }
        case AStr_Blank2: {
            static const char flashStr[] PROGMEM = "  ";
            return flashStr;
        }
        case AStr_Count:
            break;
    }
    return AStr_Blank;
}

#endif // /ifndef ASTRO_DISABLE_BUILTIN_DATA
