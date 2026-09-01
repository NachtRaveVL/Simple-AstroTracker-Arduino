/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Strings
*/

#include "Astruino.h"

static char _blank = '\000';
const char *AStr_Blank = &_blank;

static uint16_t _strDataAddress((uint16_t)-1);
void beginStringsFromEEPROM(uint16_t dataAddress)
{
    _strDataAddress = dataAddress;
}

static String _strDataFilePrefix;
void beginStringsFromSDCard(String dataFilePrefix)
{
    _strDataFilePrefix = dataFilePrefix;
}

inline String getStringsFilename()
{
    String filename; filename.reserve(_strDataFilePrefix.length() + 11 + 1);
    filename.concat(_strDataFilePrefix);
    filename.concat('s'); // Cannot use SFP here so have to do it the long way
    filename.concat('t');
    filename.concat('r');
    filename.concat('i');
    filename.concat('n');
    filename.concat('g');
    filename.concat('s');
    filename.concat('.');
    filename.concat('d');
    filename.concat('a');
    filename.concat('t');
    return filename;
}

String stringFromPGM(Astro_String strNum)
{    
    static Astro_String _lookupStrNum = (Astro_String)-1; // Simple LRU cache reduces a lot of lookup access
    static String _lookupCachedRes;
    if (strNum == _lookupStrNum) { return _lookupCachedRes; }
    else { _lookupStrNum = strNum; } // _lookupCachedRes set below

    if (_strDataAddress != (uint16_t)-1) {
        auto eeprom = getController()->getEEPROM();

        if (eeprom) {
            uint16_t lookupOffset = 0;
            eeprom->readBlock(_strDataAddress + (sizeof(uint16_t) * ((int)strNum + 1)), // +1 for initial total size word
                              (uint8_t *)&lookupOffset, sizeof(lookupOffset));

            {   String retVal;
                char buffer[ASTRO_STRING_BUFFER_SIZE] = {0};
                uint16_t bytesRead = eeprom->readBlock(lookupOffset, (uint8_t *)&buffer[0], ASTRO_STRING_BUFFER_SIZE);
                retVal.concat(charsToString(buffer, bytesRead));

                while (strnlen(buffer, ASTRO_STRING_BUFFER_SIZE) == ASTRO_STRING_BUFFER_SIZE) {
                    lookupOffset += ASTRO_STRING_BUFFER_SIZE;
                    bytesRead = eeprom->readBlock(lookupOffset, (uint8_t *)&buffer[0], ASTRO_STRING_BUFFER_SIZE);
                    if (bytesRead) { retVal.concat(charsToString(buffer, bytesRead)); }
                }

                if (retVal.length()) {
                    return (_lookupCachedRes = retVal);
                }
            }
        }
    }

    if (_strDataFilePrefix.length()) {
        #if ASTRO_SYS_LEAVE_FILES_OPEN
            static
        #endif
        auto sd = getController()->getSDCard();

        if (sd) {
            String retVal;
            #if ASTRO_SYS_LEAVE_FILES_OPEN
                static
            #endif
            auto file = sd->open(getStringsFilename().c_str(), FILE_READ);

            if (file) {
                uint16_t lookupOffset = 0;
                file.seek(sizeof(uint16_t) * (int)strNum);
                #if defined(ARDUINO_ARCH_RP2040) || defined(ESP_PLATFORM)
                    file.readBytes((char *)&lookupOffset, sizeof(lookupOffset));
                #else
                    file.readBytes((uint8_t *)&lookupOffset, sizeof(lookupOffset));
                #endif

                {   char buffer[ASTRO_STRING_BUFFER_SIZE];
                    file.seek(lookupOffset);
                    auto bytesRead = file.readBytesUntil('\000', buffer, ASTRO_STRING_BUFFER_SIZE);
                    retVal.concat(charsToString(buffer, bytesRead));

                    while (strnlen(buffer, ASTRO_STRING_BUFFER_SIZE) == ASTRO_STRING_BUFFER_SIZE) {
                        bytesRead = file.readBytesUntil('\000', buffer, ASTRO_STRING_BUFFER_SIZE);
                        if (bytesRead) { retVal.concat(charsToString(buffer, bytesRead)); }
                    }
                }

                #if !ASTRO_SYS_LEAVE_FILES_OPEN
                    file.close();
                #endif
            }

            #if !ASTRO_SYS_LEAVE_FILES_OPEN
                getController()->endSDCard(sd);
            #endif
            if (retVal.length()) {
                return (_lookupCachedRes = retVal);
            }
        }
    }

    #ifndef ASTRO_DISABLE_BUILTIN_DATA
        return (_lookupCachedRes = stringFromPGMAddr(pgmAddrForStr(strNum)));
    #else
        return (_lookupCachedRes = String());
    #endif
}

String stringFromPGMAddr(const char *flashStr) {
    String retVal; retVal.reserve(strlen_P(flashStr) + 1);
    char buffer[ASTRO_STRING_BUFFER_SIZE] = {0};
    strncpy_P(buffer, flashStr, ASTRO_STRING_BUFFER_SIZE);
    retVal.concat(charsToString(buffer, ASTRO_STRING_BUFFER_SIZE));

    while (strnlen(buffer, ASTRO_STRING_BUFFER_SIZE) == ASTRO_STRING_BUFFER_SIZE) {
        flashStr += ASTRO_STRING_BUFFER_SIZE;
        strncpy_P(buffer, flashStr, ASTRO_STRING_BUFFER_SIZE);
        if (buffer[0]) { retVal.concat(charsToString(buffer, ASTRO_STRING_BUFFER_SIZE)); }
    }

    return retVal;
}

#ifndef ASTRO_DISABLE_BUILTIN_DATA

