/*  Astruino: UI strings/prototypes.
    Copyright (C) 2026 NachtRaveVL
    Astruino UI Strings/Prototypes
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroUIStrings_H
#define AstroUIStrings_H

enum AstroUI_String : unsigned short {
    AUIStr_Keys_MatrixActions,

    AUIStr_Item_Actuators,
    AUIStr_Item_AddNew,
    AUIStr_Item_Alerts,
    AUIStr_Item_AllowRemoteCtrl,
    AUIStr_Item_Altitude,
    AUIStr_Item_AssignByDHCP,
    AUIStr_Item_AssignByHostname,
    AUIStr_Item_AutosavePrimary,
    AUIStr_Item_AutosaveSecondary,
    AUIStr_Item_BackToOverview,
    AUIStr_Item_BatteryFailure,
    AUIStr_Item_Board,
    AUIStr_Item_BrokerPort,
    AUIStr_Item_Browse,
    AUIStr_Item_Calibrations,
    AUIStr_Item_ControlMode,
    AUIStr_Item_ControllerIP,
    AUIStr_Item_Controls,
    AUIStr_Item_DSTAddHour,
    AUIStr_Item_DataPolling,
    AUIStr_Item_Date,
    AUIStr_Item_Debug,
    AUIStr_Item_DisplayMode,
    AUIStr_Item_EEPROM,
    AUIStr_Item_Firmware,
    AUIStr_Item_FreeMemory,
    AUIStr_Item_GPSPolling,
    AUIStr_Item_General,
    AUIStr_Item_Information,
    AUIStr_Item_JoystickXMid,
    AUIStr_Item_JoystickXTol,
    AUIStr_Item_JoystickYMid,
    AUIStr_Item_JoystickYTol,
    AUIStr_Item_LatDegrees,
    AUIStr_Item_Library,
    AUIStr_Item_LocalTime,
    AUIStr_Item_Location,
    AUIStr_Item_LongMinutes,
    AUIStr_Item_MACAddr0x,
    AUIStr_Item_MQTTBroker,
    AUIStr_Item_Measurements,
    AUIStr_Item_Name,
    AUIStr_Item_Networking,
    AUIStr_Item_PowerRails,
    AUIStr_Item_RTC,
    AUIStr_Item_RemoteCtrlPort,
    AUIStr_Item_Scheduling,
    AUIStr_Item_SDCard,
    AUIStr_Item_SimhubConnected,
    AUIStr_Item_Sensors,
    AUIStr_Item_Settings,
    AUIStr_Item_Size,
    AUIStr_Item_System,
    AUIStr_Item_SystemMode,
    AUIStr_Item_SystemName,
    AUIStr_Item_Time,
    AUIStr_Item_TimeZone,
    AUIStr_Item_ToggleBadConn,
    AUIStr_Item_ToggleFastTime,
    AUIStr_Item_TriggerAutosave,
    AUIStr_Item_TriggerLowMem,
    AUIStr_Item_TriggerSDCleanup,
    AUIStr_Item_TriggerSigTime,
    AUIStr_Item_TriggerSigLocation,
    AUIStr_Item_Uptime,
    AUIStr_Item_WiFiPass,
    AUIStr_Item_WiFiSSID,
    AUIStr_Item_Mounts,
    AUIStr_Item_Targets,
    AUIStr_Item_Thermal,

    AUIStr_Enum_Autosave,
    AUIStr_Enum_DataPolling,
    AUIStr_Enum_GPSPolling,
    AUIStr_Enum_Measurements,
    AUIStr_Enum_SystemMode,
    AUIStr_Enum_TimeZone,

    AUIStr_Unit_MSL,
    AUIStr_Unit_Percent,

    AUIStr_Count
};

extern String stringFromPGM(AstroUI_String strNum);
extern void beginUIStringsFromEEPROM(uint16_t uiDataAddress);
extern void beginUIStringsFromSDCard(String uiDataFilePrefix);
#ifndef ASTRO_DISABLE_BUILTIN_DATA
const char *pgmAddrForStr(AstroUI_String strNum);
#endif
extern size_t enumListPitch(const char *enumData);
inline size_t enumListPitch(AstroUI_String strNum) { return enumListPitch(CFP(strNum)); }

#endif // /ifndef AstroUIStrings_H
#endif
