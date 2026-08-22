/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Strings
*/

#ifndef AstroStrings_H
#define AstroStrings_H

#include "Astruino.h"

// Library String Enumeration
// Identifies common library strings that are stored in program Flash when built-in data is enabled.
enum Astro_String : uint16_t {
    AStr_Tracking,                           // "Tracking"
    AStr_Balancing,                          // "Balancing"
    AStr_Manual,                             // "Manual"
    AStr_Enum_Count,                         // "Count"
    AStr_Undefined,                          // "Undefined"
    AStr_Imperial,                           // "Imperial"
    AStr_Metric,                             // "Metric"
    AStr_Scientific,                         // "Scientific"
    AStr_MountAxis,                          // "MountAxis"
    AStr_Cover,                              // "Cover"
    AStr_DewHeater,                          // "DewHeater"
    AStr_CameraCooler,                       // "CameraCooler"
    AStr_Fan,                                // "Fan"
    AStr_Focuser,                            // "Focuser"
    AStr_Generic,                            // "Generic"
    AStr_Temperature,                        // "Temperature"
    AStr_Humidity,                           // "Humidity"
    AStr_Position,                           // "Position"
    AStr_LimitSwitch,                        // "LimitSwitch"
    AStr_Rain,                               // "Rain"
    AStr_WindSpeed,                          // "WindSpeed"
    AStr_Light,                              // "Light"
    AStr_Voltage,                            // "Voltage"
    AStr_Current,                            // "Current"
    AStr_CameraTemperature,                  // "CameraTemperature"
    AStr_Equatorial,                         // "Equatorial"
    AStr_AltAz,                              // "AltAz"
    AStr_SingleAxis,                         // "SingleAxis"
    AStr_DC3V3,                              // "DC3V3"
    AStr_DC5V,                               // "DC5V"
    AStr_DC12V,                              // "DC12V"
    AStr_DC24V,                              // "DC24V"
    AStr_DigitalInput,                       // "DigitalInput"
    AStr_DigitalInputPullUp,                 // "DigitalInputPullUp"
    AStr_DigitalInputPullDown,               // "DigitalInputPullDown"
    AStr_DigitalOutput,                      // "DigitalOutput"
    AStr_DigitalOutputPushPull,              // "DigitalOutputPushPull"
    AStr_AnalogInput,                        // "AnalogInput"
    AStr_AnalogOutput,                       // "AnalogOutput"
    AStr_Highest,                            // "Highest"
    AStr_Lowest,                             // "Lowest"
    AStr_Average,                            // "Average"
    AStr_Multiply,                           // "Multiply"
    AStr_InOrder,                            // "InOrder"
    AStr_RevOrder,                           // "RevOrder"
    AStr_Raw,                                // "Raw"
    AStr_Angle,                              // "Angle"
    AStr_Distance,                           // "Distance"
    AStr_Percentile,                         // "Percentile"
    AStr_Speed,                              // "Speed"
    AStr_Power,                              // "Power"
    AStr_N1,                                 // "1"
    AStr_deg,                                // "deg"
    AStr_rad,                                // "rad"
    AStr_m,                                  // "m"
    AStr_ft,                                 // "ft"
    AStr_Percent,                            // "%"
    AStr_mPers,                              // "m/s"
    AStr_ftPers,                             // "ft/s"
    AStr_C,                                  // "C"
    AStr_F,                                  // "F"
    AStr_K,                                  // "K"
    AStr_PercentRH,                          // "%RH"
    AStr_W,                                  // "W"
    AStr_V,                                  // "V"
    AStr_A,                                  // "A"
    AStr_Star,                               // "Star"
    AStr_OpenCluster,                        // "OpenCluster"
    AStr_GlobularCluster,                    // "GlobularCluster"
    AStr_Nebula,                             // "Nebula"
    AStr_PlanetaryNebula,                    // "PlanetaryNebula"
    AStr_Galaxy,                             // "Galaxy"
    AStr_SolarSystem,                        // "SolarSystem"
    AStr_Other,                              // "Other"
    AStr_DayStorage,                         // "DayStorage"
    AStr_NightObserving,                     // "NightObserving"
    AStr_SafeStowed,                         // "SafeStowed"
    AStr_DayStowed,                          // "DayStowed"
    AStr_Deploying,                          // "Deploying"
    AStr_Cooling,                            // "Cooling"
    AStr_Slewing,                            // "Slewing"
    AStr_Settling,                           // "Settling"
    AStr_Observing,                          // "Observing"
    AStr_Warming,                            // "Warming"
    AStr_Stowing,                            // "Stowing"
    AStr_Fault,                              // "Fault"
    AStr_Empty,                              // Blank string
    AStr_Astruino,                           // "Astruino"
    AStr_AstruinoDotCfg,                     // "Astruino.cfg"
    AStr_EnvironmentReport,                  // "Environment report"
    AStr_ObservationStarted,                 // "Observation started"
    AStr_ObservationStopped,                 // "Observation stopped"
    AStr_CoverOpening,                       // "Cover opening"
    AStr_CoverClosing,                       // "Cover closing"
    AStr_SystemDataSaved,                    // "System data saved"
    AStr_SystemUptimeColon,                  // "System uptime: "
    AStr_AllocationFailure,                  // "Allocation failure"
    AStr_InvalidParameter,                   // "Invalid parameter"
    AStr_ImportFailure,                      // "Import failure"
    AStr_ExportFailure,                      // "Export failure"
    AStr_OperationFailure,                   // "Operation failure"
    AStr_AlreadyInitialized,                 // "Already initialized"
    AStr_NotYetInitialized,                  // "Not yet initialized"
    AStr_null,                               // "null"
    AStr_csv,                                // "csv"
    AStr_dat,                                // "dat"
    AStr_txt,                                // "txt"
    AStr_Colon,                              // ": "
    AStr_Blank2,                             // "  "

    AStr_Count                                      // String count
};

// Blank string pointer used when no Flash string is available.
extern const char *AStr_Blank;

// Returns memory resident string from PROGMEM (Flash), EEPROM, or SD-backed string number.
extern AstroString stringFromPGM(Astro_String strNum);
#define SFP(strNum) stringFromPGM((strNum))

// Returns memory resident string from a PROGMEM (Flash) string address.
extern AstroString stringFromPGMAddr(const char *flashStr);

// Makes string lookup go through EEPROM at the specified data begin address.
extern void beginStringsFromEEPROM(uint16_t dataAddress);

// Makes string lookup go through an SD card strings file at the specified file prefix.
extern void beginStringsFromSDCard(const AstroString &dataFilePrefix);

#ifndef ASTRO_DISABLE_BUILTIN_DATA
// Returns the PROGMEM (Flash) address pointer for a library string.
extern const char *pgmAddrForStr(Astro_String strNum);
#define CFP(strNum) pgmAddrForStr((strNum))
#else
#define CFP(strNum) SFP(strNum).c_str()
#endif

#endif // /ifndef AstroStrings_H
