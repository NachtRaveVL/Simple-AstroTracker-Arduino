/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Defines
*/

#ifndef AstroDefines_H
#define AstroDefines_H

#include "AstroCompat.h"


#ifndef ASTRO_FLT_EPSILON
#define ASTRO_FLT_EPSILON 0.00001f                         // Single-precision floating point error tolerance
#endif
#ifndef ASTRO_DBL_EPSILON
#define ASTRO_DBL_EPSILON 0.0000000000001                  // Double-precision floating point error tolerance
#endif

#ifndef ASTRO_STRING_BUFFER_SIZE
#define ASTRO_STRING_BUFFER_SIZE 32                       // General-purpose Flash/external string read buffer
#endif
#ifndef ASTRO_TARGET_NAME_MAXSIZE
#define ASTRO_TARGET_NAME_MAXSIZE 32                    // Maximum target display name length
#endif
#ifndef ASTRO_TARGET_ID_MAXSIZE
#define ASTRO_TARGET_ID_MAXSIZE 16                      // Maximum catalog/object identifier length
#endif
#ifndef ASTRO_NAME_MAXSIZE
#define ASTRO_NAME_MAXSIZE 32                              // Maximum general object/system name length
#endif
#ifndef ASTRO_PREFIX_MAXSIZE
#define ASTRO_PREFIX_MAXSIZE 32                            // Maximum log/data file prefix length
#endif
#ifndef ASTRO_LOG_MESSAGE_MAXSIZE
#define ASTRO_LOG_MESSAGE_MAXSIZE 96                    // Maximum formatted logger message length
#endif
#ifndef ASTRO_PUBLISH_MAX_COLUMNS
#define ASTRO_PUBLISH_MAX_COLUMNS 32                    // Maximum sensor columns collected by Publisher
#endif
#ifndef ASTRO_SYS_OBJECTS_MAXSIZE
#define ASTRO_SYS_OBJECTS_MAXSIZE 16                     // Maximum array size for system objects (max # of objects in system)
#endif
#ifndef ASTRO_POS_MAXSIZE
#define ASTRO_POS_MAXSIZE 32                                // Position indices maximum size (max # of objects of same type)
#endif
#ifndef ASTRO_ACTIVATION_HANDLE_SLOTS
#define ASTRO_ACTIVATION_HANDLE_SLOTS 8                 // Maximum simultaneous activation requests per actuator
#endif

// System Defaults
#define ASTRO_SYS_POLLING_INTERVAL      1000                // Default sensor polling interval, in milliseconds

// Mount Defaults
#define ASTRO_MOUNT_AXIS_RATE_DEGPS     8.0                 // Default simulated/limited mount axis rate, in degrees per second
#define ASTRO_MOUNT_SIDEREAL_RATE_DEGPS 0.0041780746        // Sidereal tracking rate, in degrees per second
#define ASTRO_MOUNT_GUIDE_RATE           0.5                 // Default pulse-guide rate as a multiple of sidereal

// Cover Defaults
#define ASTRO_COVER_TRAVEL_RATE          0.2f                // Default normalized cover travel rate per second
#define ASTRO_COVER_TRAVEL_TIMEOUT_SECS  120.0               // Default cover movement timeout, in seconds

// Scheduler Defaults
#define ASTRO_SCH_DEPLOY_SUN_ALT_DEG    -6.0                // Sun altitude below which nighttime deployment may begin
#define ASTRO_SCH_STOW_SUN_ALT_DEG      -3.0                // Sun altitude above which morning stow begins
#define ASTRO_SCH_ALIGN_TOL_DEG         0.35                // Default mount alignment tolerance, in degrees
#define ASTRO_SCH_SETTLE_SECS           5                   // Default stable alignment time before observation
#define ASTRO_SCH_REPORT_INTERVAL_SECS  (8UL * 60UL * 60UL) // Default environment report interval, in seconds
#define ASTRO_SCH_CAMERA_STABLE_DEG     2.0                 // Allowed camera temperature error before observing, in degrees C