const char *pgmAddrForStr(Astro_String strNum)
{
    switch(strNum) {
        case AStr_ColonSpace: {
            static const char flashStr_ColonSpace[] PROGMEM = {": "};
            return flashStr_ColonSpace;
        } break;
        case AStr_DoubleSpace: {
            static const char flashStr_DoubleSpace[] PROGMEM = {"  "};
            return flashStr_DoubleSpace;
        } break;
        case AStr_csv: {
            static const char flashStr_csv[] PROGMEM = {"csv"};
            return flashStr_csv;
        } break;
        case AStr_dat: {
            static const char flashStr_dat[] PROGMEM = {"dat"};
            return flashStr_dat;
        } break;
        case AStr_Disabled: {
            static const char flashStr_Disabled[] PROGMEM = {"Disabled"};
            return flashStr_Disabled;
        } break;
        case AStr_raw: {
            static const char flashStr_raw[] PROGMEM = {"raw"};
            return flashStr_raw;
        } break;
        case AStr_txt: {
            static const char flashStr_txt[] PROGMEM = {"txt"};
            return flashStr_txt;
        } break;
        case AStr_Undefined: {
            static const char flashStr_Undefined[] PROGMEM = {"Undefined"};
            return flashStr_Undefined;
        } break;
        case AStr_null: {
            static const char flashStr_null[] PROGMEM = {"null"};
            return flashStr_null;
        } break;
        case AStr_Default_SystemName: {
            static const char flashStr_Default_SystemName[] PROGMEM = {"Astruino"};
            return flashStr_Default_SystemName;
        } break;
        case AStr_Default_ConfigFilename: {
            static const char flashStr_Default_ConfigFilename[] PROGMEM = {"Astruino.cfg"};
            return flashStr_Default_ConfigFilename;
        } break;
        case AStr_Err_AllocationFailure: {
            static const char flashStr_Err_AllocationFailure[] PROGMEM = {"Allocation failure"};
            return flashStr_Err_AllocationFailure;
        } break;
        case AStr_Err_AlreadyInitialized: {
            static const char flashStr_Err_AlreadyInitialized[] PROGMEM = {"Already initialized"};
            return flashStr_Err_AlreadyInitialized;
        } break;
        case AStr_Err_AssertionFailure: {
            static const char flashStr_Err_AssertionFailure[] PROGMEM = {"Assertion failure: "};
            return flashStr_Err_AssertionFailure;
        } break;
        case AStr_Err_DataVersionMismatch: {
            static const char flashStr_Err_DataVersionMismatch[] PROGMEM = {"Data version mismatch"};
            return flashStr_Err_DataVersionMismatch;
        } break;
        case AStr_Err_ExportFailure: {
            static const char flashStr_Err_ExportFailure[] PROGMEM = {"Export failure"};
            return flashStr_Err_ExportFailure;
        } break;
        case AStr_Err_HashingCollision: {
            static const char flashStr_Err_HashingCollision[] PROGMEM = {"Hashing collision"};
            return flashStr_Err_HashingCollision;
        } break;
        case AStr_Err_ImportFailure: {
            static const char flashStr_Err_ImportFailure[] PROGMEM = {"Import failure"};
            return flashStr_Err_ImportFailure;
        } break;
        case AStr_Err_InitializationFailure: {
            static const char flashStr_Err_InitializationFailure[] PROGMEM = {"Initialization failure"};
            return flashStr_Err_InitializationFailure;
        } break;
        case AStr_Err_InvalidParameter: {
            static const char flashStr_Err_InvalidParameter[] PROGMEM = {"Invalid parameter"};
            return flashStr_Err_InvalidParameter;
        } break;
        case AStr_Err_InvalidPinOrType: {
            static const char flashStr_Err_InvalidPinOrType[] PROGMEM = {"Invalid pin or type"};
            return flashStr_Err_InvalidPinOrType;
        } break;
        case AStr_Err_MeasurementFailure: {
            static const char flashStr_Err_MeasurementFailure[] PROGMEM = {"Measurement failure"};
            return flashStr_Err_MeasurementFailure;
        } break;
        case AStr_Err_MissingLinkage: {
            static const char flashStr_Err_MissingLinkage[] PROGMEM = {"Missing or no linkage"};
            return flashStr_Err_MissingLinkage;
        } break;
        case AStr_Err_NoPositionsAvailable: {
            static const char flashStr_Err_NoPositionsAvailable[] PROGMEM = {"No positions available"};
            return flashStr_Err_NoPositionsAvailable;
        } break;
        case AStr_Err_NotConfiguredProperly: {
            static const char flashStr_Err_NotConfiguredProperly[] PROGMEM = {"Not configured properly"};
            return flashStr_Err_NotConfiguredProperly;
        } break;
        case AStr_Err_NotYetInitialized: {
            static const char flashStr_Err_NotYetInitialized[] PROGMEM = {"Not yet initialized"};
            return flashStr_Err_NotYetInitialized;
        } break;
        case AStr_Err_OperationFailure: {
            static const char flashStr_Err_OperationFailure[] PROGMEM = {"Operation failure"};
            return flashStr_Err_OperationFailure;
        } break;
        case AStr_Err_RTCDefaultAddressOnly: {
            static const char flashStr_Err_RTCDefaultAddressOnly[] PROGMEM = {"RTClib only supports the default I2C RTC address"};
            return flashStr_Err_RTCDefaultAddressOnly;
        } break;
        case AStr_Err_UnknownDataDecode: {
            static const char flashStr_Err_UnknownDataDecode[] PROGMEM = {"Unknown data decode"};
            return flashStr_Err_UnknownDataDecode;
        } break;
        case AStr_Err_UnsupportedOperation: {
            static const char flashStr_Err_UnsupportedOperation[] PROGMEM = {"Unsupported operation"};
            return flashStr_Err_UnsupportedOperation;
        } break;
        case AStr_Log_AcquireSequence: {
            static const char flashStr_Log_AcquireSequence[] PROGMEM = {" acquire sequence"};
            return flashStr_Log_AcquireSequence;
        } break;
        case AStr_Log_CoverClosing: {
            static const char flashStr_Log_CoverClosing[] PROGMEM = {"Cover closing"};
            return flashStr_Log_CoverClosing;
        } break;
        case AStr_Log_CoverOpening: {
            static const char flashStr_Log_CoverOpening[] PROGMEM = {"Cover opening"};
            return flashStr_Log_CoverOpening;
        } break;
        case AStr_Log_DaytimeSequence: {
            static const char flashStr_Log_DaytimeSequence[] PROGMEM = {" daytime sequence"};
            return flashStr_Log_DaytimeSequence;
        } break;
        case AStr_Log_DeploySequence: {
            static const char flashStr_Log_DeploySequence[] PROGMEM = {" deploy sequence"};
            return flashStr_Log_DeploySequence;
        } break;
        case AStr_Log_EnvReport: {
            static const char flashStr_Log_EnvReport[] PROGMEM = {" environment report:"};
            return flashStr_Log_EnvReport;
        } break;
        case AStr_Log_HasBegan: {
            static const char flashStr_Log_HasBegan[] PROGMEM = {" has began"};
            return flashStr_Log_HasBegan;
        } break;
        case AStr_Log_HasDisabled: {
            static const char flashStr_Log_HasDisabled[] PROGMEM = {" has disabled"};
            return flashStr_Log_HasDisabled;
        } break;
        case AStr_Log_HasEnabled: {
            static const char flashStr_Log_HasEnabled[] PROGMEM = {" has enabled"};
            return flashStr_Log_HasEnabled;
        } break;
        case AStr_Log_HasEnded: {
            static const char flashStr_Log_HasEnded[] PROGMEM = {" has ended"};
            return flashStr_Log_HasEnded;
        } break;
        case AStr_Log_ObservationStarted: {
            static const char flashStr_Log_ObservationStarted[] PROGMEM = {"Observation started"};
            return flashStr_Log_ObservationStarted;
        } break;
        case AStr_Log_ObservationStopped: {
            static const char flashStr_Log_ObservationStopped[] PROGMEM = {"Observation stopped"};
            return flashStr_Log_ObservationStopped;
        } break;
        case AStr_Log_PreDuskWarmup: {
            static const char flashStr_Log_PreDuskWarmup[] PROGMEM = {" pre-dusk warm-up"};
            return flashStr_Log_PreDuskWarmup;
        } break;
        case AStr_Log_RTCBatteryFailure: {
            static const char flashStr_Log_RTCBatteryFailure[] PROGMEM = {"RTC battery failure"};
            return flashStr_Log_RTCBatteryFailure;
        } break;
        case AStr_Log_StormingSequence: {
            static const char flashStr_Log_StormingSequence[] PROGMEM = {" storming sequence"};
            return flashStr_Log_StormingSequence;
        } break;
        case AStr_Log_StowSequence: {
            static const char flashStr_Log_StowSequence[] PROGMEM = {" stow sequence"};
            return flashStr_Log_StowSequence;
        } break;
        case AStr_Log_SystemDataSaved: {
            static const char flashStr_Log_SystemDataSaved[] PROGMEM = {"System data saved"};
            return flashStr_Log_SystemDataSaved;
        } break;
        case AStr_Log_SystemUptime: {
            static const char flashStr_Log_SystemUptime[] PROGMEM = {"System uptime: "};
            return flashStr_Log_SystemUptime;
        } break;
        case AStr_Log_TrackingSequence: {
            static const char flashStr_Log_TrackingSequence[] PROGMEM = {" tracking sequence"};
            return flashStr_Log_TrackingSequence;
        } break;
        case AStr_Log_Prefix_Info: {
            static const char flashStr_Log_Prefix_Info[] PROGMEM = {"[INFO] "};
            return flashStr_Log_Prefix_Info;
        } break;
        case AStr_Log_Prefix_Warning: {
            static const char flashStr_Log_Prefix_Warning[] PROGMEM = {"[WARN] "};
            return flashStr_Log_Prefix_Warning;
        } break;
        case AStr_Log_Prefix_Error: {
            static const char flashStr_Log_Prefix_Error[] PROGMEM = {"[ERROR] "};
            return flashStr_Log_Prefix_Error;
        } break;
        case AStr_Log_Field_Heating_Duration: {
            static const char flashStr_Log_Field_Heating_Duration[] PROGMEM = {"  Heater run time: "};
            return flashStr_Log_Field_Heating_Duration;
        } break;
        case AStr_Log_Field_Light_Duration: {
            static const char flashStr_Log_Field_Light_Duration[] PROGMEM = {"  Tracking hours: "};
            return flashStr_Log_Field_Light_Duration;
        } break;
        case AStr_Log_Field_Temp_Measured: {
            static const char flashStr_Log_Field_Temp_Measured[] PROGMEM = {"  Temperature: "};
            return flashStr_Log_Field_Temp_Measured;
        } break;
        case AStr_Log_Field_Time_Finish: {
            static const char flashStr_Log_Field_Time_Finish[] PROGMEM = {"  Finish time: "};
            return flashStr_Log_Field_Time_Finish;
        } break;
        case AStr_Log_Field_Time_Measured: {
            static const char flashStr_Log_Field_Time_Measured[] PROGMEM = {"  Elapsed time: "};
            return flashStr_Log_Field_Time_Measured;
        } break;
        case AStr_Log_Field_Time_Start: {
            static const char flashStr_Log_Field_Time_Start[] PROGMEM = {"  Start time: "};
            return flashStr_Log_Field_Time_Start;
        } break;
        case AStr_Log_Field_WindSpeed_Measured: {
            static const char flashStr_Log_Field_WindSpeed_Measured[] PROGMEM = {"  Wind speed: "};
            return flashStr_Log_Field_WindSpeed_Measured;
        } break;
        case AStr_Key_ActiveLow: {
            static const char flashStr_Key_ActiveLow[] PROGMEM = {"activeLow"};
            return flashStr_Key_ActiveLow;
        } break;
        case AStr_Key_AutosaveEnabled: {
            static const char flashStr_Key_AutosaveEnabled[] PROGMEM = {"autosaveEnabled"};
            return flashStr_Key_AutosaveEnabled;
        } break;
        case AStr_Key_AutosaveFallback: {
            static const char flashStr_Key_AutosaveFallback[] PROGMEM = {"autosaveFallback"};
            return flashStr_Key_AutosaveFallback;
        } break;
        case AStr_Key_AutosaveInterval: {
            static const char flashStr_Key_AutosaveInterval[] PROGMEM = {"autosaveInterval"};
            return flashStr_Key_AutosaveInterval;
        } break;
        case AStr_Key_BitRes: {
            static const char flashStr_Key_BitRes[] PROGMEM = {"bitRes"};
            return flashStr_Key_BitRes;
        } break;
        case AStr_Key_CalibrationUnits: {
            static const char flashStr_Key_CalibrationUnits[] PROGMEM = {"calibrationUnits"};
            return flashStr_Key_CalibrationUnits;
        } break;
        case AStr_Key_Camera: {
            static const char flashStr_Key_Camera[] PROGMEM = {"camera"};
            return flashStr_Key_Camera;
        } break;
        case AStr_Key_CatalogId: {
            static const char flashStr_Key_CatalogId[] PROGMEM = {"catalogId"};
            return flashStr_Key_CatalogId;
        } break;
        case AStr_Key_Channel: {
            static const char flashStr_Key_Channel[] PROGMEM = {"channel"};
            return flashStr_Key_Channel;
        } break;
        case AStr_Key_ContinuousPowerUsage: {
            static const char flashStr_Key_ContinuousPowerUsage[] PROGMEM = {"continuousPowerUsage"};
            return flashStr_Key_ContinuousPowerUsage;
        } break;
        case AStr_Key_CtrlInMode: {
            static const char flashStr_Key_CtrlInMode[] PROGMEM = {"ctrlInMode"};
            return flashStr_Key_CtrlInMode;
        } break;
        case AStr_Key_DataFilePrefix: {
            static const char flashStr_Key_DataFilePrefix[] PROGMEM = {"dataFilePrefix"};
            return flashStr_Key_DataFilePrefix;
        } break;
        case AStr_Key_DeclinationArcseconds: {
            static const char flashStr_Key_DeclinationArcseconds[] PROGMEM = {"declinationArcseconds"};
            return flashStr_Key_DeclinationArcseconds;
        } break;
        case AStr_Key_DetriggerDelay: {
            static const char flashStr_Key_DetriggerDelay[] PROGMEM = {"detriggerDelay"};
            return flashStr_Key_DetriggerDelay;
        } break;
        case AStr_Key_DetriggerTol: {
            static const char flashStr_Key_DetriggerTol[] PROGMEM = {"detriggerTol"};
            return flashStr_Key_DetriggerTol;
        } break;
        case AStr_Key_DisplayTheme: {
            static const char flashStr_Key_DisplayTheme[] PROGMEM = {"displayTheme"};
            return flashStr_Key_DisplayTheme;
        } break;
        case AStr_Key_DispOutMode: {
            static const char flashStr_Key_DispOutMode[] PROGMEM = {"dispOutMode"};
            return flashStr_Key_DispOutMode;
        } break;
        case AStr_Key_EnableMode: {
            static const char flashStr_Key_EnableMode[] PROGMEM = {"enableMode"};
            return flashStr_Key_EnableMode;
        } break;
        case AStr_Key_ExposureMillis: {
            static const char flashStr_Key_ExposureMillis[] PROGMEM = {"exposureMillis"};
            return flashStr_Key_ExposureMillis;
        } break;
        case AStr_Key_Id: {
            static const char flashStr_Key_Id[] PROGMEM = {"id"};
            return flashStr_Key_Id;
        } break;
        case AStr_Key_InputPin: {
            static const char flashStr_Key_InputPin[] PROGMEM = {"inputPin"};
            return flashStr_Key_InputPin;
        } break;
        case AStr_Key_IntervalMillis: {
            static const char flashStr_Key_IntervalMillis[] PROGMEM = {"intervalMillis"};
            return flashStr_Key_IntervalMillis;
        } break;
        case AStr_Key_JoystickCalib: {
            static const char flashStr_Key_JoystickCalib[] PROGMEM = {"joystickCalib"};
            return flashStr_Key_JoystickCalib;
        } break;
        case AStr_Key_LimitTrigger: {
            static const char flashStr_Key_LimitTrigger[] PROGMEM = {"limitTrigger"};
            return flashStr_Key_LimitTrigger;
        } break;
        case AStr_Key_Location: {
            static const char flashStr_Key_Location[] PROGMEM = {"location"};
            return flashStr_Key_Location;
        } break;
        case AStr_Key_LogFilePrefix: {
            static const char flashStr_Key_LogFilePrefix[] PROGMEM = {"logFilePrefix"};
            return flashStr_Key_LogFilePrefix;
        } break;
        case AStr_Key_Logger: {
            static const char flashStr_Key_Logger[] PROGMEM = {"logger"};
            return flashStr_Key_Logger;
        } break;
        case AStr_Key_LogLevel: {
            static const char flashStr_Key_LogLevel[] PROGMEM = {"logLevel"};
            return flashStr_Key_LogLevel;
        } break;
        case AStr_Key_LogToSDCard: {
            static const char flashStr_Key_LogToSDCard[] PROGMEM = {"logToSDCard"};
            return flashStr_Key_LogToSDCard;
        } break;
        case AStr_Key_LogToWiFiStorage: {
            static const char flashStr_Key_LogToWiFiStorage[] PROGMEM = {"logToWiFiStorage"};
            return flashStr_Key_LogToWiFiStorage;
        } break;
        case AStr_Key_MACAddress: {
            static const char flashStr_Key_MACAddress[] PROGMEM = {"macAddress"};
            return flashStr_Key_MACAddress;
        } break;
        case AStr_Key_MaxActiveAtOnce: {
            static const char flashStr_Key_MaxActiveAtOnce[] PROGMEM = {"maxActiveAtOnce"};
            return flashStr_Key_MaxActiveAtOnce;
        } break;
        case AStr_Key_MaxContinuousMs: {
            static const char flashStr_Key_MaxContinuousMs[] PROGMEM = {"maxContinuousMs"};
            return flashStr_Key_MaxContinuousMs;
        } break;
        case AStr_Key_MaximumPosition: {
            static const char flashStr_Key_MaximumPosition[] PROGMEM = {"maximumPosition"};
            return flashStr_Key_MaximumPosition;
        } break;
        case AStr_Key_MaxPower: {
            static const char flashStr_Key_MaxPower[] PROGMEM = {"maxPower"};
            return flashStr_Key_MaxPower;
        } break;
        case AStr_Key_MeasurementRow: {
            static const char flashStr_Key_MeasurementRow[] PROGMEM = {"measurementRow"};
            return flashStr_Key_MeasurementRow;
        } break;
        case AStr_Key_MeasurementUnits: {
            static const char flashStr_Key_MeasurementUnits[] PROGMEM = {"measurementUnits"};
            return flashStr_Key_MeasurementUnits;
        } break;
        case AStr_Key_MeasureMode: {
            static const char flashStr_Key_MeasureMode[] PROGMEM = {"measureMode"};
            return flashStr_Key_MeasureMode;
        } break;
        case AStr_Key_MinimumPosition: {
            static const char flashStr_Key_MinimumPosition[] PROGMEM = {"minimumPosition"};
            return flashStr_Key_MinimumPosition;
        } break;
        case AStr_Key_Mode: {
            static const char flashStr_Key_Mode[] PROGMEM = {"mode"};
            return flashStr_Key_Mode;
        } break;
        case AStr_Key_MountAxisIndex: {
            static const char flashStr_Key_MountAxisIndex[] PROGMEM = {"mountAxisIndex"};
            return flashStr_Key_MountAxisIndex;
        } break;
        case AStr_Key_MountName: {
            static const char flashStr_Key_MountName[] PROGMEM = {"mountName"};
            return flashStr_Key_MountName;
        } break;
        case AStr_Key_MovingTarget: {
            static const char flashStr_Key_MovingTarget[] PROGMEM = {"movingTarget"};
            return flashStr_Key_MovingTarget;
        } break;
        case AStr_Key_Multiplier: {
            static const char flashStr_Key_Multiplier[] PROGMEM = {"multiplier"};
            return flashStr_Key_Multiplier;
        } break;
        case AStr_Key_Offset: {
            static const char flashStr_Key_Offset[] PROGMEM = {"offset"};
            return flashStr_Key_Offset;
        } break;
        case AStr_Key_OutputPin: {
            static const char flashStr_Key_OutputPin[] PROGMEM = {"outputPin"};
            return flashStr_Key_OutputPin;
        } break;
        case AStr_Key_OutputPin2: {
            static const char flashStr_Key_OutputPin2[] PROGMEM = {"outputPin2"};
            return flashStr_Key_OutputPin2;
        } break;
        case AStr_Key_ParkPrimary: {
            static const char flashStr_Key_ParkPrimary[] PROGMEM = {"parkPrimary"};
            return flashStr_Key_ParkPrimary;
        } break;
        case AStr_Key_ParkSecondary: {
            static const char flashStr_Key_ParkSecondary[] PROGMEM = {"parkSecondary"};
            return flashStr_Key_ParkSecondary;
        } break;
        case AStr_Key_Pin: {
            static const char flashStr_Key_Pin[] PROGMEM = {"pin"};
            return flashStr_Key_Pin;
        } break;
        case AStr_Key_PollingInterval: {
            static const char flashStr_Key_PollingInterval[] PROGMEM = {"pollingInterval"};
            return flashStr_Key_PollingInterval;
        } break;
        case AStr_Key_PowerUnits: {
            static const char flashStr_Key_PowerUnits[] PROGMEM = {"powerUnits"};
            return flashStr_Key_PowerUnits;
        } break;
        case AStr_Key_PowerUsageSensor: {
            static const char flashStr_Key_PowerUsageSensor[] PROGMEM = {"powerUsageSensor"};
            return flashStr_Key_PowerUsageSensor;
        } break;
        case AStr_Key_PreDuskHeatingMins: {
            static const char flashStr_Key_PreDuskHeatingMins[] PROGMEM = {"preDuskHeatingMins"};
            return flashStr_Key_PreDuskHeatingMins;
        } break;
        case AStr_Key_PrimaryAxisRate: {
            static const char flashStr_Key_PrimaryAxisRate[] PROGMEM = {"primaryAxisRate"};
            return flashStr_Key_PrimaryAxisRate;
        } break;
        case AStr_Key_PrimaryLimitsEnabled: {
            static const char flashStr_Key_PrimaryLimitsEnabled[] PROGMEM = {"primaryLimitsEnabled"};
            return flashStr_Key_PrimaryLimitsEnabled;
        } break;
        case AStr_Key_PrimaryMaximum: {
            static const char flashStr_Key_PrimaryMaximum[] PROGMEM = {"primaryMaximum"};
            return flashStr_Key_PrimaryMaximum;
        } break;
        case AStr_Key_PrimaryMinimum: {
            static const char flashStr_Key_PrimaryMinimum[] PROGMEM = {"primaryMinimum"};
            return flashStr_Key_PrimaryMinimum;
        } break;
        case AStr_Key_Publisher: {
            static const char flashStr_Key_Publisher[] PROGMEM = {"publisher"};
            return flashStr_Key_Publisher;
        } break;
        case AStr_Key_PublishToSDCard: {
            static const char flashStr_Key_PublishToSDCard[] PROGMEM = {"pubToSDCard"};
            return flashStr_Key_PublishToSDCard;
        } break;
        case AStr_Key_PublishToWiFiStorage: {
            static const char flashStr_Key_PublishToWiFiStorage[] PROGMEM = {"pubToWiFiStorage"};
            return flashStr_Key_PublishToWiFiStorage;
        } break;
        case AStr_Key_PWMChannel: {
            static const char flashStr_Key_PWMChannel[] PROGMEM = {"pwmChannel"};
            return flashStr_Key_PWMChannel;
        } break;
        case AStr_Key_PWMFrequency: {
            static const char flashStr_Key_PWMFrequency[] PROGMEM = {"pwmFrequency"};
            return flashStr_Key_PWMFrequency;
        } break;
        case AStr_Key_RailName: {
            static const char flashStr_Key_RailName[] PROGMEM = {"railName"};
            return flashStr_Key_RailName;
        } break;
        case AStr_Key_ReportInterval: {
            static const char flashStr_Key_ReportInterval[] PROGMEM = {"reportInterval"};
            return flashStr_Key_ReportInterval;
        } break;
        case AStr_Key_Revision: {
            static const char flashStr_Key_Revision[] PROGMEM = {"revision"};
            return flashStr_Key_Revision;
        } break;
        case AStr_Key_RightAscensionSeconds: {
            static const char flashStr_Key_RightAscensionSeconds[] PROGMEM = {"rightAscensionSeconds"};
            return flashStr_Key_RightAscensionSeconds;
        } break;
        case AStr_Key_Scheduler: {
            static const char flashStr_Key_Scheduler[] PROGMEM = {"scheduler"};
            return flashStr_Key_Scheduler;
        } break;
        case AStr_Key_SecondaryAxisRate: {
            static const char flashStr_Key_SecondaryAxisRate[] PROGMEM = {"secondaryAxisRate"};
            return flashStr_Key_SecondaryAxisRate;
        } break;
        case AStr_Key_SecondaryLimitsEnabled: {
            static const char flashStr_Key_SecondaryLimitsEnabled[] PROGMEM = {"secondaryLimitsEnabled"};
            return flashStr_Key_SecondaryLimitsEnabled;
        } break;
        case AStr_Key_SecondaryMaximum: {
            static const char flashStr_Key_SecondaryMaximum[] PROGMEM = {"secondaryMaximum"};
            return flashStr_Key_SecondaryMaximum;
        } break;
        case AStr_Key_SecondaryMinimum: {
            static const char flashStr_Key_SecondaryMinimum[] PROGMEM = {"secondaryMinimum"};
            return flashStr_Key_SecondaryMinimum;
        } break;
        case AStr_Key_SensorName: {
            static const char flashStr_Key_SensorName[] PROGMEM = {"sensorName"};
            return flashStr_Key_SensorName;
        } break;
        case AStr_Key_ShutterName: {
            static const char flashStr_Key_ShutterName[] PROGMEM = {"shutterName"};
            return flashStr_Key_ShutterName;
        } break;
        case AStr_Key_ShutterPulseMillis: {
            static const char flashStr_Key_ShutterPulseMillis[] PROGMEM = {"shutterPulseMillis"};
            return flashStr_Key_ShutterPulseMillis;
        } break;
        case AStr_Key_StateStableTimeMs: {
            static const char flashStr_Key_StateStableTimeMs[] PROGMEM = {"stateStableTimeMs"};
            return flashStr_Key_StateStableTimeMs;
        } break;
        case AStr_Key_SystemMode: {
            static const char flashStr_Key_SystemMode[] PROGMEM = {"systemMode"};
            return flashStr_Key_SystemMode;
        } break;
        case AStr_Key_SystemName: {
            static const char flashStr_Key_SystemName[] PROGMEM = {"systemName"};
            return flashStr_Key_SystemName;
        } break;
        case AStr_Key_TargetClass: {
            static const char flashStr_Key_TargetClass[] PROGMEM = {"targetClass"};
            return flashStr_Key_TargetClass;
        } break;
        case AStr_Key_TargetName: {
            static const char flashStr_Key_TargetName[] PROGMEM = {"targetName"};
            return flashStr_Key_TargetName;
        } break;
        case AStr_Key_TargetType: {
            static const char flashStr_Key_TargetType[] PROGMEM = {"targetType"};
            return flashStr_Key_TargetType;
        } break;
        case AStr_Key_Timestamp: {
            static const char flashStr_Key_Timestamp[] PROGMEM = {"timestamp"};
            return flashStr_Key_Timestamp;
        } break;
        case AStr_Key_TimeZoneOffset: {
            static const char flashStr_Key_TimeZoneOffset[] PROGMEM = {"timeZoneOffset"};
            return flashStr_Key_TimeZoneOffset;
        } break;
        case AStr_Key_Tolerance: {
            static const char flashStr_Key_Tolerance[] PROGMEM = {"tolerance"};
            return flashStr_Key_Tolerance;
        } break;
        case AStr_Key_ToleranceHigh: {
            static const char flashStr_Key_ToleranceHigh[] PROGMEM = {"toleranceHigh"};
            return flashStr_Key_ToleranceHigh;
        } break;
        case AStr_Key_ToleranceLow: {
            static const char flashStr_Key_ToleranceLow[] PROGMEM = {"toleranceLow"};
            return flashStr_Key_ToleranceLow;
        } break;
        case AStr_Key_TriggerBelow: {
            static const char flashStr_Key_TriggerBelow[] PROGMEM = {"triggerBelow"};
            return flashStr_Key_TriggerBelow;
        } break;
        case AStr_Key_TriggerOutside: {
            static const char flashStr_Key_TriggerOutside[] PROGMEM = {"triggerOutside"};
            return flashStr_Key_TriggerOutside;
        } break;
        case AStr_Key_Type: {
            static const char flashStr_Key_Type[] PROGMEM = {"type"};
            return flashStr_Key_Type;
        } break;
        case AStr_Key_Units: {
            static const char flashStr_Key_Units[] PROGMEM = {"units"};
            return flashStr_Key_Units;
        } break;
        case AStr_Key_UpdatesPerSec: {
            static const char flashStr_Key_UpdatesPerSec[] PROGMEM = {"updatesPerSec"};
            return flashStr_Key_UpdatesPerSec;
        } break;
        case AStr_Key_UsingISR: {
            static const char flashStr_Key_UsingISR[] PROGMEM = {"usingISR"};
            return flashStr_Key_UsingISR;
        } break;
        case AStr_Key_Value: {
            static const char flashStr_Key_Value[] PROGMEM = {"value"};
            return flashStr_Key_Value;
        } break;
        case AStr_Key_Version: {
            static const char flashStr_Key_Version[] PROGMEM = {"version"};
            return flashStr_Key_Version;
        } break;
        case AStr_Key_WiFiPassword: {
            static const char flashStr_Key_WiFiPassword[] PROGMEM = {"wifiPassword"};
            return flashStr_Key_WiFiPassword;
        } break;
        case AStr_Key_WiFiPasswordSeed: {
            static const char flashStr_Key_WiFiPasswordSeed[] PROGMEM = {"wifiPasswordSeed"};
            return flashStr_Key_WiFiPasswordSeed;
        } break;
        case AStr_Key_WiFiSSID: {
            static const char flashStr_Key_WiFiSSID[] PROGMEM = {"wifiSSID"};
            return flashStr_Key_WiFiSSID;
        } break;
        case AStr_Actuator: {
            static const char flashStr_Actuator[] PROGMEM = {"Actuator"};
            return flashStr_Actuator;
        } break;
        case AStr_Mount: {
            static const char flashStr_Mount[] PROGMEM = {"Mount"};
            return flashStr_Mount;
        } break;
        case AStr_Rail: {
            static const char flashStr_Rail[] PROGMEM = {"Rail"};
            return flashStr_Rail;
        } break;
        case AStr_Sensor: {
            static const char flashStr_Sensor[] PROGMEM = {"Sensor"};
            return flashStr_Sensor;
        } break;
        case AStr_Target: {
            static const char flashStr_Target[] PROGMEM = {"Target"};
            return flashStr_Target;
        } break;
        case AStr_Enum_AC110V: {
            static const char flashStr_Enum_AC110V[] PROGMEM = {"AC110V"};
            return flashStr_Enum_AC110V;
        } break;
        case AStr_Enum_AC220V: {
            static const char flashStr_Enum_AC220V[] PROGMEM = {"AC220V"};
            return flashStr_Enum_AC220V;
        } break;
        case AStr_Enum_Achernar: {
            static const char flashStr_Enum_Achernar[] PROGMEM = {"Achernar"};
            return flashStr_Enum_Achernar;
        } break;
        case AStr_Enum_Acquire: {
            static const char flashStr_Enum_Acquire[] PROGMEM = {"Acquire"};
            return flashStr_Enum_Acquire;
        } break;
        case AStr_Enum_Acrux: {
            static const char flashStr_Enum_Acrux[] PROGMEM = {"Acrux"};
            return flashStr_Enum_Acrux;
        } break;
        case AStr_Enum_Aldebaran: {
            static const char flashStr_Enum_Aldebaran[] PROGMEM = {"Aldebaran"};
            return flashStr_Enum_Aldebaran;
        } break;
        case AStr_Enum_Alnilam: {
            static const char flashStr_Enum_Alnilam[] PROGMEM = {"Alnilam"};
            return flashStr_Enum_Alnilam;
        } break;
        case AStr_Enum_Alnitak: {
            static const char flashStr_Enum_Alnitak[] PROGMEM = {"Alnitak"};
            return flashStr_Enum_Alnitak;
        } break;
        case AStr_Enum_Altair: {
            static const char flashStr_Enum_Altair[] PROGMEM = {"Altair"};
            return flashStr_Enum_Altair;
        } break;
        case AStr_Enum_AltAzimuth: {
            static const char flashStr_Enum_AltAzimuth[] PROGMEM = {"AltAzimuth"};
            return flashStr_Enum_AltAzimuth;
        } break;
        case AStr_Enum_AnalogInput: {
            static const char flashStr_Enum_AnalogInput[] PROGMEM = {"AnalogInput"};
            return flashStr_Enum_AnalogInput;
        } break;
        case AStr_Enum_AnalogJoystick: {
            static const char flashStr_Enum_AnalogJoystick[] PROGMEM = {"AnalogJoystick"};
            return flashStr_Enum_AnalogJoystick;
        } break;
        case AStr_Enum_AnalogOutput: {
            static const char flashStr_Enum_AnalogOutput[] PROGMEM = {"AnalogOutput"};
            return flashStr_Enum_AnalogOutput;
        } break;
        case AStr_Enum_Angle: {
            static const char flashStr_Enum_Angle[] PROGMEM = {"Angle"};
            return flashStr_Enum_Angle;
        } break;
        case AStr_Enum_Antares: {
            static const char flashStr_Enum_Antares[] PROGMEM = {"Antares"};
            return flashStr_Enum_Antares;
        } break;
        case AStr_Enum_Arcturus: {
            static const char flashStr_Enum_Arcturus[] PROGMEM = {"Arcturus"};
            return flashStr_Enum_Arcturus;
        } break;
        case AStr_Enum_AscOrder: {
            static const char flashStr_Enum_AscOrder[] PROGMEM = {"AscOrder"};
            return flashStr_Enum_AscOrder;
        } break;
        case AStr_Enum_Average: {
            static const char flashStr_Enum_Average[] PROGMEM = {"Average"};
            return flashStr_Enum_Average;
        } break;
        case AStr_Enum_Balancing: {
            static const char flashStr_Enum_Balancing[] PROGMEM = {"Balancing"};
            return flashStr_Enum_Balancing;
        } break;
        case AStr_Enum_Bellatrix: {
            static const char flashStr_Enum_Bellatrix[] PROGMEM = {"Bellatrix"};
            return flashStr_Enum_Bellatrix;
        } break;
        case AStr_Enum_Betelgeuse: {
            static const char flashStr_Enum_Betelgeuse[] PROGMEM = {"Betelgeuse"};
            return flashStr_Enum_Betelgeuse;
        } break;
        case AStr_Enum_CameraCooler: {
            static const char flashStr_Enum_CameraCooler[] PROGMEM = {"CameraCooler"};
            return flashStr_Enum_CameraCooler;
        } break;
        case AStr_Enum_CameraShutter: {
            static const char flashStr_Enum_CameraShutter[] PROGMEM = {"CameraShutter"};
            return flashStr_Enum_CameraShutter;
        } break;
        case AStr_Enum_CameraTemperature: {
            static const char flashStr_Enum_CameraTemperature[] PROGMEM = {"CameraTemperature"};
            return flashStr_Enum_CameraTemperature;
        } break;
        case AStr_Enum_Canopus: {
            static const char flashStr_Enum_Canopus[] PROGMEM = {"Canopus"};
            return flashStr_Enum_Canopus;
        } break;
        case AStr_Enum_Capella: {
            static const char flashStr_Enum_Capella[] PROGMEM = {"Capella"};
            return flashStr_Enum_Capella;
        } break;
        case AStr_Enum_Castor: {
            static const char flashStr_Enum_Castor[] PROGMEM = {"Castor"};
            return flashStr_Enum_Castor;
        } break;
        case AStr_Enum_Count: {
            static const char flashStr_Enum_Count[] PROGMEM = {"Count"};
            return flashStr_Enum_Count;
        } break;
        case AStr_Enum_Cover: {
            static const char flashStr_Enum_Cover[] PROGMEM = {"Cover"};
            return flashStr_Enum_Cover;
        } break;
        case AStr_Enum_Current: {
            static const char flashStr_Enum_Current[] PROGMEM = {"Current"};
            return flashStr_Enum_Current;
        } break;
        case AStr_Enum_CustomOLED: {
            static const char flashStr_Enum_CustomOLED[] PROGMEM = {"CustomOLED"};
            return flashStr_Enum_CustomOLED;
        } break;
        case AStr_Enum_CustomTarget1: {
            static const char flashStr_Enum_CustomTarget1[] PROGMEM = {"CustomTarget1"};
            return flashStr_Enum_CustomTarget1;
        } break;
        case AStr_Enum_CustomTarget2: {
            static const char flashStr_Enum_CustomTarget2[] PROGMEM = {"CustomTarget2"};
            return flashStr_Enum_CustomTarget2;
        } break;
        case AStr_Enum_CustomTarget3: {
            static const char flashStr_Enum_CustomTarget3[] PROGMEM = {"CustomTarget3"};
            return flashStr_Enum_CustomTarget3;
        } break;
        case AStr_Enum_CustomTarget4: {
            static const char flashStr_Enum_CustomTarget4[] PROGMEM = {"CustomTarget4"};
            return flashStr_Enum_CustomTarget4;
        } break;
        case AStr_Enum_CustomTarget5: {
            static const char flashStr_Enum_CustomTarget5[] PROGMEM = {"CustomTarget5"};
            return flashStr_Enum_CustomTarget5;
        } break;
        case AStr_Enum_CustomTarget6: {
            static const char flashStr_Enum_CustomTarget6[] PROGMEM = {"CustomTarget6"};
            return flashStr_Enum_CustomTarget6;
        } break;
        case AStr_Enum_CustomTarget7: {
            static const char flashStr_Enum_CustomTarget7[] PROGMEM = {"CustomTarget7"};
            return flashStr_Enum_CustomTarget7;
        } break;
        case AStr_Enum_CustomTarget8: {
            static const char flashStr_Enum_CustomTarget8[] PROGMEM = {"CustomTarget8"};
            return flashStr_Enum_CustomTarget8;
        } break;
        case AStr_Enum_DayStorage: {
            static const char flashStr_Enum_DayStorage[] PROGMEM = {"DayStorage"};
            return flashStr_Enum_DayStorage;
        } break;
        case AStr_Enum_DC12V: {
            static const char flashStr_Enum_DC12V[] PROGMEM = {"DC12V"};
            return flashStr_Enum_DC12V;
        } break;
        case AStr_Enum_DC24V: {
            static const char flashStr_Enum_DC24V[] PROGMEM = {"DC24V"};
            return flashStr_Enum_DC24V;
        } break;
        case AStr_Enum_DC3V3: {
            static const char flashStr_Enum_DC3V3[] PROGMEM = {"DC3V3"};
            return flashStr_Enum_DC3V3;
        } break;
        case AStr_Enum_DC48V: {
            static const char flashStr_Enum_DC48V[] PROGMEM = {"DC48V"};
            return flashStr_Enum_DC48V;
        } break;
        case AStr_Enum_DC5V: {
            static const char flashStr_Enum_DC5V[] PROGMEM = {"DC5V"};
            return flashStr_Enum_DC5V;
        } break;
        case AStr_Enum_Deneb: {
            static const char flashStr_Enum_Deneb[] PROGMEM = {"Deneb"};
            return flashStr_Enum_Deneb;
        } break;
        case AStr_Enum_Deploy: {
            static const char flashStr_Enum_Deploy[] PROGMEM = {"Deploy"};
            return flashStr_Enum_Deploy;
        } break;
        case AStr_Enum_DescOrder: {
            static const char flashStr_Enum_DescOrder[] PROGMEM = {"DescOrder"};
            return flashStr_Enum_DescOrder;
        } break;
        case AStr_Enum_DewHeater: {
            static const char flashStr_Enum_DewHeater[] PROGMEM = {"DewHeater"};
            return flashStr_Enum_DewHeater;
        } break;
        case AStr_Enum_DigitalInput: {
            static const char flashStr_Enum_DigitalInput[] PROGMEM = {"DigitalInput"};
            return flashStr_Enum_DigitalInput;
        } break;
        case AStr_Enum_DigitalInputPullDown: {
            static const char flashStr_Enum_DigitalInputPullDown[] PROGMEM = {"DigitalInputPullDown"};
            return flashStr_Enum_DigitalInputPullDown;
        } break;
        case AStr_Enum_DigitalInputPullUp: {
            static const char flashStr_Enum_DigitalInputPullUp[] PROGMEM = {"DigitalInputPullUp"};
            return flashStr_Enum_DigitalInputPullUp;
        } break;
        case AStr_Enum_DigitalOutput: {
            static const char flashStr_Enum_DigitalOutput[] PROGMEM = {"DigitalOutput"};
            return flashStr_Enum_DigitalOutput;
        } break;
        case AStr_Enum_DigitalOutputPushPull: {
            static const char flashStr_Enum_DigitalOutputPushPull[] PROGMEM = {"DigitalOutputPushPull"};
            return flashStr_Enum_DigitalOutputPushPull;
        } break;
        case AStr_Enum_Distance: {
            static const char flashStr_Enum_Distance[] PROGMEM = {"Distance"};
            return flashStr_Enum_Distance;
        } break;
        case AStr_Enum_Dubhe: {
            static const char flashStr_Enum_Dubhe[] PROGMEM = {"Dubhe"};
            return flashStr_Enum_Dubhe;
        } break;
        case AStr_Enum_Equatorial: {
            static const char flashStr_Enum_Equatorial[] PROGMEM = {"Equatorial"};
            return flashStr_Enum_Equatorial;
        } break;
        case AStr_Enum_Fan: {
            static const char flashStr_Enum_Fan[] PROGMEM = {"Fan"};
            return flashStr_Enum_Fan;
        } break;
        case AStr_Enum_Focuser: {
            static const char flashStr_Enum_Focuser[] PROGMEM = {"Focuser"};
            return flashStr_Enum_Focuser;
        } break;
        case AStr_Enum_Fomalhaut: {
            static const char flashStr_Enum_Fomalhaut[] PROGMEM = {"Fomalhaut"};
            return flashStr_Enum_Fomalhaut;
        } break;
        case AStr_Enum_Galaxy: {
            static const char flashStr_Enum_Galaxy[] PROGMEM = {"Galaxy"};
            return flashStr_Enum_Galaxy;
        } break;
        case AStr_Enum_GlobularCluster: {
            static const char flashStr_Enum_GlobularCluster[] PROGMEM = {"GlobularCluster"};
            return flashStr_Enum_GlobularCluster;
        } break;
        case AStr_Enum_Hadar: {
            static const char flashStr_Enum_Hadar[] PROGMEM = {"Hadar"};
            return flashStr_Enum_Hadar;
        } break;
        case AStr_Enum_Highest: {
            static const char flashStr_Enum_Highest[] PROGMEM = {"Highest"};
            return flashStr_Enum_Highest;
        } break;
        case AStr_Enum_Humidity: {
            static const char flashStr_Enum_Humidity[] PROGMEM = {"Humidity"};
            return flashStr_Enum_Humidity;
        } break;
        case AStr_Enum_IL3820: {
            static const char flashStr_Enum_IL3820[] PROGMEM = {"IL3820"};
            return flashStr_Enum_IL3820;
        } break;
        case AStr_Enum_IL3820V2: {
            static const char flashStr_Enum_IL3820V2[] PROGMEM = {"IL3820V2"};
            return flashStr_Enum_IL3820V2;
        } break;
        case AStr_Enum_ILI9341: {
            static const char flashStr_Enum_ILI9341[] PROGMEM = {"ILI9341"};
            return flashStr_Enum_ILI9341;
        } break;
        case AStr_Enum_Imperial: {
            static const char flashStr_Enum_Imperial[] PROGMEM = {"Imperial"};
            return flashStr_Enum_Imperial;
        } break;
        case AStr_Enum_Init: {
            static const char flashStr_Enum_Init[] PROGMEM = {"Init"};
            return flashStr_Enum_Init;
        } break;
        case AStr_Enum_InOrder: {
            static const char flashStr_Enum_InOrder[] PROGMEM = {"InOrder"};
            return flashStr_Enum_InOrder;
        } break;
        case AStr_Enum_Jupiter: {
            static const char flashStr_Enum_Jupiter[] PROGMEM = {"Jupiter"};
            return flashStr_Enum_Jupiter;
        } break;
        case AStr_Enum_LCD16x2: {
            static const char flashStr_Enum_LCD16x2[] PROGMEM = {"LCD16x2"};
            return flashStr_Enum_LCD16x2;
        } break;
        case AStr_Enum_LCD20x4: {
            static const char flashStr_Enum_LCD20x4[] PROGMEM = {"LCD20x4"};
            return flashStr_Enum_LCD20x4;
        } break;
        case AStr_Enum_Light: {
            static const char flashStr_Enum_Light[] PROGMEM = {"Light"};
            return flashStr_Enum_Light;
        } break;
        case AStr_Enum_LimitSwitch: {
            static const char flashStr_Enum_LimitSwitch[] PROGMEM = {"LimitSwitch"};
            return flashStr_Enum_LimitSwitch;
        } break;
        case AStr_Enum_Lowest: {
            static const char flashStr_Enum_Lowest[] PROGMEM = {"Lowest"};
            return flashStr_Enum_Lowest;
        } break;
        case AStr_Enum_M1: {
            static const char flashStr_Enum_M1[] PROGMEM = {"M1"};
            return flashStr_Enum_M1;
        } break;
        case AStr_Enum_M2: {
            static const char flashStr_Enum_M2[] PROGMEM = {"M2"};
            return flashStr_Enum_M2;
        } break;
        case AStr_Enum_M3: {
            static const char flashStr_Enum_M3[] PROGMEM = {"M3"};
            return flashStr_Enum_M3;
        } break;
        case AStr_Enum_M4: {
            static const char flashStr_Enum_M4[] PROGMEM = {"M4"};
            return flashStr_Enum_M4;
        } break;
        case AStr_Enum_M5: {
            static const char flashStr_Enum_M5[] PROGMEM = {"M5"};
            return flashStr_Enum_M5;
        } break;
        case AStr_Enum_M6: {
            static const char flashStr_Enum_M6[] PROGMEM = {"M6"};
            return flashStr_Enum_M6;
        } break;
        case AStr_Enum_M7: {
            static const char flashStr_Enum_M7[] PROGMEM = {"M7"};
            return flashStr_Enum_M7;
        } break;
        case AStr_Enum_M8: {
            static const char flashStr_Enum_M8[] PROGMEM = {"M8"};
            return flashStr_Enum_M8;
        } break;
        case AStr_Enum_M9: {
            static const char flashStr_Enum_M9[] PROGMEM = {"M9"};
            return flashStr_Enum_M9;
        } break;
        case AStr_Enum_M10: {
            static const char flashStr_Enum_M10[] PROGMEM = {"M10"};
            return flashStr_Enum_M10;
        } break;
        case AStr_Enum_M11: {
            static const char flashStr_Enum_M11[] PROGMEM = {"M11"};
            return flashStr_Enum_M11;
        } break;
        case AStr_Enum_M12: {
            static const char flashStr_Enum_M12[] PROGMEM = {"M12"};
            return flashStr_Enum_M12;
        } break;
        case AStr_Enum_M13: {
            static const char flashStr_Enum_M13[] PROGMEM = {"M13"};
            return flashStr_Enum_M13;
        } break;
        case AStr_Enum_M14: {
            static const char flashStr_Enum_M14[] PROGMEM = {"M14"};
            return flashStr_Enum_M14;
        } break;
        case AStr_Enum_M15: {
            static const char flashStr_Enum_M15[] PROGMEM = {"M15"};
            return flashStr_Enum_M15;
        } break;
        case AStr_Enum_M16: {
            static const char flashStr_Enum_M16[] PROGMEM = {"M16"};
            return flashStr_Enum_M16;
        } break;
        case AStr_Enum_M17: {
            static const char flashStr_Enum_M17[] PROGMEM = {"M17"};
            return flashStr_Enum_M17;
        } break;
        case AStr_Enum_M18: {
            static const char flashStr_Enum_M18[] PROGMEM = {"M18"};
            return flashStr_Enum_M18;
        } break;
        case AStr_Enum_M19: {
            static const char flashStr_Enum_M19[] PROGMEM = {"M19"};
            return flashStr_Enum_M19;
        } break;
        case AStr_Enum_M20: {
            static const char flashStr_Enum_M20[] PROGMEM = {"M20"};
            return flashStr_Enum_M20;
        } break;
        case AStr_Enum_M21: {
            static const char flashStr_Enum_M21[] PROGMEM = {"M21"};
            return flashStr_Enum_M21;
        } break;
        case AStr_Enum_M22: {
            static const char flashStr_Enum_M22[] PROGMEM = {"M22"};
            return flashStr_Enum_M22;
        } break;
        case AStr_Enum_M23: {
            static const char flashStr_Enum_M23[] PROGMEM = {"M23"};
            return flashStr_Enum_M23;
        } break;
        case AStr_Enum_M24: {
            static const char flashStr_Enum_M24[] PROGMEM = {"M24"};
            return flashStr_Enum_M24;
        } break;
        case AStr_Enum_M25: {
            static const char flashStr_Enum_M25[] PROGMEM = {"M25"};
            return flashStr_Enum_M25;
        } break;
        case AStr_Enum_M26: {
            static const char flashStr_Enum_M26[] PROGMEM = {"M26"};
            return flashStr_Enum_M26;
        } break;
        case AStr_Enum_M27: {
            static const char flashStr_Enum_M27[] PROGMEM = {"M27"};
            return flashStr_Enum_M27;
        } break;
        case AStr_Enum_M28: {
            static const char flashStr_Enum_M28[] PROGMEM = {"M28"};
            return flashStr_Enum_M28;
        } break;
        case AStr_Enum_M29: {
            static const char flashStr_Enum_M29[] PROGMEM = {"M29"};
            return flashStr_Enum_M29;
        } break;
        case AStr_Enum_M30: {
            static const char flashStr_Enum_M30[] PROGMEM = {"M30"};
            return flashStr_Enum_M30;
        } break;
        case AStr_Enum_M31: {
            static const char flashStr_Enum_M31[] PROGMEM = {"M31"};
            return flashStr_Enum_M31;
        } break;
        case AStr_Enum_M32: {
            static const char flashStr_Enum_M32[] PROGMEM = {"M32"};
            return flashStr_Enum_M32;
        } break;
        case AStr_Enum_M33: {
            static const char flashStr_Enum_M33[] PROGMEM = {"M33"};
            return flashStr_Enum_M33;
        } break;
        case AStr_Enum_M34: {
            static const char flashStr_Enum_M34[] PROGMEM = {"M34"};
            return flashStr_Enum_M34;
        } break;
        case AStr_Enum_M35: {
            static const char flashStr_Enum_M35[] PROGMEM = {"M35"};
            return flashStr_Enum_M35;
        } break;
        case AStr_Enum_M36: {
            static const char flashStr_Enum_M36[] PROGMEM = {"M36"};
            return flashStr_Enum_M36;
        } break;
        case AStr_Enum_M37: {
            static const char flashStr_Enum_M37[] PROGMEM = {"M37"};
            return flashStr_Enum_M37;
        } break;
        case AStr_Enum_M38: {
            static const char flashStr_Enum_M38[] PROGMEM = {"M38"};
            return flashStr_Enum_M38;
        } break;
        case AStr_Enum_M39: {
            static const char flashStr_Enum_M39[] PROGMEM = {"M39"};
            return flashStr_Enum_M39;
        } break;
        case AStr_Enum_M40: {
            static const char flashStr_Enum_M40[] PROGMEM = {"M40"};
            return flashStr_Enum_M40;
        } break;
        case AStr_Enum_M41: {
            static const char flashStr_Enum_M41[] PROGMEM = {"M41"};
            return flashStr_Enum_M41;
        } break;
        case AStr_Enum_M42: {
            static const char flashStr_Enum_M42[] PROGMEM = {"M42"};
            return flashStr_Enum_M42;
        } break;
        case AStr_Enum_M43: {
            static const char flashStr_Enum_M43[] PROGMEM = {"M43"};
            return flashStr_Enum_M43;
        } break;
        case AStr_Enum_M44: {
            static const char flashStr_Enum_M44[] PROGMEM = {"M44"};
            return flashStr_Enum_M44;
        } break;
        case AStr_Enum_M45: {
            static const char flashStr_Enum_M45[] PROGMEM = {"M45"};
            return flashStr_Enum_M45;
        } break;
        case AStr_Enum_M46: {
            static const char flashStr_Enum_M46[] PROGMEM = {"M46"};
            return flashStr_Enum_M46;
        } break;
        case AStr_Enum_M47: {
            static const char flashStr_Enum_M47[] PROGMEM = {"M47"};
            return flashStr_Enum_M47;
        } break;
        case AStr_Enum_M48: {
            static const char flashStr_Enum_M48[] PROGMEM = {"M48"};
            return flashStr_Enum_M48;
        } break;
        case AStr_Enum_M49: {
            static const char flashStr_Enum_M49[] PROGMEM = {"M49"};
            return flashStr_Enum_M49;
        } break;
        case AStr_Enum_M50: {
            static const char flashStr_Enum_M50[] PROGMEM = {"M50"};
            return flashStr_Enum_M50;
        } break;
        case AStr_Enum_M51: {
            static const char flashStr_Enum_M51[] PROGMEM = {"M51"};
            return flashStr_Enum_M51;
        } break;
        case AStr_Enum_M52: {
            static const char flashStr_Enum_M52[] PROGMEM = {"M52"};
            return flashStr_Enum_M52;
        } break;
        case AStr_Enum_M53: {
            static const char flashStr_Enum_M53[] PROGMEM = {"M53"};
            return flashStr_Enum_M53;
        } break;
        case AStr_Enum_M54: {
            static const char flashStr_Enum_M54[] PROGMEM = {"M54"};
            return flashStr_Enum_M54;
        } break;
        case AStr_Enum_M55: {
            static const char flashStr_Enum_M55[] PROGMEM = {"M55"};
            return flashStr_Enum_M55;
        } break;
        case AStr_Enum_M56: {
            static const char flashStr_Enum_M56[] PROGMEM = {"M56"};
            return flashStr_Enum_M56;
        } break;
        case AStr_Enum_M57: {
            static const char flashStr_Enum_M57[] PROGMEM = {"M57"};
            return flashStr_Enum_M57;
        } break;
        case AStr_Enum_M58: {
            static const char flashStr_Enum_M58[] PROGMEM = {"M58"};
            return flashStr_Enum_M58;
        } break;
        case AStr_Enum_M59: {
            static const char flashStr_Enum_M59[] PROGMEM = {"M59"};
            return flashStr_Enum_M59;
        } break;
        case AStr_Enum_M60: {
            static const char flashStr_Enum_M60[] PROGMEM = {"M60"};
            return flashStr_Enum_M60;
        } break;
        case AStr_Enum_M61: {
            static const char flashStr_Enum_M61[] PROGMEM = {"M61"};
            return flashStr_Enum_M61;
        } break;
        case AStr_Enum_M62: {
            static const char flashStr_Enum_M62[] PROGMEM = {"M62"};
            return flashStr_Enum_M62;
        } break;
        case AStr_Enum_M63: {
            static const char flashStr_Enum_M63[] PROGMEM = {"M63"};
            return flashStr_Enum_M63;
        } break;
        case AStr_Enum_M64: {
            static const char flashStr_Enum_M64[] PROGMEM = {"M64"};
            return flashStr_Enum_M64;
        } break;
        case AStr_Enum_M65: {
            static const char flashStr_Enum_M65[] PROGMEM = {"M65"};
            return flashStr_Enum_M65;
        } break;
        case AStr_Enum_M66: {
            static const char flashStr_Enum_M66[] PROGMEM = {"M66"};
            return flashStr_Enum_M66;
        } break;
        case AStr_Enum_M67: {
            static const char flashStr_Enum_M67[] PROGMEM = {"M67"};
            return flashStr_Enum_M67;
        } break;
        case AStr_Enum_M68: {
            static const char flashStr_Enum_M68[] PROGMEM = {"M68"};
            return flashStr_Enum_M68;
        } break;
        case AStr_Enum_M69: {
            static const char flashStr_Enum_M69[] PROGMEM = {"M69"};
            return flashStr_Enum_M69;
        } break;
        case AStr_Enum_M70: {
            static const char flashStr_Enum_M70[] PROGMEM = {"M70"};
            return flashStr_Enum_M70;
        } break;
        case AStr_Enum_M71: {
            static const char flashStr_Enum_M71[] PROGMEM = {"M71"};
            return flashStr_Enum_M71;
        } break;
        case AStr_Enum_M72: {
            static const char flashStr_Enum_M72[] PROGMEM = {"M72"};
            return flashStr_Enum_M72;
        } break;
        case AStr_Enum_M73: {
            static const char flashStr_Enum_M73[] PROGMEM = {"M73"};
            return flashStr_Enum_M73;
        } break;
        case AStr_Enum_M74: {
            static const char flashStr_Enum_M74[] PROGMEM = {"M74"};
            return flashStr_Enum_M74;
        } break;
        case AStr_Enum_M75: {
            static const char flashStr_Enum_M75[] PROGMEM = {"M75"};
            return flashStr_Enum_M75;
        } break;
        case AStr_Enum_M76: {
            static const char flashStr_Enum_M76[] PROGMEM = {"M76"};
            return flashStr_Enum_M76;
        } break;
        case AStr_Enum_M77: {
            static const char flashStr_Enum_M77[] PROGMEM = {"M77"};
            return flashStr_Enum_M77;
        } break;
        case AStr_Enum_M78: {
            static const char flashStr_Enum_M78[] PROGMEM = {"M78"};
            return flashStr_Enum_M78;
        } break;
        case AStr_Enum_M79: {
            static const char flashStr_Enum_M79[] PROGMEM = {"M79"};
            return flashStr_Enum_M79;
        } break;
        case AStr_Enum_M80: {
            static const char flashStr_Enum_M80[] PROGMEM = {"M80"};
            return flashStr_Enum_M80;
        } break;
        case AStr_Enum_M81: {
            static const char flashStr_Enum_M81[] PROGMEM = {"M81"};
            return flashStr_Enum_M81;
        } break;
        case AStr_Enum_M82: {
            static const char flashStr_Enum_M82[] PROGMEM = {"M82"};
            return flashStr_Enum_M82;
        } break;
        case AStr_Enum_M83: {
            static const char flashStr_Enum_M83[] PROGMEM = {"M83"};
            return flashStr_Enum_M83;
        } break;
        case AStr_Enum_M84: {
            static const char flashStr_Enum_M84[] PROGMEM = {"M84"};
            return flashStr_Enum_M84;
        } break;
        case AStr_Enum_M85: {
            static const char flashStr_Enum_M85[] PROGMEM = {"M85"};
            return flashStr_Enum_M85;
        } break;
        case AStr_Enum_M86: {
            static const char flashStr_Enum_M86[] PROGMEM = {"M86"};
            return flashStr_Enum_M86;
        } break;
        case AStr_Enum_M87: {
            static const char flashStr_Enum_M87[] PROGMEM = {"M87"};
            return flashStr_Enum_M87;
        } break;
        case AStr_Enum_M88: {
            static const char flashStr_Enum_M88[] PROGMEM = {"M88"};
            return flashStr_Enum_M88;
        } break;
        case AStr_Enum_M89: {
            static const char flashStr_Enum_M89[] PROGMEM = {"M89"};
            return flashStr_Enum_M89;
        } break;
        case AStr_Enum_M90: {
            static const char flashStr_Enum_M90[] PROGMEM = {"M90"};
            return flashStr_Enum_M90;
        } break;
        case AStr_Enum_M91: {
            static const char flashStr_Enum_M91[] PROGMEM = {"M91"};
            return flashStr_Enum_M91;
        } break;
        case AStr_Enum_M92: {
            static const char flashStr_Enum_M92[] PROGMEM = {"M92"};
            return flashStr_Enum_M92;
        } break;
        case AStr_Enum_M93: {
            static const char flashStr_Enum_M93[] PROGMEM = {"M93"};
            return flashStr_Enum_M93;
        } break;
        case AStr_Enum_M94: {
            static const char flashStr_Enum_M94[] PROGMEM = {"M94"};
            return flashStr_Enum_M94;
        } break;
        case AStr_Enum_M95: {
            static const char flashStr_Enum_M95[] PROGMEM = {"M95"};
            return flashStr_Enum_M95;
        } break;
        case AStr_Enum_M96: {
            static const char flashStr_Enum_M96[] PROGMEM = {"M96"};
            return flashStr_Enum_M96;
        } break;
        case AStr_Enum_M97: {
            static const char flashStr_Enum_M97[] PROGMEM = {"M97"};
            return flashStr_Enum_M97;
        } break;
        case AStr_Enum_M98: {
            static const char flashStr_Enum_M98[] PROGMEM = {"M98"};
            return flashStr_Enum_M98;
        } break;
        case AStr_Enum_M99: {
            static const char flashStr_Enum_M99[] PROGMEM = {"M99"};
            return flashStr_Enum_M99;
        } break;
        case AStr_Enum_M100: {
            static const char flashStr_Enum_M100[] PROGMEM = {"M100"};
            return flashStr_Enum_M100;
        } break;
        case AStr_Enum_M101: {
            static const char flashStr_Enum_M101[] PROGMEM = {"M101"};
            return flashStr_Enum_M101;
        } break;
        case AStr_Enum_M102: {
            static const char flashStr_Enum_M102[] PROGMEM = {"M102"};
            return flashStr_Enum_M102;
        } break;
        case AStr_Enum_M103: {
            static const char flashStr_Enum_M103[] PROGMEM = {"M103"};
            return flashStr_Enum_M103;
        } break;
        case AStr_Enum_M104: {
            static const char flashStr_Enum_M104[] PROGMEM = {"M104"};
            return flashStr_Enum_M104;
        } break;
        case AStr_Enum_M105: {
            static const char flashStr_Enum_M105[] PROGMEM = {"M105"};
            return flashStr_Enum_M105;
        } break;
        case AStr_Enum_M106: {
            static const char flashStr_Enum_M106[] PROGMEM = {"M106"};
            return flashStr_Enum_M106;
        } break;
        case AStr_Enum_M107: {
            static const char flashStr_Enum_M107[] PROGMEM = {"M107"};
            return flashStr_Enum_M107;
        } break;
        case AStr_Enum_M108: {
            static const char flashStr_Enum_M108[] PROGMEM = {"M108"};
            return flashStr_Enum_M108;
        } break;
        case AStr_Enum_M109: {
            static const char flashStr_Enum_M109[] PROGMEM = {"M109"};
            return flashStr_Enum_M109;
        } break;
        case AStr_Enum_M110: {
            static const char flashStr_Enum_M110[] PROGMEM = {"M110"};
            return flashStr_Enum_M110;
        } break;
        case AStr_Enum_Manual: {
            static const char flashStr_Enum_Manual[] PROGMEM = {"Manual"};
            return flashStr_Enum_Manual;
        } break;
        case AStr_Enum_Mars: {
            static const char flashStr_Enum_Mars[] PROGMEM = {"Mars"};
            return flashStr_Enum_Mars;
        } break;
        case AStr_Enum_Matrix2x2: {
            static const char flashStr_Enum_Matrix2x2[] PROGMEM = {"Matrix2x2"};
            return flashStr_Enum_Matrix2x2;
        } break;
        case AStr_Enum_Matrix3x4: {
            static const char flashStr_Enum_Matrix3x4[] PROGMEM = {"Matrix3x4"};
            return flashStr_Enum_Matrix3x4;
        } break;
        case AStr_Enum_Matrix4x4: {
            static const char flashStr_Enum_Matrix4x4[] PROGMEM = {"Matrix4x4"};
            return flashStr_Enum_Matrix4x4;
        } break;
        case AStr_Enum_Mercury: {
            static const char flashStr_Enum_Mercury[] PROGMEM = {"Mercury"};
            return flashStr_Enum_Mercury;
        } break;
        case AStr_Enum_Metric: {
            static const char flashStr_Enum_Metric[] PROGMEM = {"Metric"};
            return flashStr_Enum_Metric;
        } break;
        case AStr_Enum_Mizar: {
            static const char flashStr_Enum_Mizar[] PROGMEM = {"Mizar"};
            return flashStr_Enum_Mizar;
        } break;
        case AStr_Enum_Moon: {
            static const char flashStr_Enum_Moon[] PROGMEM = {"Moon"};
            return flashStr_Enum_Moon;
        } break;
        case AStr_Enum_MountAxis: {
            static const char flashStr_Enum_MountAxis[] PROGMEM = {"MountAxis"};
            return flashStr_Enum_MountAxis;
        } break;
        case AStr_Enum_Multiply: {
            static const char flashStr_Enum_Multiply[] PROGMEM = {"Multiply"};
            return flashStr_Enum_Multiply;
        } break;
        case AStr_Enum_Nebula: {
            static const char flashStr_Enum_Nebula[] PROGMEM = {"Nebula"};
            return flashStr_Enum_Nebula;
        } break;
        case AStr_Enum_Neptune: {
            static const char flashStr_Enum_Neptune[] PROGMEM = {"Neptune"};
            return flashStr_Enum_Neptune;
        } break;
        case AStr_Enum_NightObserving: {
            static const char flashStr_Enum_NightObserving[] PROGMEM = {"NightObserving"};
            return flashStr_Enum_NightObserving;
        } break;
        case AStr_Enum_OpenCluster: {
            static const char flashStr_Enum_OpenCluster[] PROGMEM = {"OpenCluster"};
            return flashStr_Enum_OpenCluster;
        } break;
        case AStr_Enum_Other: {
            static const char flashStr_Enum_Other[] PROGMEM = {"Other"};
            return flashStr_Enum_Other;
        } break;
        case AStr_Enum_Percentile: {
            static const char flashStr_Enum_Percentile[] PROGMEM = {"Percentile"};
            return flashStr_Enum_Percentile;
        } break;
        case AStr_Enum_PlanetaryNebula: {
            static const char flashStr_Enum_PlanetaryNebula[] PROGMEM = {"PlanetaryNebula"};
            return flashStr_Enum_PlanetaryNebula;
        } break;
        case AStr_Enum_Polaris: {
            static const char flashStr_Enum_Polaris[] PROGMEM = {"Polaris"};
            return flashStr_Enum_Polaris;
        } break;
        case AStr_Enum_Pollux: {
            static const char flashStr_Enum_Pollux[] PROGMEM = {"Pollux"};
            return flashStr_Enum_Pollux;
        } break;
        case AStr_Enum_Position: {
            static const char flashStr_Enum_Position[] PROGMEM = {"Position"};
            return flashStr_Enum_Position;
        } break;
        case AStr_Enum_Power: {
            static const char flashStr_Enum_Power[] PROGMEM = {"Power"};
            return flashStr_Enum_Power;
        } break;
        case AStr_Enum_Procyon: {
            static const char flashStr_Enum_Procyon[] PROGMEM = {"Procyon"};
            return flashStr_Enum_Procyon;
        } break;
        case AStr_Enum_Rain: {
            static const char flashStr_Enum_Rain[] PROGMEM = {"Rain"};
            return flashStr_Enum_Rain;
        } break;
        case AStr_Enum_Raw: {
            static const char flashStr_Enum_Raw[] PROGMEM = {"Raw"};
            return flashStr_Enum_Raw;
        } break;
        case AStr_Enum_Regulus: {
            static const char flashStr_Enum_Regulus[] PROGMEM = {"Regulus"};
            return flashStr_Enum_Regulus;
        } break;
        case AStr_Enum_RemoteControl: {
            static const char flashStr_Enum_RemoteControl[] PROGMEM = {"RemoteControl"};
            return flashStr_Enum_RemoteControl;
        } break;
        case AStr_Enum_ResistiveTouch: {
            static const char flashStr_Enum_ResistiveTouch[] PROGMEM = {"ResistiveTouch"};
            return flashStr_Enum_ResistiveTouch;
        } break;
        case AStr_Enum_RevOrder: {
            static const char flashStr_Enum_RevOrder[] PROGMEM = {"RevOrder"};
            return flashStr_Enum_RevOrder;
        } break;
        case AStr_Enum_RigelKentaurus: {
            static const char flashStr_Enum_RigelKentaurus[] PROGMEM = {"RigelKentaurus"};
            return flashStr_Enum_RigelKentaurus;
        } break;
        case AStr_Enum_RotaryEncoder: {
            static const char flashStr_Enum_RotaryEncoder[] PROGMEM = {"RotaryEncoder"};
            return flashStr_Enum_RotaryEncoder;
        } break;
        case AStr_Enum_SafeStowed: {
            static const char flashStr_Enum_SafeStowed[] PROGMEM = {"SafeStowed"};
            return flashStr_Enum_SafeStowed;
        } break;
        case AStr_Enum_Saturn: {
            static const char flashStr_Enum_Saturn[] PROGMEM = {"Saturn"};
            return flashStr_Enum_Saturn;
        } break;
        case AStr_Enum_Scientific: {
            static const char flashStr_Enum_Scientific[] PROGMEM = {"Scientific"};
            return flashStr_Enum_Scientific;
        } break;
        case AStr_Enum_SH1106: {
            static const char flashStr_Enum_SH1106[] PROGMEM = {"SH1106"};
            return flashStr_Enum_SH1106;
        } break;
        case AStr_Enum_SingleAxis: {
            static const char flashStr_Enum_SingleAxis[] PROGMEM = {"SingleAxis"};
            return flashStr_Enum_SingleAxis;
        } break;
        case AStr_Enum_Sirius: {
            static const char flashStr_Enum_Sirius[] PROGMEM = {"Sirius"};
            return flashStr_Enum_Sirius;
        } break;
        case AStr_Enum_SolarSystem: {
            static const char flashStr_Enum_SolarSystem[] PROGMEM = {"SolarSystem"};
            return flashStr_Enum_SolarSystem;
        } break;
        case AStr_Enum_Speed: {
            static const char flashStr_Enum_Speed[] PROGMEM = {"Speed"};
            return flashStr_Enum_Speed;
        } break;
        case AStr_Enum_Spica: {
            static const char flashStr_Enum_Spica[] PROGMEM = {"Spica"};
            return flashStr_Enum_Spica;
        } break;
        case AStr_Enum_SSD1305: {
            static const char flashStr_Enum_SSD1305[] PROGMEM = {"SSD1305"};
            return flashStr_Enum_SSD1305;
        } break;
        case AStr_Enum_SSD1305x32Ada: {
            static const char flashStr_Enum_SSD1305x32Ada[] PROGMEM = {"SSD1305x32Ada"};
            return flashStr_Enum_SSD1305x32Ada;
        } break;
        case AStr_Enum_SSD1305x64Ada: {
            static const char flashStr_Enum_SSD1305x64Ada[] PROGMEM = {"SSD1305x64Ada"};
            return flashStr_Enum_SSD1305x64Ada;
        } break;
        case AStr_Enum_SSD1306: {
            static const char flashStr_Enum_SSD1306[] PROGMEM = {"SSD1306"};
            return flashStr_Enum_SSD1306;
        } break;
        case AStr_Enum_SSD1607: {
            static const char flashStr_Enum_SSD1607[] PROGMEM = {"SSD1607"};
            return flashStr_Enum_SSD1607;
        } break;
        case AStr_Enum_ST7735: {
            static const char flashStr_Enum_ST7735[] PROGMEM = {"ST7735"};
            return flashStr_Enum_ST7735;
        } break;
        case AStr_Enum_ST7789: {
            static const char flashStr_Enum_ST7789[] PROGMEM = {"ST7789"};
            return flashStr_Enum_ST7789;
        } break;
        case AStr_Enum_Star: {
            static const char flashStr_Enum_Star[] PROGMEM = {"Star"};
            return flashStr_Enum_Star;
        } break;
        case AStr_Enum_Stow: {
            static const char flashStr_Enum_Stow[] PROGMEM = {"Stow"};
            return flashStr_Enum_Stow;
        } break;
        case AStr_Enum_Sun: {
            static const char flashStr_Enum_Sun[] PROGMEM = {"Sun"};
            return flashStr_Enum_Sun;
        } break;
        case AStr_Enum_Temperature: {
            static const char flashStr_Enum_Temperature[] PROGMEM = {"Temperature"};
            return flashStr_Enum_Temperature;
        } break;
        case AStr_Enum_TFTTouch: {
            static const char flashStr_Enum_TFTTouch[] PROGMEM = {"TFTTouch"};
            return flashStr_Enum_TFTTouch;
        } break;
        case AStr_Enum_TouchScreen: {
            static const char flashStr_Enum_TouchScreen[] PROGMEM = {"TouchScreen"};
            return flashStr_Enum_TouchScreen;
        } break;
        case AStr_Enum_Track: {
            static const char flashStr_Enum_Track[] PROGMEM = {"Track"};
            return flashStr_Enum_Track;
        } break;
        case AStr_Enum_Tracking: {
            static const char flashStr_Enum_Tracking[] PROGMEM = {"Tracking"};
            return flashStr_Enum_Tracking;
        } break;
        case AStr_Enum_Unknown: {
            static const char flashStr_Enum_Unknown[] PROGMEM = {"Unknown"};
            return flashStr_Enum_Unknown;
        } break;
        case AStr_Enum_UpDownButtons: {
            static const char flashStr_Enum_UpDownButtons[] PROGMEM = {"UpDownButtons"};
            return flashStr_Enum_UpDownButtons;
        } break;
        case AStr_Enum_UpDownESP32Touch: {
            static const char flashStr_Enum_UpDownESP32Touch[] PROGMEM = {"UpDownESP32Touch"};
            return flashStr_Enum_UpDownESP32Touch;
        } break;
        case AStr_Enum_Uranus: {
            static const char flashStr_Enum_Uranus[] PROGMEM = {"Uranus"};
            return flashStr_Enum_Uranus;
        } break;
        case AStr_Enum_Vega: {
            static const char flashStr_Enum_Vega[] PROGMEM = {"Vega"};
            return flashStr_Enum_Vega;
        } break;
        case AStr_Enum_Venus: {
            static const char flashStr_Enum_Venus[] PROGMEM = {"Venus"};
            return flashStr_Enum_Venus;
        } break;
        case AStr_Enum_Voltage: {
            static const char flashStr_Enum_Voltage[] PROGMEM = {"Voltage"};
            return flashStr_Enum_Voltage;
        } break;
        case AStr_Enum_Warm: {
            static const char flashStr_Enum_Warm[] PROGMEM = {"Warm"};
            return flashStr_Enum_Warm;
        } break;
        case AStr_Enum_WindSpeed: {
            static const char flashStr_Enum_WindSpeed[] PROGMEM = {"WindSpeed"};
            return flashStr_Enum_WindSpeed;
        } break;
        case AStr_Unit_Count: {
            static const char flashStr_Unit_Count[] PROGMEM = {"[qty]"};
            return flashStr_Unit_Count;
        } break;
        case AStr_Unit_Degree: {
            static const char flashStr_Unit_Degree[] PROGMEM = {"\xC2\xB0"};
            return flashStr_Unit_Degree;
        } break;
        case AStr_Unit_Feet: {
            static const char flashStr_Unit_Feet[] PROGMEM = {"ft"};
            return flashStr_Unit_Feet;
        } break;
        case AStr_Unit_PerSecond: {
            static const char flashStr_Unit_PerSecond[] PROGMEM = {"/s"};
            return flashStr_Unit_PerSecond;
        } break;
        case AStr_Unit_Radians: {
            static const char flashStr_Unit_Radians[] PROGMEM = {"rad"};
            return flashStr_Unit_Radians;
        } break;
        case AStr_Unit_Undefined: {
            static const char flashStr_Unit_Undefined[] PROGMEM = {"[undef]"};
            return flashStr_Unit_Undefined;
        } break;
        default:
            return AStr_Blank;
    }
}

#endif // /ifndef ASTRO_DISABLE_BUILTIN_DATA
