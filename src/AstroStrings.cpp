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
        case AStr_ColonSpace: {
            static const char flashStr[] PROGMEM = ": ";
            return flashStr;
        }
        case AStr_Blank2: {
            static const char flashStr[] PROGMEM = "  ";
            return flashStr;
        }
        case AStr_Space: {
            static const char flashStr[] PROGMEM = " ";
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
        case AStr_null: {
            static const char flashStr[] PROGMEM = "null";
        case AStr_Disabled: {
            static const char flashStr[] PROGMEM = "Disabled";
            return flashStr;
        }
        case AStr_Empty: {
            static const char flashStr[] PROGMEM = "";
            return flashStr;
        }
        case AStr_HARD: {
            static const char flashStr[] PROGMEM = " HARD";
            return flashStr;
        }
        case AStr_In: {
            static const char flashStr[] PROGMEM = " in ";
            return flashStr;
        }
        case AStr_public: {
            static const char flashStr[] PROGMEM = "public";
            return flashStr;
        }
        case AStr_txt: {
            static const char flashStr[] PROGMEM = "txt";
            return flashStr;
        }
        case AStr_Undefined: {
            static const char flashStr[] PROGMEM = "Undefined";
            return flashStr;
        }
        case AStr_Unknown: {
            static const char flashStr[] PROGMEM = "Unknown";
            return flashStr;
        }
        case AStr_null: {
            static const char flashStr[] PROGMEM = "null";
            return flashStr;
        }
        case AStr_Default_SystemName: {
            static const char flashStr[] PROGMEM = "Astruino";
            return flashStr;
        }
        case AStr_Default_ConfigFilename: {
            static const char flashStr[] PROGMEM = "Astruino.cfg";
            return flashStr;
        }
        case AStr_Err_AllocationFailure: {
            static const char flashStr[] PROGMEM = "Allocation failure";
            return flashStr;
        }
        case AStr_Err_AlreadyInitialized: {
            static const char flashStr[] PROGMEM = "Already initialized";
            return flashStr;
        }
        case AStr_Err_AssertionFailure: {
            static const char flashStr[] PROGMEM = "Assertion failure: ";
            return flashStr;
        }
        case AStr_Err_DataVersionMismatch: {
            static const char flashStr[] PROGMEM = "Data version mismatch";
            return flashStr;
        }
        case AStr_Err_ExportFailure: {
            static const char flashStr[] PROGMEM = "Export failure";
            return flashStr;
        }
        case AStr_Err_HashingCollision: {
            static const char flashStr[] PROGMEM = "Hashing collision";
            return flashStr;
        }
        case AStr_Err_ImportFailure: {
            static const char flashStr[] PROGMEM = "Import failure";
            return flashStr;
        }
        case AStr_Err_InitializationFailure: {
            static const char flashStr[] PROGMEM = "Initialization failure";
            return flashStr;
        }
        case AStr_Err_InvalidParameter: {
            static const char flashStr[] PROGMEM = "Invalid parameter";
            return flashStr;
        }
        case AStr_Err_NoPositionsAvailable: {
            static const char flashStr[] PROGMEM = "No positions available";
            return flashStr;
        }
        case AStr_Err_NotConfiguredProperly: {
            static const char flashStr[] PROGMEM = "Not configured properly";
            return flashStr;
        }
        case AStr_Err_NotYetInitialized: {
            static const char flashStr[] PROGMEM = "Not yet initialized";
            return flashStr;
        }
        case AStr_Err_OperationFailure: {
            static const char flashStr[] PROGMEM = "Operation failure";
            return flashStr;
        }
        case AStr_Err_RTCDefaultAddressOnly: {
            static const char flashStr[] PROGMEM = "RTClib only supports the default I2C RTC address";
            return flashStr;
        }
        case AStr_Err_UnknownDataDecode: {
            static const char flashStr[] PROGMEM = "Unknown data decode";
            return flashStr;
        }
        case AStr_Err_UnsupportedOperation: {
            static const char flashStr[] PROGMEM = "Unsupported operation";
            return flashStr;
        }
        case AStr_Log_AcquireSequence: {
            static const char flashStr[] PROGMEM = " acquire sequence";
            return flashStr;
        }
        case AStr_Log_CoverClosing: {
            static const char flashStr[] PROGMEM = "Cover closing";
            return flashStr;
        }
        case AStr_Log_CoverOpening: {
            static const char flashStr[] PROGMEM = "Cover opening";
            return flashStr;
        }
        case AStr_Log_DaytimeSequence: {
            static const char flashStr[] PROGMEM = " daytime sequence";
            return flashStr;
        }
        case AStr_Log_DeploySequence: {
            static const char flashStr[] PROGMEM = " deploy sequence";
            return flashStr;
        }
        case AStr_Log_EnvReport: {
            static const char flashStr[] PROGMEM = " environment report:";
            return flashStr;
        }
        case AStr_Log_HasBegan: {
            static const char flashStr[] PROGMEM = " has began";
            return flashStr;
        }
        case AStr_Log_HasDisabled: {
            static const char flashStr[] PROGMEM = " has disabled";
            return flashStr;
        }
        case AStr_Log_HasEnabled: {
            static const char flashStr[] PROGMEM = " has enabled";
            return flashStr;
        }
        case AStr_Log_HasEnded: {
            static const char flashStr[] PROGMEM = " has ended";
            return flashStr;
        }
        case AStr_Log_ObservationStarted: {
            static const char flashStr[] PROGMEM = "Observation started";
            return flashStr;
        }
        case AStr_Log_ObservationStopped: {
            static const char flashStr[] PROGMEM = "Observation stopped";
            return flashStr;
        }
        case AStr_Log_PreDuskWarmup: {
            static const char flashStr[] PROGMEM = " pre-dusk warm-up";
            return flashStr;
        }
        case AStr_Log_RTCBatteryFailure: {
            static const char flashStr[] PROGMEM = "RTC battery failure";
            return flashStr;
        }
        case AStr_Log_StormingSequence: {
            static const char flashStr[] PROGMEM = " storming sequence";
            return flashStr;
        }
        case AStr_Log_StowSequence: {
            static const char flashStr[] PROGMEM = " stow sequence";
            return flashStr;
        }
        case AStr_Log_SystemDataSaved: {
            static const char flashStr[] PROGMEM = "System data saved";
            return flashStr;
        }
        case AStr_Log_SystemUptime: {
            static const char flashStr[] PROGMEM = "System uptime: ";
            return flashStr;
        }
        case AStr_Log_TrackingSequence: {
            static const char flashStr[] PROGMEM = " tracking sequence";
            return flashStr;
        }
        case AStr_Log_Prefix_Info: {
            static const char flashStr[] PROGMEM = "[INFO] ";
            return flashStr;
        }
        case AStr_Log_Prefix_Warning: {
            static const char flashStr[] PROGMEM = "[WARN] ";
            return flashStr;
        }
        case AStr_Log_Prefix_Error: {
            static const char flashStr[] PROGMEM = "[ERROR] ";
            return flashStr;
        }
        case AStr_Log_Field_Heating_Duration: {
            static const char flashStr[] PROGMEM = "  Heater run time: ";
            return flashStr;
        }
        case AStr_Log_Field_Light_Duration: {
            static const char flashStr[] PROGMEM = "  Tracking hours: ";
            return flashStr;
        }
        case AStr_Log_Field_Temp_Measured: {
            static const char flashStr[] PROGMEM = "  Temperature: ";
            return flashStr;
        }
        case AStr_Log_Field_Time_Finish: {
            static const char flashStr[] PROGMEM = "  Finish time: ";
            return flashStr;
        }
        case AStr_Log_Field_Time_Measured: {
            static const char flashStr[] PROGMEM = "  Elapsed time: ";
            return flashStr;
        }
        case AStr_Log_Field_Time_Start: {
            static const char flashStr[] PROGMEM = "  Start time: ";
            return flashStr;
        }
        case AStr_Log_Field_WindSpeed_Measured: {
            static const char flashStr[] PROGMEM = "  Wind speed: ";
            return flashStr;
        }
        case AStr_Key_ActiveLow: {
            static const char flashStr[] PROGMEM = "activeLow";
            return flashStr;
        }
        case AStr_Key_AutosaveEnabled: {
            static const char flashStr[] PROGMEM = "autosaveEnabled";
            return flashStr;
        }
        case AStr_Key_AutosaveFallback: {
            static const char flashStr[] PROGMEM = "autosaveFallback";
            return flashStr;
        }
        case AStr_Key_AutosaveInterval: {
            static const char flashStr[] PROGMEM = "autosaveInterval";
            return flashStr;
        }
        case AStr_Key_BitRes: {
            static const char flashStr[] PROGMEM = "bitRes";
            return flashStr;
        }
        case AStr_Key_CalibrationUnits: {
            static const char flashStr[] PROGMEM = "calibrationUnits";
            return flashStr;
        }
        case AStr_Key_CatalogId: {
            static const char flashStr[] PROGMEM = "catalogId";
            return flashStr;
        }
        case AStr_Key_Channel: {
            static const char flashStr[] PROGMEM = "channel";
            return flashStr;
        }
        case AStr_Key_ContinuousPowerUsage: {
            static const char flashStr[] PROGMEM = "continuousPowerUsage";
            return flashStr;
        }
        case AStr_Key_CtrlInMode: {
            static const char flashStr[] PROGMEM = "ctrlInMode";
            return flashStr;
        }
        case AStr_Key_DataFilePrefix: {
            static const char flashStr[] PROGMEM = "dataFilePrefix";
            return flashStr;
        }
        case AStr_Key_DeclinationArcseconds: {
            static const char flashStr[] PROGMEM = "declinationArcseconds";
            return flashStr;
        }
        case AStr_Key_DetriggerDelay: {
            static const char flashStr[] PROGMEM = "detriggerDelay";
            return flashStr;
        }
        case AStr_Key_DetriggerTol: {
            static const char flashStr[] PROGMEM = "detriggerTol";
            return flashStr;
        }
        case AStr_Key_DisplayTheme: {
            static const char flashStr[] PROGMEM = "displayTheme";
            return flashStr;
        }
        case AStr_Key_DispOutMode: {
            static const char flashStr[] PROGMEM = "dispOutMode";
            return flashStr;
        }
        case AStr_Key_EnableMode: {
            static const char flashStr[] PROGMEM = "enableMode";
            return flashStr;
        }
        case AStr_Key_Id: {
            static const char flashStr[] PROGMEM = "id";
            return flashStr;
        }
        case AStr_Key_InputPin: {
            static const char flashStr[] PROGMEM = "inputPin";
            return flashStr;
        }
        case AStr_Key_JoystickCalib: {
            static const char flashStr[] PROGMEM = "joystickCalib";
            return flashStr;
        }
        case AStr_Key_LimitTrigger: {
            static const char flashStr[] PROGMEM = "limitTrigger";
            return flashStr;
        }
        case AStr_Key_Location: {
            static const char flashStr[] PROGMEM = "location";
            return flashStr;
        }
        case AStr_Key_LogFilePrefix: {
            static const char flashStr[] PROGMEM = "logFilePrefix";
            return flashStr;
        }
        case AStr_Key_Logger: {
            static const char flashStr[] PROGMEM = "logger";
            return flashStr;
        }
        case AStr_Key_LogLevel: {
            static const char flashStr[] PROGMEM = "logLevel";
            return flashStr;
        }
        case AStr_Key_LogToSDCard: {
            static const char flashStr[] PROGMEM = "logToSDCard";
            return flashStr;
        }
        case AStr_Key_LogToWiFiStorage: {
            static const char flashStr[] PROGMEM = "logToWiFiStorage";
            return flashStr;
        }
        case AStr_Key_MACAddress: {
            static const char flashStr[] PROGMEM = "macAddress";
            return flashStr;
        }
        case AStr_Key_MaxActiveAtOnce: {
            static const char flashStr[] PROGMEM = "maxActiveAtOnce";
            return flashStr;
        }
        case AStr_Key_MaxContinuousMs: {
            static const char flashStr[] PROGMEM = "maxContinuousMs";
            return flashStr;
        }
        case AStr_Key_MaximumPosition: {
            static const char flashStr[] PROGMEM = "maximumPosition";
            return flashStr;
        }
        case AStr_Key_MaxPower: {
            static const char flashStr[] PROGMEM = "maxPower";
            return flashStr;
        }
        case AStr_Key_MeasurementRow: {
            static const char flashStr[] PROGMEM = "measurementRow";
            return flashStr;
        }
        case AStr_Key_MeasurementUnits: {
            static const char flashStr[] PROGMEM = "measurementUnits";
            return flashStr;
        }
        case AStr_Key_MeasureMode: {
            static const char flashStr[] PROGMEM = "measureMode";
            return flashStr;
        }
        case AStr_Key_MinimumPosition: {
            static const char flashStr[] PROGMEM = "minimumPosition";
            return flashStr;
        }
        case AStr_Key_Mode: {
            static const char flashStr[] PROGMEM = "mode";
            return flashStr;
        }
        case AStr_Key_MovingTarget: {
            static const char flashStr[] PROGMEM = "movingTarget";
            return flashStr;
        }
        case AStr_Key_Multiplier: {
            static const char flashStr[] PROGMEM = "multiplier";
            return flashStr;
        }
        case AStr_Key_Offset: {
            static const char flashStr[] PROGMEM = "offset";
            return flashStr;
        }
        case AStr_Key_OutputPin: {
            static const char flashStr[] PROGMEM = "outputPin";
            return flashStr;
        }
        case AStr_Key_OutputPin2: {
            static const char flashStr[] PROGMEM = "outputPin2";
            return flashStr;
        }
        case AStr_Key_ParkPrimary: {
            static const char flashStr[] PROGMEM = "parkPrimary";
            return flashStr;
        }
        case AStr_Key_ParkSecondary: {
            static const char flashStr[] PROGMEM = "parkSecondary";
            return flashStr;
        }
        case AStr_Key_Pin: {
            static const char flashStr[] PROGMEM = "pin";
            return flashStr;
        }
        case AStr_Key_PollingInterval: {
            static const char flashStr[] PROGMEM = "pollingInterval";
            return flashStr;
        }
        case AStr_Key_PowerUnits: {
            static const char flashStr[] PROGMEM = "powerUnits";
            return flashStr;
        }
        case AStr_Key_PowerUsageSensor: {
            static const char flashStr[] PROGMEM = "powerUsageSensor";
            return flashStr;
        }
        case AStr_Key_PreDuskHeatingMins: {
            static const char flashStr[] PROGMEM = "preDuskHeatingMins";
            return flashStr;
        }
        case AStr_Key_PrimaryAxisRate: {
            static const char flashStr[] PROGMEM = "primaryAxisRate";
            return flashStr;
        }
        case AStr_Key_PrimaryLimitsEnabled: {
            static const char flashStr[] PROGMEM = "primaryLimitsEnabled";
            return flashStr;
        }
        case AStr_Key_PrimaryMaximum: {
            static const char flashStr[] PROGMEM = "primaryMaximum";
            return flashStr;
        }
        case AStr_Key_PrimaryMinimum: {
            static const char flashStr[] PROGMEM = "primaryMinimum";
            return flashStr;
        }
        case AStr_Key_Publisher: {
            static const char flashStr[] PROGMEM = "publisher";
            return flashStr;
        }
        case AStr_Key_PublishToSDCard: {
            static const char flashStr[] PROGMEM = "pubToSDCard";
            return flashStr;
        }
        case AStr_Key_PublishToWiFiStorage: {
            static const char flashStr[] PROGMEM = "pubToWiFiStorage";
            return flashStr;
        }
        case AStr_Key_PWMChannel: {
            static const char flashStr[] PROGMEM = "pwmChannel";
            return flashStr;
        }
        case AStr_Key_PWMFrequency: {
            static const char flashStr[] PROGMEM = "pwmFrequency";
            return flashStr;
        }
        case AStr_Key_RailName: {
            static const char flashStr[] PROGMEM = "railName";
            return flashStr;
        }
        case AStr_Key_ReportInterval: {
            static const char flashStr[] PROGMEM = "reportInterval";
            return flashStr;
        }
        case AStr_Key_Revision: {
            static const char flashStr[] PROGMEM = "revision";
            return flashStr;
        }
        case AStr_Key_RightAscensionSeconds: {
            static const char flashStr[] PROGMEM = "rightAscensionSeconds";
            return flashStr;
        }
        case AStr_Key_Scheduler: {
            static const char flashStr[] PROGMEM = "scheduler";
            return flashStr;
        }
        case AStr_Key_SecondaryAxisRate: {
            static const char flashStr[] PROGMEM = "secondaryAxisRate";
            return flashStr;
        }
        case AStr_Key_SecondaryLimitsEnabled: {
            static const char flashStr[] PROGMEM = "secondaryLimitsEnabled";
            return flashStr;
        }
        case AStr_Key_SecondaryMaximum: {
            static const char flashStr[] PROGMEM = "secondaryMaximum";
            return flashStr;
        }
        case AStr_Key_SecondaryMinimum: {
            static const char flashStr[] PROGMEM = "secondaryMinimum";
            return flashStr;
        }
        case AStr_Key_SensorName: {
            static const char flashStr[] PROGMEM = "sensorName";
            return flashStr;
        }
        case AStr_Key_StateStableTimeMs: {
            static const char flashStr[] PROGMEM = "stateStableTimeMs";
            return flashStr;
        }
        case AStr_Key_SystemMode: {
            static const char flashStr[] PROGMEM = "systemMode";
            return flashStr;
        }
        case AStr_Key_SystemName: {
            static const char flashStr[] PROGMEM = "systemName";
            return flashStr;
        }
        case AStr_Key_TargetClass: {
            static const char flashStr[] PROGMEM = "targetClass";
            return flashStr;
        }
        case AStr_Key_TargetName: {
            static const char flashStr[] PROGMEM = "targetName";
            return flashStr;
        }
        case AStr_Key_TargetType: {
            static const char flashStr[] PROGMEM = "targetType";
            return flashStr;
        }
        case AStr_Key_Timestamp: {
            static const char flashStr[] PROGMEM = "timestamp";
            return flashStr;
        }
        case AStr_Key_TimeZoneOffset: {
            static const char flashStr[] PROGMEM = "timeZoneOffset";
            return flashStr;
        }
        case AStr_Key_Tolerance: {
            static const char flashStr[] PROGMEM = "tolerance";
            return flashStr;
        }
        case AStr_Key_ToleranceHigh: {
            static const char flashStr[] PROGMEM = "toleranceHigh";
            return flashStr;
        }
        case AStr_Key_ToleranceLow: {
            static const char flashStr[] PROGMEM = "toleranceLow";
            return flashStr;
        }
        case AStr_Key_TriggerBelow: {
            static const char flashStr[] PROGMEM = "triggerBelow";
            return flashStr;
        }
        case AStr_Key_TriggerOutside: {
            static const char flashStr[] PROGMEM = "triggerOutside";
            return flashStr;
        }
        case AStr_Key_Type: {
            static const char flashStr[] PROGMEM = "type";
            return flashStr;
        }
        case AStr_Key_Units: {
            static const char flashStr[] PROGMEM = "units";
            return flashStr;
        }
        case AStr_Key_UpdatesPerSec: {
            static const char flashStr[] PROGMEM = "updatesPerSec";
            return flashStr;
        }
        case AStr_Key_UsingISR: {
            static const char flashStr[] PROGMEM = "usingISR";
            return flashStr;
        }
        case AStr_Key_Value: {
            static const char flashStr[] PROGMEM = "value";
            return flashStr;
        }
        case AStr_Key_Version: {
            static const char flashStr[] PROGMEM = "version";
            return flashStr;
        }
        case AStr_Key_WiFiPassword: {
            static const char flashStr[] PROGMEM = "wifiPassword";
            return flashStr;
        }
        case AStr_Key_WiFiPasswordSeed: {
            static const char flashStr[] PROGMEM = "wifiPasswordSeed";
            return flashStr;
        }
        case AStr_Key_WiFiSSID: {
            static const char flashStr[] PROGMEM = "wifiSSID";
            return flashStr;
        }
        case AStr_Actuator: {
            static const char flashStr[] PROGMEM = "Actuator";
            return flashStr;
        }
        case AStr_Mount: {
            static const char flashStr[] PROGMEM = "Mount";
            return flashStr;
        }
        case AStr_ObservationDevice: {
            static const char flashStr[] PROGMEM = "ObservationDevice";
            return flashStr;
        }
        case AStr_ObservationDeviceDisplay: {
            static const char flashStr[] PROGMEM = "Observation Device";
            return flashStr;
        }
        case AStr_Rail: {
            static const char flashStr[] PROGMEM = "Rail";
            return flashStr;
        }
        case AStr_Sensor: {
            static const char flashStr[] PROGMEM = "Sensor";
            return flashStr;
        }
        case AStr_Target: {
            static const char flashStr[] PROGMEM = "Target";
            return flashStr;
        }
        case AStr_Sun: {
            static const char flashStr[] PROGMEM = "Sun";
            return flashStr;
        }
        case AStr_Moon: {
            static const char flashStr[] PROGMEM = "Moon";
            return flashStr;
        }
        case AStr_Mercury: {
            static const char flashStr[] PROGMEM = "Mercury";
            return flashStr;
        }
        case AStr_Venus: {
            static const char flashStr[] PROGMEM = "Venus";
            return flashStr;
        }
        case AStr_Mars: {
            static const char flashStr[] PROGMEM = "Mars";
            return flashStr;
        }
        case AStr_Jupiter: {
            static const char flashStr[] PROGMEM = "Jupiter";
            return flashStr;
        }
        case AStr_Saturn: {
            static const char flashStr[] PROGMEM = "Saturn";
            return flashStr;
        }
        case AStr_Uranus: {
            static const char flashStr[] PROGMEM = "Uranus";
            return flashStr;
        }
        case AStr_Neptune: {
            static const char flashStr[] PROGMEM = "Neptune";
            return flashStr;
        }
        case AStr_Sirius: {
            static const char flashStr[] PROGMEM = "Sirius";
            return flashStr;
        }
        case AStr_Canopus: {
            static const char flashStr[] PROGMEM = "Canopus";
            return flashStr;
        }
        case AStr_Arcturus: {
            static const char flashStr[] PROGMEM = "Arcturus";
            return flashStr;
        }
        case AStr_Vega: {
            static const char flashStr[] PROGMEM = "Vega";
            return flashStr;
        }
        case AStr_Capella: {
            static const char flashStr[] PROGMEM = "Capella";
            return flashStr;
        }
        case AStr_RigelKentaurus: {
            static const char flashStr[] PROGMEM = "RigelKentaurus";
            return flashStr;
        }
        case AStr_Procyon: {
            static const char flashStr[] PROGMEM = "Procyon";
            return flashStr;
        }
        case AStr_Betelgeuse: {
            static const char flashStr[] PROGMEM = "Betelgeuse";
            return flashStr;
        }
        case AStr_Achernar: {
            static const char flashStr[] PROGMEM = "Achernar";
            return flashStr;
        }
        case AStr_Hadar: {
            static const char flashStr[] PROGMEM = "Hadar";
            return flashStr;
        }
        case AStr_Altair: {
            static const char flashStr[] PROGMEM = "Altair";
            return flashStr;
        }
        case AStr_Acrux: {
            static const char flashStr[] PROGMEM = "Acrux";
            return flashStr;
        }
        case AStr_Aldebaran: {
            static const char flashStr[] PROGMEM = "Aldebaran";
            return flashStr;
        }
        case AStr_Spica: {
            static const char flashStr[] PROGMEM = "Spica";
            return flashStr;
        }
        case AStr_Antares: {
            static const char flashStr[] PROGMEM = "Antares";
            return flashStr;
        }
        case AStr_Pollux: {
            static const char flashStr[] PROGMEM = "Pollux";
            return flashStr;
        }
        case AStr_Fomalhaut: {
            static const char flashStr[] PROGMEM = "Fomalhaut";
            return flashStr;
        }
        case AStr_Deneb: {
            static const char flashStr[] PROGMEM = "Deneb";
            return flashStr;
        }
        case AStr_Regulus: {
            static const char flashStr[] PROGMEM = "Regulus";
            return flashStr;
        }
        case AStr_Polaris: {
            static const char flashStr[] PROGMEM = "Polaris";
            return flashStr;
        }
        case AStr_Castor: {
            static const char flashStr[] PROGMEM = "Castor";
            return flashStr;
        }
        case AStr_Bellatrix: {
            static const char flashStr[] PROGMEM = "Bellatrix";
            return flashStr;
        }
        case AStr_Alnilam: {
            static const char flashStr[] PROGMEM = "Alnilam";
            return flashStr;
        }
        case AStr_Alnitak: {
            static const char flashStr[] PROGMEM = "Alnitak";
            return flashStr;
        }
        case AStr_Mizar: {
            static const char flashStr[] PROGMEM = "Mizar";
            return flashStr;
        }
        case AStr_Dubhe: {
            static const char flashStr[] PROGMEM = "Dubhe";
            return flashStr;
        }
        case AStr_MessierPrefix: {
            static const char flashStr[] PROGMEM = "M";
            return flashStr;
        }
        case AStr_CustomTargetPrefix: {
            static const char flashStr[] PROGMEM = "CustomTarget";
            return flashStr;
        }
        case AStr_Enum_AC110V: {
            static const char flashStr[] PROGMEM = "AC110V";
            return flashStr;
        }
        case AStr_Enum_AC220V: {
            static const char flashStr[] PROGMEM = "AC220V";
            return flashStr;
        }
        case AStr_Enum_Acquire: {
            static const char flashStr[] PROGMEM = "Acquire";
            return flashStr;
        }
        case AStr_Enum_AltAzimuth: {
            static const char flashStr[] PROGMEM = "AltAzimuth";
            return flashStr;
        }
        case AStr_Enum_AnalogInput: {
            static const char flashStr[] PROGMEM = "AnalogInput";
            return flashStr;
        }
        case AStr_Enum_AnalogJoystick: {
            static const char flashStr[] PROGMEM = "AnalogJoystick";
            return flashStr;
        }
        case AStr_Enum_AnalogOutput: {
            static const char flashStr[] PROGMEM = "AnalogOutput";
            return flashStr;
        }
        case AStr_Enum_Angle: {
            static const char flashStr[] PROGMEM = "Angle";
            return flashStr;
        }
        case AStr_Enum_AscOrder: {
            static const char flashStr[] PROGMEM = "AscOrder";
            return flashStr;
        }
        case AStr_Enum_Average: {
            static const char flashStr[] PROGMEM = "Average";
            return flashStr;
        }
        case AStr_Enum_Balancing: {
            static const char flashStr[] PROGMEM = "Balancing";
            return flashStr;
        }
        case AStr_Enum_CameraCooler: {
            static const char flashStr[] PROGMEM = "CameraCooler";
            return flashStr;
        }
        case AStr_Enum_CameraTemperature: {
            static const char flashStr[] PROGMEM = "CameraTemperature";
            return flashStr;
        }
        case AStr_Enum_Count: {
            static const char flashStr[] PROGMEM = "Count";
            return flashStr;
        }
        case AStr_Enum_Cover: {
            static const char flashStr[] PROGMEM = "Cover";
            return flashStr;
        }
        case AStr_Enum_Current: {
            static const char flashStr[] PROGMEM = "Current";
            return flashStr;
        }
        case AStr_Enum_CustomOLED: {
            static const char flashStr[] PROGMEM = "CustomOLED";
            return flashStr;
        }
        case AStr_Enum_DayStorage: {
            static const char flashStr[] PROGMEM = "DayStorage";
            return flashStr;
        }
        case AStr_Enum_DC12V: {
            static const char flashStr[] PROGMEM = "DC12V";
            return flashStr;
        }
        case AStr_Enum_DC24V: {
            static const char flashStr[] PROGMEM = "DC24V";
            return flashStr;
        }
        case AStr_Enum_DC3V3: {
            static const char flashStr[] PROGMEM = "DC3V3";
            return flashStr;
        }
        case AStr_Enum_DC48V: {
            static const char flashStr[] PROGMEM = "DC48V";
            return flashStr;
        }
        case AStr_Enum_DC5V: {
            static const char flashStr[] PROGMEM = "DC5V";
            return flashStr;
        }
        case AStr_Enum_Deploy: {
            static const char flashStr[] PROGMEM = "Deploy";
            return flashStr;
        }
        case AStr_Enum_DescOrder: {
            static const char flashStr[] PROGMEM = "DescOrder";
            return flashStr;
        }
        case AStr_Enum_DewHeater: {
            static const char flashStr[] PROGMEM = "DewHeater";
            return flashStr;
        }
        case AStr_Enum_DigitalInput: {
            static const char flashStr[] PROGMEM = "DigitalInput";
            return flashStr;
        }
        case AStr_Enum_DigitalInputPullDown: {
            static const char flashStr[] PROGMEM = "DigitalInputPullDown";
            return flashStr;
        }
        case AStr_Enum_DigitalInputPullUp: {
            static const char flashStr[] PROGMEM = "DigitalInputPullUp";
            return flashStr;
        }
        case AStr_Enum_DigitalOutput: {
            static const char flashStr[] PROGMEM = "DigitalOutput";
            return flashStr;
        }
        case AStr_Enum_DigitalOutputPushPull: {
            static const char flashStr[] PROGMEM = "DigitalOutputPushPull";
            return flashStr;
        }
        case AStr_Enum_Distance: {
            static const char flashStr[] PROGMEM = "Distance";
            return flashStr;
        }
        case AStr_Enum_Equatorial: {
            static const char flashStr[] PROGMEM = "Equatorial";
            return flashStr;
        }
        case AStr_Enum_Fan: {
            static const char flashStr[] PROGMEM = "Fan";
            return flashStr;
        }
        case AStr_Enum_Focuser: {
            static const char flashStr[] PROGMEM = "Focuser";
            return flashStr;
        }
        case AStr_Enum_Galaxy: {
            static const char flashStr[] PROGMEM = "Galaxy";
            return flashStr;
        }
        case AStr_Enum_GlobularCluster: {
            static const char flashStr[] PROGMEM = "GlobularCluster";
            return flashStr;
        }
        case AStr_Enum_Highest: {
            static const char flashStr[] PROGMEM = "Highest";
            return flashStr;
        }
        case AStr_Enum_Humidity: {
            static const char flashStr[] PROGMEM = "Humidity";
            return flashStr;
        }
        case AStr_Enum_IL3820: {
            static const char flashStr[] PROGMEM = "IL3820";
            return flashStr;
        }
        case AStr_Enum_IL3820V2: {
            static const char flashStr[] PROGMEM = "IL3820V2";
            return flashStr;
        }
        case AStr_Enum_ILI9341: {
            static const char flashStr[] PROGMEM = "ILI9341";
            return flashStr;
        }
        case AStr_Enum_Imperial: {
            static const char flashStr[] PROGMEM = "Imperial";
            return flashStr;
        }
        case AStr_Enum_Init: {
            static const char flashStr[] PROGMEM = "Init";
            return flashStr;
        }
        case AStr_Enum_InOrder: {
            static const char flashStr[] PROGMEM = "InOrder";
            return flashStr;
        }
        case AStr_Enum_LCD16x2: {
            static const char flashStr[] PROGMEM = "LCD16x2";
            return flashStr;
        }
        case AStr_Enum_LCD20x4: {
            static const char flashStr[] PROGMEM = "LCD20x4";
            return flashStr;
        }
        case AStr_Enum_Light: {
            static const char flashStr[] PROGMEM = "Light";
            return flashStr;
        }
        case AStr_Enum_LimitSwitch: {
            static const char flashStr[] PROGMEM = "LimitSwitch";
            return flashStr;
        }
        case AStr_Enum_Lowest: {
            static const char flashStr[] PROGMEM = "Lowest";
            return flashStr;
        }
        case AStr_Enum_Manual: {
            static const char flashStr[] PROGMEM = "Manual";
            return flashStr;
        }
        case AStr_Enum_Matrix2x2: {
            static const char flashStr[] PROGMEM = "Matrix2x2";
            return flashStr;
        }
        case AStr_Enum_Matrix3x4: {
            static const char flashStr[] PROGMEM = "Matrix3x4";
            return flashStr;
        }
        case AStr_Enum_Matrix4x4: {
            static const char flashStr[] PROGMEM = "Matrix4x4";
            return flashStr;
        }
        case AStr_Enum_Metric: {
            static const char flashStr[] PROGMEM = "Metric";
            return flashStr;
        }
        case AStr_Enum_MountAxis: {
            static const char flashStr[] PROGMEM = "MountAxis";
            return flashStr;
        }
        case AStr_Enum_Multiply: {
            static const char flashStr[] PROGMEM = "Multiply";
            return flashStr;
        }
        case AStr_Enum_Nebula: {
            static const char flashStr[] PROGMEM = "Nebula";
            return flashStr;
        }
        case AStr_Enum_NightObserving: {
            static const char flashStr[] PROGMEM = "NightObserving";
            return flashStr;
        }
        case AStr_Enum_OpenCluster: {
            static const char flashStr[] PROGMEM = "OpenCluster";
            return flashStr;
        }
        case AStr_Enum_Other: {
            static const char flashStr[] PROGMEM = "Other";
            return flashStr;
        }
        case AStr_Enum_Percentile: {
            static const char flashStr[] PROGMEM = "Percentile";
            return flashStr;
        }
        case AStr_Enum_PlanetaryNebula: {
            static const char flashStr[] PROGMEM = "PlanetaryNebula";
            return flashStr;
        }
        case AStr_Enum_Position: {
            static const char flashStr[] PROGMEM = "Position";
            return flashStr;
        }
        case AStr_Enum_Power: {
            static const char flashStr[] PROGMEM = "Power";
            return flashStr;
        }
        case AStr_Enum_Rain: {
            static const char flashStr[] PROGMEM = "Rain";
            return flashStr;
        }
        case AStr_Enum_Raw: {
            static const char flashStr[] PROGMEM = "Raw";
            return flashStr;
        }
        case AStr_Enum_RemoteControl: {
            static const char flashStr[] PROGMEM = "RemoteControl";
            return flashStr;
        }
        case AStr_Enum_ResistiveTouch: {
            static const char flashStr[] PROGMEM = "ResistiveTouch";
            return flashStr;
        }
        case AStr_Enum_RevOrder: {
            static const char flashStr[] PROGMEM = "RevOrder";
            return flashStr;
        }
        case AStr_Enum_RotaryEncoder: {
            static const char flashStr[] PROGMEM = "RotaryEncoder";
            return flashStr;
        }
        case AStr_Enum_SafeStowed: {
            static const char flashStr[] PROGMEM = "SafeStowed";
            return flashStr;
        }
        case AStr_Enum_Scientific: {
            static const char flashStr[] PROGMEM = "Scientific";
            return flashStr;
        }
        case AStr_Enum_SH1106: {
            static const char flashStr[] PROGMEM = "SH1106";
            return flashStr;
        }
        case AStr_Enum_SingleAxis: {
            static const char flashStr[] PROGMEM = "SingleAxis";
            return flashStr;
        }
        case AStr_Enum_SolarSystem: {
            static const char flashStr[] PROGMEM = "SolarSystem";
            return flashStr;
        }
        case AStr_Enum_Speed: {
            static const char flashStr[] PROGMEM = "Speed";
            return flashStr;
        }
        case AStr_Enum_SSD1305: {
            static const char flashStr[] PROGMEM = "SSD1305";
            return flashStr;
        }
        case AStr_Enum_SSD1305x32Ada: {
            static const char flashStr[] PROGMEM = "SSD1305x32Ada";
            return flashStr;
        }
        case AStr_Enum_SSD1305x64Ada: {
            static const char flashStr[] PROGMEM = "SSD1305x64Ada";
            return flashStr;
        }
        case AStr_Enum_SSD1306: {
            static const char flashStr[] PROGMEM = "SSD1306";
            return flashStr;
        }
        case AStr_Enum_SSD1607: {
            static const char flashStr[] PROGMEM = "SSD1607";
            return flashStr;
        }
        case AStr_Enum_ST7735: {
            static const char flashStr[] PROGMEM = "ST7735";
            return flashStr;
        }
        case AStr_Enum_ST7789: {
            static const char flashStr[] PROGMEM = "ST7789";
            return flashStr;
        }
        case AStr_Enum_Star: {
            static const char flashStr[] PROGMEM = "Star";
            return flashStr;
        }
        case AStr_Enum_Stow: {
            static const char flashStr[] PROGMEM = "Stow";
            return flashStr;
        }
        case AStr_Enum_Temperature: {
            static const char flashStr[] PROGMEM = "Temperature";
            return flashStr;
        }
        case AStr_Enum_TFTTouch: {
            static const char flashStr[] PROGMEM = "TFTTouch";
            return flashStr;
        }
        case AStr_Enum_TouchScreen: {
            static const char flashStr[] PROGMEM = "TouchScreen";
            return flashStr;
        }
        case AStr_Enum_Track: {
            static const char flashStr[] PROGMEM = "Track";
            return flashStr;
        }
        case AStr_Enum_Tracking: {
            static const char flashStr[] PROGMEM = "Tracking";
            return flashStr;
        }
        case AStr_Enum_UpDownButtons: {
            static const char flashStr[] PROGMEM = "UpDownButtons";
            return flashStr;
        }
        case AStr_Enum_UpDownESP32Touch: {
            static const char flashStr[] PROGMEM = "UpDownESP32Touch";
            return flashStr;
        }
        case AStr_Enum_Voltage: {
            static const char flashStr[] PROGMEM = "Voltage";
            return flashStr;
        }
        case AStr_Enum_Warm: {
            static const char flashStr[] PROGMEM = "Warm";
            return flashStr;
        }
        case AStr_Enum_WindSpeed: {
            static const char flashStr[] PROGMEM = "WindSpeed";
            return flashStr;
        }
        case AStr_Unit_Count: {
            static const char flashStr[] PROGMEM = "Count";
            return flashStr;
        }
        case AStr_Unit_A: {
            static const char flashStr[] PROGMEM = "A";
            return flashStr;
        }
        case AStr_Unit_C: {
            static const char flashStr[] PROGMEM = "C";
            return flashStr;
        }
        case AStr_Unit_deg: {
            static const char flashStr[] PROGMEM = "deg";
            return flashStr;
        }
        case AStr_Unit_F: {
            static const char flashStr[] PROGMEM = "F";
            return flashStr;
        }
        case AStr_Unit_ft: {
            static const char flashStr[] PROGMEM = "ft";
            return flashStr;
        }
        case AStr_Unit_ftPers: {
            static const char flashStr[] PROGMEM = "ft/s";
            return flashStr;
        }
        case AStr_Unit_K: {
            static const char flashStr[] PROGMEM = "K";
            return flashStr;
        }
        case AStr_Unit_m: {
            static const char flashStr[] PROGMEM = "m";
            return flashStr;
        }
        case AStr_Unit_mPers: {
            static const char flashStr[] PROGMEM = "m/s";
            return flashStr;
        }
        case AStr_Unit_N1: {
            static const char flashStr[] PROGMEM = "1";
            return flashStr;
        }
        case AStr_Unit_Percent: {
            static const char flashStr[] PROGMEM = "%";
            return flashStr;
        }
        case AStr_Unit_PercentRH: {
            static const char flashStr[] PROGMEM = "%RH";
            return flashStr;
        }
        case AStr_Unit_rad: {
            static const char flashStr[] PROGMEM = "rad";
            return flashStr;
        }
        case AStr_Unit_V: {
            static const char flashStr[] PROGMEM = "V";
            return flashStr;
        }
        case AStr_Unit_W: {
            static const char flashStr[] PROGMEM = "W";
            return flashStr;
        }
        case AStr_Unit_Undefined: {
            static const char flashStr[] PROGMEM = "Undefined";
            return flashStr;
        }
        case AStr_Count: break;

    }
    return AStr_Blank;
}

#endif