// Thermal Defaults
#define ASTRO_THERMAL_DEW_MARGIN_C      3.0                 // Default optics margin above dew point, in degrees C
#define ASTRO_THERMAL_OPTICS_MAX_C      4.0                 // Maximum optics target above ambient, in degrees C
#define ASTRO_THERMAL_CAMERA_TARGET_C   -10.0               // Default requested cooled-camera sensor target, in degrees C
#define ASTRO_THERMAL_CAMERA_RAMP_CPM   2.0                 // Default camera cooling/warming ramp, in degrees C per minute
#define ASTRO_THERMAL_ELECTRONICS_MIN_C -5.0                // Default minimum electronics/body temperature, in degrees C

#define ASTRO_POS_SEARCH_FROMBEG ((aposi_t)-1)           // Search from beginning to end, 0 up to MAXSIZE-1
#define ASTRO_POS_SEARCH_FROMEND ((aposi_t)ASTRO_POS_MAXSIZE) // Search from end to beginning, MAXSIZE-1 down to 0


// System Run Mode
// Specifies how mount target positions are determined and controlled.
enum Astro_SystemMode : int8_t {
    Astro_SystemMode_Tracking,                               // Astronomical position tracking from time/location and target coordinates
    Astro_SystemMode_Balancing,                              // Sensor-feedback balancing/correction mode
    Astro_SystemMode_Manual,                                 // User/external code controls mount targets directly
    Astro_SystemMode_Count,                                  // Placeholder
    Astro_SystemMode_Undefined = -1                         // Placeholder
};

// Measurement Units Mode
// Specifies the preferred measurement system used by sensors and reporting.
enum Astro_MeasurementMode : int8_t {
    Astro_MeasurementMode_Imperial,                          // Imperial measurement mode
    Astro_MeasurementMode_Metric,                            // Metric measurement mode
    Astro_MeasurementMode_Scientific,                        // Scientific/SI-oriented measurement mode
    Astro_MeasurementMode_Count,                             // Placeholder
    Astro_MeasurementMode_Undefined = -1                    // Placeholder
};

// Drive Direction
// Normalized direction used by actuators and mount movement.
enum Astro_DirectionMode : int8_t {
    Astro_DirectionMode_Reverse = -1,                        // Reverse/negative drive direction
    Astro_DirectionMode_Stop = 0,                            // Stop/no drive
    Astro_DirectionMode_Forward = 1,                         // Forward/positive drive direction
    Astro_DirectionMode_Undefined = -2                      // Placeholder
};

// Actuator Enable Mode
// Specifies how simultaneous activation handles are combined.
enum Astro_EnableMode : int8_t {
    Astro_EnableMode_Highest,                                // Highest active request wins
    Astro_EnableMode_Lowest,                                 // Lowest active request wins
    Astro_EnableMode_Average,                                // Average active requests together
    Astro_EnableMode_Multiply,                               // Multiply active request intensities
    Astro_EnableMode_InOrder,                                // Process requests in insertion order
    Astro_EnableMode_RevOrder,                               // Process requests in reverse insertion order
    Astro_EnableMode_Count,                                  // Placeholder
    Astro_EnableMode_Undefined = -1                         // Placeholder
};

// Actuator Type
// Identifies the general purpose of a controllable output.
enum Astro_ActuatorType : int8_t {
    Astro_ActuatorType_MountAxis,                            // Mount axis
    Astro_ActuatorType_Cover,                                // Cover
    Astro_ActuatorType_DewHeater,                            // Dew heater
    Astro_ActuatorType_CameraCooler,                         // Camera cooler
    Astro_ActuatorType_Fan,                                  // Fan
    Astro_ActuatorType_Focuser,                              // Focuser
    Astro_ActuatorType_Generic,                              // Generic
    Astro_ActuatorType_Count,                                // Placeholder / count
    Astro_ActuatorType_Undefined = -1                         // Placeholder
};

