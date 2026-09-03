/*  Astruino: Simple automation controller for solar tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino UI Strings
*/

#include "AstruinoUI.h"
#ifdef ASTRO_USE_GUI

static uint16_t _uiStrDataAddress((uint16_t)-1);
void beginUIStringsFromEEPROM(uint16_t uiDataAddress)
{
    _uiStrDataAddress = uiDataAddress;
}

static String _uiStrDataFilePrefix;
void beginUIStringsFromSDCard(String uiDataFilePrefix)
{
    _uiStrDataFilePrefix = uiDataFilePrefix;
}

inline String getUIStringsFilename()
{
    String filename; filename.reserve(_uiStrDataFilePrefix.length() + 11 + 1);
    filename.concat(_uiStrDataFilePrefix);
    filename.concat('u'); // Cannot use SFP here so have to do it the long way
    filename.concat('i');
    filename.concat('d');
    filename.concat('s');
    filename.concat('t');
    filename.concat('r');
    filename.concat('s');
    filename.concat('.');
    filename.concat('d');
    filename.concat('a');
    filename.concat('t');
    return filename;
}

String stringFromPGM(AstroUI_String strNum)
{
    static AstroUI_String _lookupStrNum = (AstroUI_String)-1; // Simple LRU cache reduces a lot of lookup access
    static String _lookupCachedRes;
    if (strNum == _lookupStrNum) { return _lookupCachedRes; }
    else { _lookupStrNum = strNum; } // _lookupCachedRes set below

    if (_uiStrDataAddress != (uint16_t)-1) {
        auto eeprom = getController()->getEEPROM();

        if (eeprom) {
            uint16_t lookupOffset = 0;
            eeprom->readBlock(_uiStrDataAddress + (sizeof(uint16_t) * ((int)strNum + 1)), // +1 for initial total size word
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

    if (_uiStrDataFilePrefix.length()) {
        #if ASTRO_SYS_LEAVE_FILES_OPEN
            static
        #endif
        auto sd = getController()->getSDCard();

        if (sd) {
            String retVal;
            #if ASTRO_SYS_LEAVE_FILES_OPEN
                static
            #endif
            auto file = sd->open(getUIStringsFilename().c_str(), FILE_READ);

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

#ifndef ASTRO_DISABLE_BUILTIN_DATA

const char *pgmAddrForStr(AstroUI_String strNum)
{
    switch(strNum) {
        case AUIStr_Keys_MatrixActions: {
            static const char flashUIStr_Keys_MatrixActions[] PROGMEM = {ASTRO_UI_MATRIX_ACTIONS};
            return flashUIStr_Keys_MatrixActions;
        } break;

        case AUIStr_Item_Actuators: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Actuators = { "Actuators", 20, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Actuators;
        } break;
        case AUIStr_Item_AddNew: {
            static const char flashUIStr_Item_AddNew[] PROGMEM = {"Add New"};
            return flashUIStr_Item_AddNew;
        } break;
        case AUIStr_Item_Alerts: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Alerts = { "Alerts", 1, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Alerts;
        } break;
        case AUIStr_Item_AllowRemoteCtrl: {
            static const char flashUIStr_Item_AllowRemoteCtrl[] PROGMEM = {"Allow Remote Ctrl"};
            return flashUIStr_Item_AllowRemoteCtrl;
        } break;
        case AUIStr_Item_Altitude: {
            static const char flashUIStr_Item_Altitude[] PROGMEM = {"Altitude"};
            return flashUIStr_Item_Altitude;
        } break;
        case AUIStr_Item_AssignByDHCP: {
            static const char flashUIStr_Item_AssignByDHCP[] PROGMEM = {"Assign by DHCP"};
            return flashUIStr_Item_AssignByDHCP;
        } break;
        case AUIStr_Item_AssignByHostname: {
            static const char flashUIStr_Item_AssignByHostname[] PROGMEM = {"Assign by Hostname"};
            return flashUIStr_Item_AssignByHostname;
        } break;
        case AUIStr_Item_AutosavePrimary: {
            static const char flashUIStr_Item_AutosavePrimary[] PROGMEM = {"Autosave Primary"};
            return flashUIStr_Item_AutosavePrimary;
        } break;
        case AUIStr_Item_AutosaveSecondary: {
            static const char flashUIStr_Item_AutosaveSecondary[] PROGMEM = {"Autosave Secondary"};
            return flashUIStr_Item_AutosaveSecondary;
        } break;
        case AUIStr_Item_BackToOverview: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_BackToOverview = { "Back to Overview", 7, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_BackToOverview;
        } break;
        case AUIStr_Item_BatteryFailure: {
            static const char flashUIStr_Item_BatteryFailure[] PROGMEM = {"Battery Failure"};
            return flashUIStr_Item_BatteryFailure;
        } break;
        case AUIStr_Item_Board: {
            static const char flashUIStr_Item_Board[] PROGMEM = {"Board"};
            return flashUIStr_Item_Board;
        } break;
        case AUIStr_Item_BrokerPort: {
            static const char flashUIStr_Item_BrokerPort[] PROGMEM = {"Broker Port"};
            return flashUIStr_Item_BrokerPort;
        } break;
        case AUIStr_Item_Browse: {
            static const char flashUIStr_Item_Browse[] PROGMEM = {"Browse"};
            return flashUIStr_Item_Browse;
        } break;
        case AUIStr_Item_Calibrations: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Calibrations = { "Calibrations", 42, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Calibrations;
        } break;
        case AUIStr_Item_ControlMode: {
            static const char flashUIStr_Item_ControlMode[] PROGMEM = {"Control Mode"};
            return flashUIStr_Item_ControlMode;
        } break;
        case AUIStr_Item_ControllerIP: {
            static const char flashUIStr_Item_ControllerIP[] PROGMEM = {"Controller IP"};
            return flashUIStr_Item_ControllerIP;
        } break;
        case AUIStr_Item_Controls: {
            static const char flashUIStr_Item_Controls[] PROGMEM = {"Controls"};
            return flashUIStr_Item_Controls;
        } break;
        case AUIStr_Item_DSTAddHour: {
            static const char flashUIStr_Item_DSTAddHour[] PROGMEM = {"DST Add Hour"};
            return flashUIStr_Item_DSTAddHour;
        } break;
        case AUIStr_Item_DataPolling: {
            static const char flashUIStr_Item_DataPolling[] PROGMEM = {"Data Polling"};
            return flashUIStr_Item_DataPolling;
        } break;
        case AUIStr_Item_Date: {
            static const char flashUIStr_Item_Date[] PROGMEM = {"Date"};
            return flashUIStr_Item_Date;
        } break;
        case AUIStr_Item_Debug: {
            static const PROGMEM SubMenuInfo flashUIStr_Item_Debug = { "Debug", 6, NO_ADDRESS, 0, NO_CALLBACK };
            return (const char *)&flashUIStr_Item_Debug;
        } break;
        case AUIStr_Item_DisplayMode: {
            static const char flashUIStr_Item_DisplayMode[] PROGMEM = {"Display Mode"};
            return flashUIStr_Item_DisplayMode;
        } break;
        case AUIStr_Item_EEPROM: {
            static const char flashUIStr_Item_EEPROM[] PROGMEM = {"EEPROM"};
            return flashUIStr_Item_EEPROM;
        } break;
        case AUIStr_Item_Firmware: {
            static const char flashUIStr_Item_Firmware[] PROGMEM = {"Firmware"};
            return flashUIStr_Item_Firmware;
        } break;
        case AUIStr_Item_FreeMemory: {
            static const char flashUIStr_Item_FreeMemory[] PROGMEM = {"Free Memory"};
            return flashUIStr_Item_FreeMemory;
        } break;
        case AUIStr_Item_GPSPolling: {
            static const char flashUIStr_Item_GPSPolling[] PROGMEM = {"GPS Polling"};
            return flashUIStr_Item_GPSPolling;
        } break;
        case AUIStr_Item_General: {
            static const char flashUIStr_Item_General[] PROGMEM = {"General"};
            return flashUIStr_Item_General;
        } break;
        case AUIStr_Item_Information: {
            static const PROGMEM BooleanMenuInfo flashUIStr_Item_Information = { "Information", 5, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Information;
        } break;
        case AUIStr_Item_JoystickXMid: {
            static const char flashUIStr_Item_JoystickXMid[] PROGMEM = {"Joystick X Mid"};
            return flashUIStr_Item_JoystickXMid;
        } break;
        case AUIStr_Item_JoystickXTol: {
            static const char flashUIStr_Item_JoystickXTol[] PROGMEM = {"Joystick X Tol"};
            return flashUIStr_Item_JoystickXTol;
        } break;
        case AUIStr_Item_JoystickYMid: {
            static const char flashUIStr_Item_JoystickYMid[] PROGMEM = {"Joystick Y Mid"};
            return flashUIStr_Item_JoystickYMid;
        } break;
        case AUIStr_Item_JoystickYTol: {
            static const char flashUIStr_Item_JoystickYTol[] PROGMEM = {"Joystick Y Tol"};
            return flashUIStr_Item_JoystickYTol;
        } break;
        case AUIStr_Item_LatDegrees: {
            static const char flashUIStr_Item_LatDegrees[] PROGMEM = {"Lat Degrees"};
            return flashUIStr_Item_LatDegrees;
        } break;
        case AUIStr_Item_Library: {
            static const PROGMEM SubMenuInfo flashUIStr_Item_Library = { "Library", 4, NO_ADDRESS, 0, NO_CALLBACK };
            return (const char *)&flashUIStr_Item_Library;
        } break;
        case AUIStr_Item_LocalTime: {
            static const char flashUIStr_Item_LocalTime[] PROGMEM = {"Local Time"};
            return flashUIStr_Item_LocalTime;
        } break;
        case AUIStr_Item_Location: {
            static const char flashUIStr_Item_Location[] PROGMEM = {"Location"};
            return flashUIStr_Item_Location;
        } break;
        case AUIStr_Item_LongMinutes: {
            static const char flashUIStr_Item_LongMinutes[] PROGMEM = {"Long Minutes"};
            return flashUIStr_Item_LongMinutes;
        } break;
        case AUIStr_Item_MACAddr0x: {
            static const char flashUIStr_Item_MACAddr0x[] PROGMEM = {"MAC Addr 0x"};
            return flashUIStr_Item_MACAddr0x;
        } break;
        case AUIStr_Item_MQTTBroker: {
            static const char flashUIStr_Item_MQTTBroker[] PROGMEM = {"MQTT Broker"};
            return flashUIStr_Item_MQTTBroker;
        } break;
        case AUIStr_Item_Measurements: {
            static const char flashUIStr_Item_Measurements[] PROGMEM = {"Measurements"};
            return flashUIStr_Item_Measurements;
        } break;
        case AUIStr_Item_Name: {
            static const char flashUIStr_Item_Name[] PROGMEM = {"Name"};
            return flashUIStr_Item_Name;
        } break;
        case AUIStr_Item_Networking: {
            static const char flashUIStr_Item_Networking[] PROGMEM = {"Networking"};
            return flashUIStr_Item_Networking;
        } break;
        case AUIStr_Item_Mounts: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Mounts = { "Mounts", 22, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Mounts;
        } break;
        case AUIStr_Item_Targets: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Targets = { "Targets", 23, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Targets;
        } break;
        case AUIStr_Item_Thermal: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Thermal = { "Thermal", 26, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Thermal;
        } break;
        case AUIStr_Item_PowerRails: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_PowerRails = { "Power Rails", 24, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_PowerRails;
        } break;
        case AUIStr_Item_RTC: {
            static const char flashUIStr_Item_RTC[] PROGMEM = {"RTC"};
            return flashUIStr_Item_RTC;
        } break;
        case AUIStr_Item_RemoteCtrlPort: {
            static const char flashUIStr_Item_RemoteCtrlPort[] PROGMEM = {"Remote Ctrl Port"};
            return flashUIStr_Item_RemoteCtrlPort;
        } break;
        case AUIStr_Item_Scheduling: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Scheduling = { "Scheduling", 25, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Scheduling;
        } break;
        case AUIStr_Item_SDCard: {
            static const char flashUIStr_Item_SDCard[] PROGMEM = {"SD Card"};
            return flashUIStr_Item_SDCard;
        } break;
        case AUIStr_Item_SimhubConnected: {
            static const PROGMEM BooleanMenuInfo flashUIStr_Item_SimhubConnected = { "Simhub Connected", 60, NO_ADDRESS, 1, NO_CALLBACK, NAMING_CHECKBOX };
            return (const char *)&flashUIStr_Item_SimhubConnected;
        } break;
        case AUIStr_Item_Sensors: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Sensors = { "Sensors", 21, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Sensors;
        } break;
        case AUIStr_Item_Settings: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Settings = { "Settings", 3, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Settings;
        } break;
        case AUIStr_Item_Size: {
            static const char flashUIStr_Item_Size[] PROGMEM = {"Size"};
            return flashUIStr_Item_Size;
        } break;
        case AUIStr_Item_System: {
            static const PROGMEM SubMenuInfo flashUIStr_Item_System = { "System", 2, NO_ADDRESS, 0, NO_CALLBACK };
            return (const char *)&flashUIStr_Item_System;
        } break;
        case AUIStr_Item_SystemMode: {
            static const char flashUIStr_Item_SystemMode[] PROGMEM = {"System Mode"};
            return flashUIStr_Item_SystemMode;
        } break;
        case AUIStr_Item_SystemName: {
            static const char flashUIStr_Item_SystemName[] PROGMEM = {"System Name"};
            return flashUIStr_Item_SystemName;
        } break;
        case AUIStr_Item_Time: {
            static const char flashUIStr_Item_Time[] PROGMEM = {"Time"};
            return flashUIStr_Item_Time;
        } break;
        case AUIStr_Item_TimeZone: {
            static const char flashUIStr_Item_TimeZone[] PROGMEM = {"Time Zone"};
            return flashUIStr_Item_TimeZone;
        } break;
        case AUIStr_Item_TriggerAutosave: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_TriggerAutosave = { "Trigger Autosave", 61, NO_ADDRESS, 0, debugAction };
            return (const char *)&flashUIStr_Item_TriggerAutosave;
        } break;
        case AUIStr_Item_TriggerLowMem: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_TriggerLowMem = { "Trigger LowMem", 62, NO_ADDRESS, 0, debugAction };
            return (const char *)&flashUIStr_Item_TriggerLowMem;
        } break;
        case AUIStr_Item_TriggerSDCleanup: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_TriggerSDCleanup = { "Trigger SDCleanup", 63, NO_ADDRESS, 0, debugAction };
            return (const char *)&flashUIStr_Item_TriggerSDCleanup;
        } break;
        case AUIStr_Item_TriggerSigTime: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_TriggerSigTime = { "Trigger SigTime", 64, NO_ADDRESS, 0, debugAction };
            return (const char *)&flashUIStr_Item_TriggerSigTime;
        } break;
        case AUIStr_Item_TriggerSigLocation: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_TriggerSigLocation = { "Trigger SigLocation", 65, NO_ADDRESS, 0, debugAction };
            return (const char *)&flashUIStr_Item_TriggerSigLocation;
        } break;
        case AUIStr_Item_Uptime: {
            static const char flashUIStr_Item_Uptime[] PROGMEM = {"Uptime"};
            return flashUIStr_Item_Uptime;
        } break;
        case AUIStr_Item_WiFiPass: {
            static const char flashUIStr_Item_WiFiPass[] PROGMEM = {"WiFi Pass"};
            return flashUIStr_Item_WiFiPass;
        } break;
        case AUIStr_Item_WiFiSSID: {
            static const char flashUIStr_Item_WiFiSSID[] PROGMEM = {"WiFi SSID"};
            return flashUIStr_Item_WiFiSSID;
        } break;


        case AUIStr_Count: break;
    }
    return nullptr;
}

#endif // /ifndef ASTRO_DISABLE_BUILTIN_DATA

size_t enumListPitch(const char *enumData)
{
    if (!enumData) { return 0; }
    size_t size = 0;
    while (get_info_char(enumData) != '\000' && size < 64) {
        ++size; ++enumData;
    }
    while (get_info_char(enumData) == '\000' && size < 64) {
        ++size; ++enumData;
    }
    return size;
}

#endif