// Sensor Type
// Identifies the general purpose of a measured input.
enum Astro_SensorType : int8_t {
    Astro_SensorType_Temperature,                            // Temperature
    Astro_SensorType_Humidity,                               // Humidity
    Astro_SensorType_Position,                               // Position
    Astro_SensorType_LimitSwitch,                            // Limit switch
    Astro_SensorType_Rain,                                   // Rain
    Astro_SensorType_WindSpeed,                              // Wind speed
    Astro_SensorType_Light,                                  // Light
    Astro_SensorType_Voltage,                                // Voltage
    Astro_SensorType_Current,                                // Current
    Astro_SensorType_CameraTemperature,                      // Camera temperature
    Astro_SensorType_Generic,                                // Generic
    Astro_SensorType_Count,                                  // Placeholder / count
    Astro_SensorType_Undefined = -1                           // Placeholder
};

// Power Rail Type
// Common low-voltage rails used by DIY astronomical equipment.
enum Astro_RailType : int8_t {
    Astro_RailType_DC3V3,                                    // 3.3V DC rail
    Astro_RailType_DC5V,                                     // 5V DC rail
    Astro_RailType_DC12V,                                    // 12V DC rail
    Astro_RailType_DC24V,                                    // 24V DC rail
    Astro_RailType_Count,                                    // Placeholder
    Astro_RailType_Undefined = -1                           // Placeholder
};

// RTC Device Type
// Supported real-time clocks, matching the sibling controller libraries.
enum Astro_RTCType : int8_t {
    Astro_RTCType_DS1307 = 13,                               // DS1307 (no battFail)
    Astro_RTCType_DS3231 = 32,                               // DS3231
    Astro_RTCType_PCF8523 = 85,                              // PCF8523
    Astro_RTCType_PCF8563 = 86,                              // PCF8563
    Astro_RTCType_None = -1                                  // No RTC
};

// Pin Mode
// Portable pin direction/mode settings used by serialized pin data.
enum Astro_PinMode : int8_t {
    Astro_PinMode_Digital_Input,                             // Digital input
    Astro_PinMode_Digital_Input_PullUp,                      // Digital input with pull-up
    Astro_PinMode_Digital_Input_PullDown,                    // Digital input with pull-down
    Astro_PinMode_Digital_Output,                            // Digital output
    Astro_PinMode_Digital_Output_PushPull,                   // Push-pull digital output
    Astro_PinMode_Analog_Input,                              // Analog input
    Astro_PinMode_Analog_Output,                             // Analog/PWM output
    Astro_PinMode_Count,                                     // Placeholder
    Astro_PinMode_Undefined = -1                            // Placeholder
};

// Measurement Units Category
// Groups compatible units for conversion and default selection.
enum Astro_UnitsCategory : int8_t {
    Astro_UnitsCategory_Raw,                                 // Raw
    Astro_UnitsCategory_Angle,                               // Angle
    Astro_UnitsCategory_Distance,                            // Distance
    Astro_UnitsCategory_Percentile,                          // Percentile
    Astro_UnitsCategory_Speed,                               // Speed
    Astro_UnitsCategory_Temperature,                         // Temperature
    Astro_UnitsCategory_Humidity,                            // Humidity
    Astro_UnitsCategory_Power,                               // Power
    Astro_UnitsCategory_Voltage,                             // Voltage
    Astro_UnitsCategory_Current,                             // Current
    Astro_UnitsCategory_Count,                               // Placeholder / count
    Astro_UnitsCategory_Undefined = -1                        // Placeholder
};

// Measurement Units Type
// Supported units used by Astruino measurements and reporting.
enum Astro_UnitsType : int8_t {
    Astro_UnitsType_Raw_1,                                   // Raw/dimensionless value
    Astro_UnitsType_Angle_Degrees_360,                       // Degrees
    Astro_UnitsType_Angle_Radians_2pi,                       // Radians
    Astro_UnitsType_Distance_Meters,                         // Meters
    Astro_UnitsType_Distance_Feet,                           // Feet
    Astro_UnitsType_Percentile_100,                          // Percent [0,100]
    Astro_UnitsType_Speed_MetersPerSec,                      // Meters per sec
    Astro_UnitsType_Speed_FeetPerSec,                        // Feet per sec
    Astro_UnitsType_Temperature_Celsius,                     // Celsius
    Astro_UnitsType_Temperature_Fahrenheit,                  // Fahrenheit
    Astro_UnitsType_Temperature_Kelvin,                      // Kelvin
    Astro_UnitsType_Humidity_RH,                             // Relative humidity percent
    Astro_UnitsType_Power_Wattage,                           // Wattage
    Astro_UnitsType_Voltage_Volts,                           // Volts
    Astro_UnitsType_Current_Amperage,                        // Amperage
    Astro_UnitsType_Count,                                   // Placeholder
    Astro_UnitsType_Undefined = -1                          // Placeholder
};

// Built-in Astronomy Target
// Identifies built-in solar-system, Messier, alignment-star, and custom target slots.
enum Astro_TargetId : uint16_t {
    Astro_Target_Sun,                                        // Sun
    Astro_Target_Moon,                                       // Moon
    Astro_Target_Mercury,                                    // Mercury
    Astro_Target_Venus,                                      // Venus
    Astro_Target_Mars,                                       // Mars
    Astro_Target_Jupiter,                                    // Jupiter
    Astro_Target_Saturn,                                     // Saturn
    Astro_Target_Uranus,                                     // Uranus
    Astro_Target_Neptune,                                    // Neptune
    Astro_Target_M1,                                         // M1
    Astro_Target_M2,                                         // M2
    Astro_Target_M3,                                         // M3
    Astro_Target_M4,                                         // M4
    Astro_Target_M5,                                         // M5
    Astro_Target_M6,                                         // M6
    Astro_Target_M7,                                         // M7
    Astro_Target_M8,                                         // M8
    Astro_Target_M9,                                         // M9
    Astro_Target_M10,                                        // M10
    Astro_Target_M11,                                        // M11
    Astro_Target_M12,                                        // M12
    Astro_Target_M13,                                        // M13
    Astro_Target_M14,                                        // M14
    Astro_Target_M15,                                        // M15
    Astro_Target_M16,                                        // M16
    Astro_Target_M17,                                        // M17
    Astro_Target_M18,                                        // M18
    Astro_Target_M19,                                        // M19
    Astro_Target_M20,                                        // M20
    Astro_Target_M21,                                        // M21
    Astro_Target_M22,                                        // M22
    Astro_Target_M23,                                        // M23
    Astro_Target_M24,                                        // M24
    Astro_Target_M25,                                        // M25
    Astro_Target_M26,                                        // M26
    Astro_Target_M27,                                        // M27
    Astro_Target_M28,                                        // M28
    Astro_Target_M29,                                        // M29
    Astro_Target_M30,                                        // M30
    Astro_Target_M31,                                        // M31
    Astro_Target_M32,                                        // M32
    Astro_Target_M33,                                        // M33
    Astro_Target_M34,                                        // M34
    Astro_Target_M35,                                        // M35
    Astro_Target_M36,                                        // M36
    Astro_Target_M37,                                        // M37
    Astro_Target_M38,                                        // M38
    Astro_Target_M39,                                        // M39
    Astro_Target_M40,                                        // M40
    Astro_Target_M41,                                        // M41
    Astro_Target_M42,                                        // M42
    Astro_Target_M43,                                        // M43
    Astro_Target_M44,                                        // M44
    Astro_Target_M45,                                        // M45
    Astro_Target_M46,                                        // M46
    Astro_Target_M47,                                        // M47
    Astro_Target_M48,                                        // M48
    Astro_Target_M49,                                        // M49
    Astro_Target_M50,                                        // M50
    Astro_Target_M51,                                        // M51
    Astro_Target_M52,                                        // M52
    Astro_Target_M53,                                        // M53
    Astro_Target_M54,                                        // M54
    Astro_Target_M55,                                        // M55
    Astro_Target_M56,                                        // M56
    Astro_Target_M57,                                        // M57
    Astro_Target_M58,                                        // M58
    Astro_Target_M59,                                        // M59
    Astro_Target_M60,                                        // M60
    Astro_Target_M61,                                        // M61
    Astro_Target_M62,                                        // M62
    Astro_Target_M63,                                        // M63
    Astro_Target_M64,                                        // M64
    Astro_Target_M65,                                        // M65
    Astro_Target_M66,                                        // M66
    Astro_Target_M67,                                        // M67
    Astro_Target_M68,                                        // M68
    Astro_Target_M69,                                        // M69
    Astro_Target_M70,                                        // M70
    Astro_Target_M71,                                        // M71
    Astro_Target_M72,                                        // M72
    Astro_Target_M73,                                        // M73
    Astro_Target_M74,                                        // M74
    Astro_Target_M75,                                        // M75
    Astro_Target_M76,                                        // M76
    Astro_Target_M77,                                        // M77
    Astro_Target_M78,                                        // M78
    Astro_Target_M79,                                        // M79
    Astro_Target_M80,                                        // M80
    Astro_Target_M81,                                        // M81
    Astro_Target_M82,                                        // M82
    Astro_Target_M83,                                        // M83
    Astro_Target_M84,                                        // M84
    Astro_Target_M85,                                        // M85
    Astro_Target_M86,                                        // M86
    Astro_Target_M87,                                        // M87
    Astro_Target_M88,                                        // M88
    Astro_Target_M89,                                        // M89
    Astro_Target_M90,                                        // M90
    Astro_Target_M91,                                        // M91
    Astro_Target_M92,                                        // M92
    Astro_Target_M93,                                        // M93
    Astro_Target_M94,                                        // M94
    Astro_Target_M95,                                        // M95
    Astro_Target_M96,                                        // M96
    Astro_Target_M97,                                        // M97
    Astro_Target_M98,                                        // M98
    Astro_Target_M99,                                        // M99
    Astro_Target_M100,                                       // M100
    Astro_Target_M101,                                       // M101
    Astro_Target_M102,                                       // M102
    Astro_Target_M103,                                       // M103
    Astro_Target_M104,                                       // M104
    Astro_Target_M105,                                       // M105
    Astro_Target_M106,                                       // M106
    Astro_Target_M107,                                       // M107
    Astro_Target_M108,                                       // M108
    Astro_Target_M109,                                       // M109
    Astro_Target_M110,                                       // M110
    Astro_Target_Sirius,                                     // Sirius
    Astro_Target_Canopus,                                    // Canopus
    Astro_Target_Arcturus,                                   // Arcturus
    Astro_Target_Vega,                                       // Vega
    Astro_Target_Capella,                                    // Capella
    Astro_Target_RigelKentaurus,                             // Rigel kentaurus
    Astro_Target_Procyon,                                    // Procyon
    Astro_Target_Betelgeuse,                                 // Betelgeuse
    Astro_Target_Achernar,                                   // Achernar
    Astro_Target_Hadar,                                      // Hadar
    Astro_Target_Altair,                                     // Altair
    Astro_Target_Acrux,                                      // Acrux
    Astro_Target_Aldebaran,                                  // Aldebaran
    Astro_Target_Spica,                                      // Spica
    Astro_Target_Antares,                                    // Antares
    Astro_Target_Pollux,                                     // Pollux
    Astro_Target_Fomalhaut,                                  // Fomalhaut
    Astro_Target_Deneb,                                      // Deneb
    Astro_Target_Regulus,                                    // Regulus
    Astro_Target_Polaris,                                    // Polaris
    Astro_Target_Castor,                                     // Castor
    Astro_Target_Bellatrix,                                  // Bellatrix
    Astro_Target_Alnilam,                                    // Alnilam
    Astro_Target_Alnitak,                                    // Alnitak
    Astro_Target_Mizar,                                      // Mizar
    Astro_Target_Dubhe,                                      // Dubhe
    Astro_Target_Custom1,                                    // Custom1
    Astro_Target_Custom2,                                    // Custom2
    Astro_Target_Custom3,                                    // Custom3
    Astro_Target_Custom4,                                    // Custom4
    Astro_Target_Custom5,                                    // Custom5
    Astro_Target_Custom6,                                    // Custom6
    Astro_Target_Custom7,                                    // Custom7
    Astro_Target_Custom8,                                    // Custom8
    Astro_Target_Count,                                      // Placeholder / count
    Astro_Target_Undefined = 0xffff                       // Placeholder
};

// Astronomy Target Class
// Broad object classification used for catalog metadata and display.
enum Astro_TargetClass : int8_t {
    Astro_TargetClass_Star,                                  // Star
    Astro_TargetClass_OpenCluster,                           // Open cluster
    Astro_TargetClass_GlobularCluster,                       // Globular cluster
    Astro_TargetClass_Nebula,                                // Nebula
    Astro_TargetClass_PlanetaryNebula,                       // Planetary nebula
    Astro_TargetClass_Galaxy,                                // Galaxy
    Astro_TargetClass_SolarSystem,                           // Solar system
    Astro_TargetClass_Other,                                 // Other
    Astro_TargetClass_Count,                                 // Placeholder
    Astro_TargetClass_Unknown = -1                          // Placeholder
};

// Telescope Mount Type
// Geometry used to convert target coordinates into controlled mount axes.
enum Astro_MountType : int8_t {
    Astro_MountType_Equatorial,                              // Equatorial
    Astro_MountType_AltAz,                                   // Altitude/azimuth mount
    Astro_MountType_SingleAxis,                              // Single-axis sidereal tracker
    Astro_MountType_Count,                                   // Placeholder
    Astro_MountType_Unknown = -1                              // Placeholder
};

// Thermal Control Mode
// Selects thermal-balancing goals for storage, observing, or safe stow.
enum Astro_ThermalMode : int8_t {
    Astro_ThermalMode_DayStorage,                            // Day storage
    Astro_ThermalMode_NightObserving,                        // Night observing
    Astro_ThermalMode_SafeStowed,                            // Safe stowed
    Astro_ThermalMode_Count,                                 // Placeholder
    Astro_ThermalMode_Undefined = -1                          // Placeholder
};

// Scheduler Stage
// Current stage of the automatic nighttime observation sequence.
enum Astro_SchedulerStage : int8_t {
    Astro_SchedulerStage_DayStowed,                          // Day stowed
    Astro_SchedulerStage_Deploying,                          // Deploying
    Astro_SchedulerStage_Cooling,                            // Cooling
    Astro_SchedulerStage_Slewing,                            // Slewing
    Astro_SchedulerStage_Settling,                           // Settling
    Astro_SchedulerStage_Observing,                          // Observing
    Astro_SchedulerStage_Warming,                            // Warming
    Astro_SchedulerStage_Stowing,                            // Stowing
    Astro_SchedulerStage_SafeStowed,                         // Safe stowed
    Astro_SchedulerStage_Fault,                              // Hardware or motion fault requiring intervention
    Astro_SchedulerStage_Count,                              // Placeholder
    Astro_SchedulerStage_Undefined = -1                       // Placeholder
};

// Logging Level
// Filters event output generated by the system logger.
enum Astro_LogLevel : int8_t {
    Astro_LogLevel_All,                                      // All messages
    Astro_LogLevel_Warnings,                                 // Warnings and errors
    Astro_LogLevel_Errors,                                   // Errors only
    Astro_LogLevel_Count,                                    // Placeholder
    Astro_LogLevel_None = -1,                                // Logging disabled
    Astro_LogLevel_Info = Astro_LogLevel_All                    // Informational level alias
};

#endif // /ifndef AstroDefines_H
