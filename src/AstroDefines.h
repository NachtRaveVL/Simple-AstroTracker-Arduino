/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Defines
*/

#ifndef AstroDefines_H
#define AstroDefines_H

#ifndef JOIN
#define JOIN_(X,Y) X##_##Y
#define JOIN(X,Y) JOIN_(X,Y)
#endif
#ifndef JOIN3
#define JOIN3_(X,Y,Z) X##_##Y##_##Z
#define JOIN3(X,Y,Z) JOIN3_(X,Y,Z)
#endif
#ifndef STR
#define STR_(X) #X
#define STR(X) STR_(X)
#endif

#define ACT_HIGH                        false               // Active high (convenience)
#define ACT_ABOVE                       false               // Active above (convenience)
#define ACT_LOW                         true                // Active low (convenience)
#define ACT_BELOW                       true                // Active below (convenience)
#define PULL_DOWN                       false               // Pull down (convenience)
#define PULL_UP                         true                // Pull up (convenience)
#define RAW                             false               // Raw mode (convenience)
#define JSON                            true                // JSON mode (convenience)

#ifndef FLT_EPSILON
#define FLT_EPSILON                     0.00001f            // Single-precision floating point error tolerance
#endif
#ifndef DBL_EPSILON
#define DBL_EPSILON                     0.0000000000001     // Double-precision floating point error tolerance
#endif
#ifndef FLT_UNDEF
#define FLT_UNDEF                       __FLT_MAX__         // Single-precision floating point value to stand in for "undefined"
#endif
#ifndef DBL_UNDEF
#define DBL_UNDEF                       __DBL_MAX__         // Double-precision floating point value to stand in for "undefined"
#endif
#ifndef MIN_PER_DAY
#define MIN_PER_DAY                     1440                // Minutes per day
#endif
#ifndef TWO_PI
#define TWO_PI                          6.283185307179586476925286766559 // Two pi
#endif

// Platform standardizations
#if (defined(ESP32) || defined(ESP8266)) && !defined(ESP_PLATFORM) // ESP_PLATFORM for any esp
#define ESP_PLATFORM
#endif
#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_STM32)    // Missing min/max
#define min(a,b)                        ((a)<(b)?(a):(b))
#define max(a,b)                        ((a)>(b)?(a):(b))
#endif
#if defined(CORE_TEENSY)                                    // Missing abs
#define abs(x)                          ((x)>0?(x):-(x))
#endif
#ifndef RANDOM_MAX                                          // Missing RANDOM_MAX
#if defined(RAND_MAX)
#define RANDOM_MAX                      RAND_MAX
#else
#define RANDOM_MAX                      __INT_MAX__
#endif
#endif

// MCU capabilities
#ifndef ADC_RESOLUTION                                      // Resolving ADC resolution, or define manually by build define (see BOARD for example)
#if defined(IOA_ANALOGIN_RES)                               // From IOAbstraction
#define ADC_RESOLUTION                  IOA_ANALOGIN_RES
#else
#define ADC_RESOLUTION                  10                  // Default per AVR
#endif
#endif
#ifndef DAC_RESOLUTION                                      // Resolving DAC resolution, or define manually by build define (see BOARD for example)
#if defined(IOA_ANALOGOUT_RES)                              // From IOAbstraction
#define DAC_RESOLUTION                  IOA_ANALOGOUT_RES
#else
#define DAC_RESOLUTION                  8                   // Default per AVR
#endif
#endif
#ifndef F_SPD                                               // Resolving F_SPD=F_CPU|F_BUS alias (for default SPI device speeds), or define manually by build define (see BOARD for example)
#if defined(F_CPU)
#define F_SPD                           F_CPU
#elif defined(F_BUS)                                        // Teensy/etc support
#define F_SPD                           F_BUS
#else                                                       // Fast/good enough (32MHz)
#define F_SPD                           32000000U
#endif
#endif
#ifndef V_MCU                                               // Resolving MCU voltage, or define manually by build define (see BOARD for example)
#if (defined(__AVR__) || defined(__STM8__)) && !defined(ARDUINO_AVR_FIO) && !(defined(ARDUINO_AVR_PRO) && F_CPU == 8000000L)
#define V_MCU                           5.0                 // 5v MCU (assumed/tolerant)
#else
#define V_MCU                           3.3                 // 3v3 MCU (assumed)
#endif
#endif
#ifndef BOARD                                               // Resolving board name alias, or define manually by build define via creating platform.local.txt in %applocaldata%\Arduino15\packages\{platform}\hardware\{arch}\{version}\ containing (/w quotes): compiler.cpp.extra_flags="-DBOARD={build.board}"
#if defined(CORE_TEENSY)                                    // For Teensy, define manually by build define via editing platform.txt directly in %applocaldata%\Arduino15\packages\teensy\hardware\avr\{version}\ and adding (/w space & quotes):  "-DBOARD={build.board}" to end of recipe.cpp.o.pattern=
#define BOARD                           "TEENSY"
#elif defined(ARDUINO_BOARD)
#define BOARD                           ARDUINO_BOARD
#elif defined(BOARD_NAME)
#define BOARD                           BOARD_NAME
#elif defined(USB_PRODUCT)
#define BOARD                           USB_PRODUCT
#else
#define BOARD                           "OTHER"
#endif
#endif
#ifndef HAS_INPUT_PULLDOWN                                  // Resolve for INPUT_PULLDOWN availability
#if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ARM) || defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040) || defined(ESP32) || defined(ARDUINO_ARCH_STM32) || defined(CORE_TEENSY)
#define HAS_INPUT_PULLDOWN              true                // INPUT_PULLDOWN likely available (INPUT able to be PULLDOWN/floating/PULLUP)
#else
#define HAS_INPUT_PULLDOWN              false               // INPUT_PULLDOWN likely unavailable (INPUT restricted to just floating/PULLUP)
#endif
#endif
#ifndef HAS_LARGE_SRAM                                      // Resolve for large SRAM size availability
#if defined(ARDUINO_SAM_DUE) || defined(ARDUINO_ARCH_ARM) || defined(ARDUINO_ARCH_MBED)  || defined(ARDUINO_ARCH_RP2040) || defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_STM32) || defined(CORE_TEENSY)
#define HAS_LARGE_SRAM                  true                // Large SRAM size likely available (memory saving features disabled)
#else
#define HAS_LARGE_SRAM                  false               // Large SRAM size likely unavailable (memory saving features enabled)
#endif
#endif

// Standardized gfx WxH
#ifdef ASTRO_USE_GUI
#include <User_Setup.h>
#endif
#ifndef TFT_GFX_WIDTH
#ifdef TFT_WIDTH
#define TFT_GFX_WIDTH                   TFT_WIDTH           // Custom defined
#else
#define TFT_GFX_WIDTH                   240                 // Default 240x
#endif
#endif
#ifndef TFT_GFX_HEIGHT
#ifdef TFT_HEIGHT
#define TFT_GFX_HEIGHT                  TFT_HEIGHT          // Custom defined
#else
#define TFT_GFX_HEIGHT                  320                 // Default x320
#endif
#endif

typedef typeof(millis())                millis_t;           // Millisecond time type
typedef int8_t                          aposi_t;            // Position indexing type alias
typedef uint32_t                        akey_t;             // Key type alias, for hashing
typedef int8_t                          aid_t;              // Id type alias, for RTTI
typedef uint16_t                        aframe_t;           // Polling frame type, for sync
#define millis_none                     ((millis_t)0)       // No millis defined/none placeholder
#define aposi_none                      ((aposi_t)-1)       // No position defined/none placeholder
#define akey_none                       ((akey_t)-1)        // No key defined/none placeholder
#define aid_none                        ((aid_t)-1)         // No id defined/none placeholder
#define aframe_none                     ((aframe_t)0)       // No frame defined/none placeholder
#define apin_none                       ((pintype_t)-1)     // No pin defined/none placeholder
#define apin_virtual                    ((pintype_t)100)    // Virtual pin section start placeholder
#define apinchnl_none                   ((int8_t)-127)      // No pin channel defined/none placeholder
typedef typeof(INPUT)                   ard_pinmode_t;      // Arduino pin mode type alias
typedef typeof(LOW)                     ard_pinstatus_t;    // Arduino pin status type alias

// The following sizes apply to all architectures
#define ASTRO_PREFIX_MAXSIZE            16                  // Prefix names maximum size (for logging/publishing)
#define ASTRO_NAME_MAXSIZE              24                  // Naming character maximum size (system name, target name, etc.)
#define ASTRO_TARGET_NAME_MAXSIZE       32                  // Target display name maximum size
#define ASTRO_TARGET_ID_MAXSIZE         16                  // Target catalog/object identifier maximum size
#define ASTRO_POS_MAXSIZE               32                  // Position indicies maximum size (max # of objs of same type)
#define ASTRO_URL_MAXSIZE               64                  // URL string maximum size (max url length)
#define ASTRO_JSON_DOC_SYSSIZE          256                 // JSON document chunk data bytes for reading in main system data (serialization buffer size)
#define ASTRO_JSON_DOC_DEFSIZE          192                 // Default JSON document chunk data bytes (serialization buffer size)
#define ASTRO_STRING_BUFFER_SIZE        32                  // Size in bytes of string serialization buffers
#define ASTRO_WIFISTREAM_BUFFER_SIZE    128                 // Size in bytes of WiFi serialization buffers
#define ASTRO_ACTIVATION_HANDLE_SLOTS   8                   // Maximum simultaneous activation requests per actuator
// The following sizes only apply to architectures that do not have STL support (AVR/SAM)
#define ASTRO_DEFAULT_MAXSIZE           8                   // Default maximum array/map size
#define ASTRO_ACTUATOR_SIGNAL_SLOTS     4                   // Maximum number of slots for actuator's activation signal
#define ASTRO_SENSOR_SIGNAL_SLOTS       6                   // Maximum number of slots for sensor's measurement signal
#define ASTRO_TRIGGER_SIGNAL_SLOTS      4                   // Maximum number of slots for trigger's state signal
#define ASTRO_LOG_SIGNAL_SLOTS          2                   // Maximum number of slots for system log signal
#define ASTRO_PUBLISH_SIGNAL_SLOTS      2                   // Maximum number of slots for data publish signal
#define ASTRO_RAIL_SIGNAL_SLOTS         8                   // Maximum number of slots for rail capacity signal
#define ASTRO_SYS_OBJECTS_MAXSIZE       16                  // Maximum array size for system objects (max # of objects in system)
#define ASTRO_TARGETS_TARGETSLIB_MAXSIZE 8                  // Maximum array size for targets library objects (max # of different kinds of targets)
#define ASTRO_CAL_CALIBS_MAXSIZE        8                   // Maximum array size for calibration store objects (max # of different custom calibrations)
#define ASTRO_OBJ_LINKS_MAXSIZE         8                   // Maximum array size for object linkage list, per obj (max # of linked objects)
#define ASTRO_SYS_ONEWIRES_MAXSIZE      2                   // Maximum array size for pin OneWire list (max # of OneWire comm pins)
#define ASTRO_SYS_PINLOCKS_MAXSIZE      2                   // Maximum array size for pin locks list (max # of locks)
#define ASTRO_SYS_PINMUXERS_MAXSIZE     2                   // Maximum array size for pin muxers list (max # of muxers)
#define ASTRO_SYS_PINEXPANDERS_MAXSIZE  2                   // Maximum array size for pin expanders list (max # of expanders)


#define ASTRO_CONTROL_LOOP_INTERVAL     100                 // Run interval of main control loop, in milliseconds
#define ASTRO_DATA_LOOP_INTERVAL        2000                // Default run interval of data loop, in milliseconds (customizable later)
#define ASTRO_MISC_LOOP_INTERVAL        250                 // Run interval of misc loop, in milliseconds

#define ASTRO_TARGETS_LINKS_BASESIZE    1                   // Base array size for target's linkage list

#define ASTRO_MOUNT_AXIS_RATE_DEGPS     8.0                 // Default simulated/limited mount axis rate, in degrees per second
#define ASTRO_MOUNT_SIDEREAL_RATE_DEGPS 0.0041780746        // Sidereal tracking rate, in degrees per second
#define ASTRO_MOUNT_GUIDE_RATE          0.5                 // Default pulse-guide rate as a multiple of sidereal

#define ASTRO_COVER_TRAVEL_RATE         0.2f                // Default normalized cover travel rate per second
#define ASTRO_COVER_TRAVEL_TIMEOUT_SECS 120.0               // Default cover movement timeout, in seconds

#define ASTRO_SCH_DEPLOY_SUN_ALT_DEG    -6.0                // Sun altitude below which nighttime deployment may begin
#define ASTRO_SCH_STOW_SUN_ALT_DEG      -3.0                // Sun altitude above which morning stow begins
#define ASTRO_SCH_ALIGN_TOL_DEG         0.35                // Default mount alignment tolerance, in degrees
#define ASTRO_SCH_SETTLE_SECS           5                   // Default stable alignment time before observation
#define ASTRO_SCH_REPORT_INTERVAL_SECS  (8UL * 60UL * 60UL) // Default environment report interval, in seconds
#define ASTRO_SCH_CAMERA_STABLE_DEG     2.0                 // Allowed camera temperature error before observing, in degrees C

#define ASTRO_THERMAL_DEW_MARGIN_C      3.0                 // Default optics margin above dew point, in degrees C
#define ASTRO_THERMAL_OPTICS_MAX_C      4.0                 // Maximum optics target above ambient, in degrees C
#define ASTRO_THERMAL_CAMERA_TARGET_C   -10.0               // Default requested cooled-camera sensor target, in degrees C
#define ASTRO_THERMAL_CAMERA_RAMP_CPM   2.0                 // Default camera cooling/warming ramp, in degrees C per minute
#define ASTRO_THERMAL_ELECTRONICS_MIN_C -5.0                // Default minimum electronics/body temperature, in degrees C

#define ASTRO_MUXERS_SHARED_ADDR_BUS    false               // Pin muxer channel selects should disable all pin muxers due to using same address bus (true), or not (false)

#define ASTRO_NIGHT_START_HR            20                  // Hour of the day night starts (used if not able to calculate from location & time)
#define ASTRO_NIGHT_FINISH_HR           6                   // Hour of the day night finishes (used if not able to calculate from location & time)

#define ASTRO_POS_SEARCH_FROMBEG        -1                  // Search from beginning to end, 0 up to MAXSIZE-1
#define ASTRO_POS_SEARCH_FROMEND        ASTRO_POS_MAXSIZE   // Search from end to beginning, MAXSIZE-1 down to 0
#define ASTRO_POS_EXPORT_BEGFROM        1                   // Whenever exported/user-facing position indexing starts at 1 or 0 (aka display offset)

#define ASTRO_RAILS_LINKS_BASESIZE      4                   // Base array size for rail's linkage list
#define ASTRO_RAILS_FRACTION_SATURATED  0.8f                // What fraction of maximum power is allowed to be used in canActivate() checks (aka maximum saturation point), used in addition to regulated rail's limitTrigger

#define ASTRO_SENSOR_BINARY_STABLE_MILLIS 100               // Minimum time a binary sensor input must remain changed before the new state is accepted, in milliseconds
#define ASTRO_SENSOR_ANALOGREAD_SAMPLES 5                   // Number of samples to take for any analogRead call inside of a sensor's takeMeasurement call, or 0 to disable sampling (note: bitRes.maxValue * # of samples must fit inside a uint32_t)
#define ASTRO_SENSOR_ANALOGREAD_DELAY   0                   // Delay time between samples, or 0 to disable delay, in milliseconds

#define ASTRO_SYS_AUTOSAVE_INTERVAL     120                 // Default autosave interval, in minutes
#define ASTRO_SYS_I2CEEPROM_BASEADDR    0x50                // Base address of I2C EEPROM (bitwise or'ed with passed address)
#define ASTRO_SYS_ATWIFI_SERIALBAUD     115200              // Data baud rate for serial AT WiFi, in bps (older modules may need 9600)
#define ASTRO_SYS_ATWIFI_SERIALMODE     SERIAL_8N1          // Data transfer mode for serial AT WiFi (see SERIAL_* defines)
#define ASTRO_SYS_NMEAGPS_SERIALBAUD    9600                // Data baud rate for serial NMEA GPS, in bps (older modules may need 4800)
#define ASTRO_SYS_URLHTTP_PORT          80                  // Which default port to access when accessing HTTP resources
#define ASTRO_SYS_LEAVE_FILES_OPEN      !defined(__AVR__)   // If high access files should be left open to improve performance (true), or closed after use to reduce memory consumption (false)
#define ASTRO_SYS_FREERAM_LOWBYTES      1024                // How many bytes of free memory left spawns a handle low mem call to all objects
#define ASTRO_SYS_FREESPACE_INTERVAL    240                 // How many minutes should pass before checking attached file systems have enough disk space (performs cleanup if not)
#define ASTRO_SYS_FREESPACE_LOWSPACE    256                 // How many kilobytes of disk space remaining will force cleanup of oldest log/data files first
#define ASTRO_SYS_FREESPACE_DAYSBACK    180                 // How many days back log/data files are allowed to be stored up to (any beyond this are deleted during cleanup)
#define ASTRO_SYS_SUNRISESET_CALCITERS  3                   // # of iterations that sunrise/sunset calculations should run (higher # = more accurate but also more costly)
#define ASTRO_SYS_LATLONG_DISTSQRDTOL   0.25                // Squared difference in lat/long coords that needs to occur for it to be considered significant enough for system update
#define ASTRO_SYS_ALTITUDE_DISTTOL      0.5                 // Difference in altitude coords that needs to occur for it to be considered significant enough for system update
#define ASTRO_SYS_DELAYFINE_SPINMILLIS  20                  // How many milliseconds away from stop time fine delays can use yield() up to before using a blocking spin-lock (used for fine timing)
#define ASTRO_SYS_YIELD_AFTERMILLIS     20                  // How many milliseconds must pass by before system run loops call a yield() mid-loop, in order to allow finely-timed tasks a chance to run
#define ASTRO_SYS_DEBUGOUT_FLUSH_YIELD  false               // If debug output statements should flush and yield afterwards to force send through to serial monitor (mainly used for debugging)
#define ASTRO_SYS_MEM_LOGGING_ENABLE    false               // If system will periodically log memory remaining messages (mainly used for debugging)
#define ASTRO_SYS_DRY_RUN_ENABLE        false               // Disables pins from actually enabling in order to simply simulate (mainly used for debugging)

#if defined(__APPLE__) || defined(__APPLE) || defined(__unix__) || defined(__unix)
#define ASTRO_BLDPATH_SEPARATOR         '/'                 // Path separator for nix-based build machines
#else
#define ASTRO_BLDPATH_SEPARATOR         '\\'                // Path separator for win-based build machines
#endif
#define ASTRO_FSPATH_SEPARATOR          '/'                 // Path separator for filesystem paths (SD card/WiFiStorage)
#define ASTRO_URLPATH_SEPARATOR         '/'                 // Path separator for URL paths

#if ASTRO_SYS_LEAVE_FILES_OPEN                              // How subsequent getters should be called when file left open
#define ASTRO_LOFS_BEGIN false
#else
#define ASTRO_LOFS_BEGIN true
#endif

#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SPI)) && (SPI_INTERFACES_COUNT > 0 || SPI_HOWMANY > 0)
#define ASTRO_USE_SPI                   &SPI
#else
#define ASTRO_USE_SPI                   nullptr
#endif
#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SPI1)) && (SPI_INTERFACES_COUNT > 1 || SPI_HOWMANY > 1)
#define ASTRO_USE_SPI1                  &SPI1
#else
#define ASTRO_USE_SPI1                  nullptr
#endif
#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_TWOWIRE)) && (WIRE_INTERFACES_COUNT > 0 || WIRE_HOWMANY > 0)
#define ASTRO_USE_WIRE                  &Wire
#else
#define ASTRO_USE_WIRE                  nullptr
#endif
#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_TWOWIRE1)) && (WIRE_INTERFACES_COUNT > 1 || WIRE_HOWMANY > 1)
#define ASTRO_USE_WIRE1                 &Wire1
#else
#define ASTRO_USE_WIRE1                 nullptr
#endif
#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SERIAL1)) && (SERIAL_HOWMANY > 1 || defined(HWSERIAL1) || defined(HAVE_HWSERIAL1) || defined(PIN_SERIAL1_RX) || defined(SERIAL2_RX) || defined(Serial1))
#define ASTRO_USE_SERIAL1               &Serial1
#else
#define ASTRO_USE_SERIAL1               nullptr
#endif


// EEPROM Device Type Enumeration
enum Astro_EEPROMType : signed short {
    Astro_EEPROMType_AT24LC01 = I2C_DEVICESIZE_24LC01 >> 7,     // AT24LC01 (1k bits, 128 bytes), 7-bit address space
    Astro_EEPROMType_AT24LC02 = I2C_DEVICESIZE_24LC02 >> 7,     // AT24LC02 (2k bits, 256 bytes), 8-bit address space
    Astro_EEPROMType_AT24LC04 = I2C_DEVICESIZE_24LC04 >> 7,     // AT24LC04 (4k bits, 512 bytes), 9-bit address space
    Astro_EEPROMType_AT24LC08 = I2C_DEVICESIZE_24LC08 >> 7,     // AT24LC08 (8k bits, 1024 bytes), 10-bit address space
    Astro_EEPROMType_AT24LC16 = I2C_DEVICESIZE_24LC16 >> 7,     // AT24LC16 (16k bits, 2048 bytes), 11-bit address space
    Astro_EEPROMType_AT24LC32 = I2C_DEVICESIZE_24LC32 >> 7,     // AT24LC32 (32k bits, 4096 bytes), 12-bit address space
    Astro_EEPROMType_AT24LC64 = I2C_DEVICESIZE_24LC64 >> 7,     // AT24LC64 (64k bits, 8192 bytes), 13-bit address space
    Astro_EEPROMType_AT24LC128 = I2C_DEVICESIZE_24LC128 >> 7,   // AT24LC128 (128k bits, 16384 bytes), 14-bit address space
    Astro_EEPROMType_AT24LC256 = I2C_DEVICESIZE_24LC256 >> 7,   // AT24LC256 (256k bits, 32768 bytes), 15-bit address space
    Astro_EEPROMType_AT24LC512 = I2C_DEVICESIZE_24LC512 >> 7,   // AT24LC512 (512k bits, 65536 bytes), 16-bit address space
    Astro_EEPROMType_None = -1,                                 // No EEPROM

    Astro_EEPROMType_Bits_1k = Astro_EEPROMType_AT24LC01,       // 1k bits (alias of AT24LC01)
    Astro_EEPROMType_Bits_2k = Astro_EEPROMType_AT24LC02,       // 2k bits (alias of AT24LC02)
    Astro_EEPROMType_Bits_4k = Astro_EEPROMType_AT24LC04,       // 4k bits (alias of AT24LC04)
    Astro_EEPROMType_Bits_8k = Astro_EEPROMType_AT24LC08,       // 8k bits (alias of AT24LC08)
    Astro_EEPROMType_Bits_16k = Astro_EEPROMType_AT24LC16,      // 16k bits (alias of AT24LC16)
    Astro_EEPROMType_Bits_32k = Astro_EEPROMType_AT24LC32,      // 32k bits (alias of AT24LC32)
    Astro_EEPROMType_Bits_64k = Astro_EEPROMType_AT24LC64,      // 64k bits (alias of AT24LC64)
    Astro_EEPROMType_Bits_128k = Astro_EEPROMType_AT24LC128,    // 128k bits (alias of AT24LC128)
    Astro_EEPROMType_Bits_256k = Astro_EEPROMType_AT24LC256,    // 256k bits (alias of AT24LC256)
    Astro_EEPROMType_Bits_512k = Astro_EEPROMType_AT24LC512,    // 512k bits (alias of AT24LC512)

    Astro_EEPROMType_Bytes_128 = Astro_EEPROMType_AT24LC01,     // 128 bytes (alias of AT24LC01)
    Astro_EEPROMType_Bytes_256 = Astro_EEPROMType_AT24LC02,     // 256 bytes (alias of AT24LC02)
    Astro_EEPROMType_Bytes_512 = Astro_EEPROMType_AT24LC04,     // 512 bytes (alias of AT24LC04)
    Astro_EEPROMType_Bytes_1024 = Astro_EEPROMType_AT24LC08,    // 1024 bytes (alias of AT24LC08)
    Astro_EEPROMType_Bytes_2048 = Astro_EEPROMType_AT24LC16,    // 2048 bytes (alias of AT24LC16)
    Astro_EEPROMType_Bytes_4096 = Astro_EEPROMType_AT24LC32,    // 4096 bytes (alias of AT24LC32)
    Astro_EEPROMType_Bytes_8192 = Astro_EEPROMType_AT24LC64,    // 8192 bytes (alias of AT24LC64)
    Astro_EEPROMType_Bytes_16384 = Astro_EEPROMType_AT24LC128,  // 16384 bytes (alias of AT24LC128)
    Astro_EEPROMType_Bytes_32768 = Astro_EEPROMType_AT24LC256,  // 32768 bytes (alias of AT24LC256)
    Astro_EEPROMType_Bytes_65536 = Astro_EEPROMType_AT24LC512   // 65536 bytes (alias of AT24LC512)
};

// RTC Device Type Enumeration
enum Astro_RTCType : signed char {
    Astro_RTCType_DS1307 = 13,                              // DS1307 (no battFail)
    Astro_RTCType_DS3231 = 32,                              // DS3231
    Astro_RTCType_PCF8523 = 85,                             // PCF8523
    Astro_RTCType_PCF8563 = 86,                             // PCF8563
    Astro_RTCType_None = -1                                 // No RTC
};

// DHT Device Type Enumeration
enum Astro_DHTType : signed char {
    Astro_DHTType_DHT11 = DHT11,                            // DHT11 (using DHT library)
    Astro_DHTType_DHT12 = DHT12,                            // DHT12 (using DHT library)
    Astro_DHTType_DHT21 = DHT21,                            // DHT21 (using DHT library)
    Astro_DHTType_DHT22 = DHT22,                            // DHT22 (using DHT library)
    //Astro_DHTType_BME280 = 28,                              // BME280 (using BME280 library, TODO)
    Astro_DHTType_None = -1,                                // No DHT

    Astro_DHTType_AM2301 = AM2301                           // AM2301 (alias of DHT21)
};

// Target Type
// Common astronomical targets. Controls which catalog/ephemeris entry a target object uses.
enum Astro_TargetType : unsigned char {
    Astro_TargetType_Sun,                                   // Sun
    Astro_TargetType_Moon,                                  // Moon
    Astro_TargetType_Mercury,                               // Mercury
    Astro_TargetType_Venus,                                 // Venus
    Astro_TargetType_Mars,                                  // Mars
    Astro_TargetType_Jupiter,                               // Jupiter
    Astro_TargetType_Saturn,                                // Saturn
    Astro_TargetType_Uranus,                                // Uranus
    Astro_TargetType_Neptune,                               // Neptune
    Astro_TargetType_Sirius,                                // Sirius
    Astro_TargetType_Canopus,                               // Canopus
    Astro_TargetType_Arcturus,                              // Arcturus
    Astro_TargetType_Vega,                                  // Vega
    Astro_TargetType_Capella,                               // Capella
    Astro_TargetType_RigelKentaurus,                        // Rigel Kentaurus
    Astro_TargetType_Procyon,                               // Procyon
    Astro_TargetType_Betelgeuse,                            // Betelgeuse
    Astro_TargetType_Achernar,                              // Achernar
    Astro_TargetType_Hadar,                                 // Hadar
    Astro_TargetType_Altair,                                // Altair
    Astro_TargetType_Acrux,                                 // Acrux
    Astro_TargetType_Aldebaran,                             // Aldebaran
    Astro_TargetType_Spica,                                 // Spica
    Astro_TargetType_Antares,                               // Antares
    Astro_TargetType_Pollux,                                // Pollux
    Astro_TargetType_Fomalhaut,                             // Fomalhaut
    Astro_TargetType_Deneb,                                 // Deneb
    Astro_TargetType_Regulus,                               // Regulus
    Astro_TargetType_Polaris,                               // Polaris
    Astro_TargetType_Castor,                                // Castor
    Astro_TargetType_Bellatrix,                             // Bellatrix
    Astro_TargetType_Alnilam,                               // Alnilam
    Astro_TargetType_Alnitak,                               // Alnitak
    Astro_TargetType_Mizar,                                 // Mizar
    Astro_TargetType_Dubhe,                                 // Dubhe
    Astro_TargetType_M1,                                    // M1
    Astro_TargetType_M2,                                    // M2
    Astro_TargetType_M3,                                    // M3
    Astro_TargetType_M4,                                    // M4
    Astro_TargetType_M5,                                    // M5
    Astro_TargetType_M6,                                    // M6
    Astro_TargetType_M7,                                    // M7
    Astro_TargetType_M8,                                    // M8
    Astro_TargetType_M9,                                    // M9
    Astro_TargetType_M10,                                   // M10
    Astro_TargetType_M11,                                   // M11
    Astro_TargetType_M12,                                   // M12
    Astro_TargetType_M13,                                   // M13
    Astro_TargetType_M14,                                   // M14
    Astro_TargetType_M15,                                   // M15
    Astro_TargetType_M16,                                   // M16
    Astro_TargetType_M17,                                   // M17
    Astro_TargetType_M18,                                   // M18
    Astro_TargetType_M19,                                   // M19
    Astro_TargetType_M20,                                   // M20
    Astro_TargetType_M21,                                   // M21
    Astro_TargetType_M22,                                   // M22
    Astro_TargetType_M23,                                   // M23
    Astro_TargetType_M24,                                   // M24
    Astro_TargetType_M25,                                   // M25
    Astro_TargetType_M26,                                   // M26
    Astro_TargetType_M27,                                   // M27
    Astro_TargetType_M28,                                   // M28
    Astro_TargetType_M29,                                   // M29
    Astro_TargetType_M30,                                   // M30
    Astro_TargetType_M31,                                   // M31
    Astro_TargetType_M32,                                   // M32
    Astro_TargetType_M33,                                   // M33
    Astro_TargetType_M34,                                   // M34
    Astro_TargetType_M35,                                   // M35
    Astro_TargetType_M36,                                   // M36
    Astro_TargetType_M37,                                   // M37
    Astro_TargetType_M38,                                   // M38
    Astro_TargetType_M39,                                   // M39
    Astro_TargetType_M40,                                   // M40
    Astro_TargetType_M41,                                   // M41
    Astro_TargetType_M42,                                   // M42
    Astro_TargetType_M43,                                   // M43
    Astro_TargetType_M44,                                   // M44
    Astro_TargetType_M45,                                   // M45
    Astro_TargetType_M46,                                   // M46
    Astro_TargetType_M47,                                   // M47
    Astro_TargetType_M48,                                   // M48
    Astro_TargetType_M49,                                   // M49
    Astro_TargetType_M50,                                   // M50
    Astro_TargetType_M51,                                   // M51
    Astro_TargetType_M52,                                   // M52
    Astro_TargetType_M53,                                   // M53
    Astro_TargetType_M54,                                   // M54
    Astro_TargetType_M55,                                   // M55
    Astro_TargetType_M56,                                   // M56
    Astro_TargetType_M57,                                   // M57
    Astro_TargetType_M58,                                   // M58
    Astro_TargetType_M59,                                   // M59
    Astro_TargetType_M60,                                   // M60
    Astro_TargetType_M61,                                   // M61
    Astro_TargetType_M62,                                   // M62
    Astro_TargetType_M63,                                   // M63
    Astro_TargetType_M64,                                   // M64
    Astro_TargetType_M65,                                   // M65
    Astro_TargetType_M66,                                   // M66
    Astro_TargetType_M67,                                   // M67
    Astro_TargetType_M68,                                   // M68
    Astro_TargetType_M69,                                   // M69
    Astro_TargetType_M70,                                   // M70
    Astro_TargetType_M71,                                   // M71
    Astro_TargetType_M72,                                   // M72
    Astro_TargetType_M73,                                   // M73
    Astro_TargetType_M74,                                   // M74
    Astro_TargetType_M75,                                   // M75
    Astro_TargetType_M76,                                   // M76
    Astro_TargetType_M77,                                   // M77
    Astro_TargetType_M78,                                   // M78
    Astro_TargetType_M79,                                   // M79
    Astro_TargetType_M80,                                   // M80
    Astro_TargetType_M81,                                   // M81
    Astro_TargetType_M82,                                   // M82
    Astro_TargetType_M83,                                   // M83
    Astro_TargetType_M84,                                   // M84
    Astro_TargetType_M85,                                   // M85
    Astro_TargetType_M86,                                   // M86
    Astro_TargetType_M87,                                   // M87
    Astro_TargetType_M88,                                   // M88
    Astro_TargetType_M89,                                   // M89
    Astro_TargetType_M90,                                   // M90
    Astro_TargetType_M91,                                   // M91
    Astro_TargetType_M92,                                   // M92
    Astro_TargetType_M93,                                   // M93
    Astro_TargetType_M94,                                   // M94
    Astro_TargetType_M95,                                   // M95
    Astro_TargetType_M96,                                   // M96
    Astro_TargetType_M97,                                   // M97
    Astro_TargetType_M98,                                   // M98
    Astro_TargetType_M99,                                   // M99
    Astro_TargetType_M100,                                  // M100
    Astro_TargetType_M101,                                  // M101
    Astro_TargetType_M102,                                  // M102
    Astro_TargetType_M103,                                  // M103
    Astro_TargetType_M104,                                  // M104
    Astro_TargetType_M105,                                  // M105
    Astro_TargetType_M106,                                  // M106
    Astro_TargetType_M107,                                  // M107
    Astro_TargetType_M108,                                  // M108
    Astro_TargetType_M109,                                  // M109
    Astro_TargetType_M110,                                  // M110
    Astro_TargetType_CustomTarget1,                         // Custom target 1
    Astro_TargetType_CustomTarget2,                         // Custom target 2
    Astro_TargetType_CustomTarget3,                         // Custom target 3
    Astro_TargetType_CustomTarget4,                         // Custom target 4
    Astro_TargetType_CustomTarget5,                         // Custom target 5
    Astro_TargetType_CustomTarget6,                         // Custom target 6
    Astro_TargetType_CustomTarget7,                         // Custom target 7
    Astro_TargetType_CustomTarget8,                         // Custom target 8

    Astro_TargetType_Count,                                 // Placeholder
    Astro_TargetType_CustomTargetCount = 8,                 // Placeholder
    Astro_TargetType_Undefined = 0xff                       // Placeholder
};

// Target Class
// Broad astronomical object classification used by target catalog metadata.
enum Astro_TargetClass : signed char {
    Astro_TargetClass_Star,                                 // Star
    Astro_TargetClass_OpenCluster,                          // Open cluster
    Astro_TargetClass_GlobularCluster,                      // Globular cluster
    Astro_TargetClass_Nebula,                               // Nebula
    Astro_TargetClass_PlanetaryNebula,                      // Planetary nebula
    Astro_TargetClass_Galaxy,                               // Galaxy
    Astro_TargetClass_SolarSystem,                          // Solar system object
    Astro_TargetClass_Other,                                // Other target class

    Astro_TargetClass_Count,                                // Placeholder
    Astro_TargetClass_Unknown = -1                          // Placeholder
};

// System Run Mode
// Specifies how mount target positions are determined and controlled.
enum Astro_SystemMode : signed char {
    Astro_SystemMode_Tracking,                              // Astronomical position tracking from time/location and target coordinates
    Astro_SystemMode_Balancing,                             // Sensor-feedback balancing/correction mode
    Astro_SystemMode_Manual,                                // User/external code controls mount targets directly

    Astro_SystemMode_Count,                                 // Placeholder
    Astro_SystemMode_Undefined = -1                         // Placeholder
};

// Measurement Units Mode
// Specifies the standard of measurement style that units will use.
enum Astro_MeasurementMode : signed char {
    Astro_MeasurementMode_Imperial,                         // Imperial measurement mode (default setting, °F Ft Gal Lbs M-D-Y Val.X etc)
    Astro_MeasurementMode_Metric,                           // Metric measurement mode (°C M L Kg Y-M-D Val.X etc)
    Astro_MeasurementMode_Scientific,                       // Scientific measurement mode (°K M L Kg Y-M-D Val.XX etc)

    Astro_MeasurementMode_Count,                            // Placeholder
    Astro_MeasurementMode_Undefined = -1,                   // Placeholder
    Astro_MeasurementMode_Default = Astro_MeasurementMode_Metric // Default system measurement mode (alias, feel free to change)
};

// LCD/Display Output Mode
// Specifies what kind of visual output device is to be used.
// Display output mode support provided by tcMenu.
enum Astro_DisplayOutputMode : signed char {
    Astro_DisplayOutputMode_Disabled,                       // No display output
    Astro_DisplayOutputMode_LCD16x2_EN,                     // 16x2 text LCD (with EN first, pins: {EN,RW,RS,BL,Data}), using LiquidCrystalIO (i2c only)
    Astro_DisplayOutputMode_LCD16x2_RS,                     // 16x2 text LCD (with RS first, pins: {RS,RW,EN,BL,Data}), using LiquidCrystalIO (i2c only)
    Astro_DisplayOutputMode_LCD20x4_EN,                     // 20x4 text LCD (with EN first, pins: {EN,RW,RS,BL,Data}), using LiquidCrystalIO (i2c only)
    Astro_DisplayOutputMode_LCD20x4_RS,                     // 20x4 text LCD (with RS first, pins: {RS,RW,EN,BL,Data}), using LiquidCrystalIO (i2c only)
    Astro_DisplayOutputMode_SSD1305,                        // SSD1305 128x32 OLED, using U8g2 (i2c or SPI)
    Astro_DisplayOutputMode_SSD1305_x32Ada,                 // Adafruit SSD1305 128x32 OLED, using U8g2 (i2c or SPI)
    Astro_DisplayOutputMode_SSD1305_x64Ada,                 // Adafruit SSD1305 128x64 OLED, using U8g2 (i2c or SPI)
    Astro_DisplayOutputMode_SSD1306,                        // SSD1306 128x64 OLED, using U8g2 (i2c or SPI)
    Astro_DisplayOutputMode_SH1106,                         // SH1106 128x64 OLED, using U8g2 (i2c or SPI)
    Astro_DisplayOutputMode_CustomOLED,                     // Custom OLED, using U8g2 (i2c or SPI, note: custom device/size defined statically by ASTRO_UI_CUSTOM_OLED_I2C / ASTRO_UI_CUSTOM_OLED_SPI)
    Astro_DisplayOutputMode_SSD1607,                        // SSD1607 200x200 OLED, using U8g2 (SPI only)
    Astro_DisplayOutputMode_IL3820,                         // IL3820 296x128 OLED, using U8g2 (SPI only)
    Astro_DisplayOutputMode_IL3820_V2,                      // IL3820 V2 296x128 OLED, using U8g2 (SPI only)
    Astro_DisplayOutputMode_ST7735,                         // ST7735 graphical LCD, using AdafruitGFX (SPI only, note: usage requires correct tag color enumeration setting)
    Astro_DisplayOutputMode_ST7789,                         // ST7789 graphical LCD, using AdafruitGFX (SPI only, note: usage requires correct screen resolution enumeration setting, CustomTFT size defined statically by TFT_GFX_WIDTH & TFT_GFX_HEIGHT)
    Astro_DisplayOutputMode_ILI9341,                        // ILI9341 graphical LCD, using AdafruitGFX (SPI only)
    Astro_DisplayOutputMode_TFT,                            // Graphical LCD, using TFT_eSPI (SPI only, note: usage requires editing TFT_eSPI/User_Setup.h & properly defining TFT_CS, TFT_DC, & TFT_RST, size defined statically by TFT_GFX_WIDTH & TFT_GFX_HEIGHT which defaults to TFT_WIDTH & TFT_HEIGHT)

    Astro_DisplayOutputMode_Count,                          // Placeholder
    Astro_DisplayOutputMode_Undefined = -1                  // Placeholder
};

// Control Input Mode
// Specifies what kind of control input mode (and pin ribbon set) is to be used.
// Control input mode support provided by tcMenu.
enum Astro_ControlInputMode : signed char {
    Astro_ControlInputMode_Disabled,                        // No control input
    Astro_ControlInputMode_RotaryEncoderOk,                 // Rotary encoder /w momentary Ok button, pins: {eA,eB,Ok}
    Astro_ControlInputMode_RotaryEncoderOkLR,               // Rotary encoder /w momentary Ok, Back(L), and Next(R) buttons, pins: {eA,eB,Ok,Bk,Nx}
    Astro_ControlInputMode_UpDownButtonsOk,                 // Momentary Up, Down, and Ok buttons, pins: {Up,Dw,Ok}
    Astro_ControlInputMode_UpDownButtonsOkLR,               // Momentary Up, Down, Ok, Back(L), and Next(R) buttons, pins: {Up,Dw,Ok,Bk,Nx}
    Astro_ControlInputMode_UpDownESP32TouchOk,              // ESP32-Touch Up, Down, and Ok keys, pins: {Up,Dw,Ok}
    Astro_ControlInputMode_UpDownESP32TouchOkLR,            // ESP32-Touch Up, Down, Ok, Back(L), and Next(R) keys, pins: {Up,Dw,Ok,Bk,Nx}
    Astro_ControlInputMode_AnalogJoystickOk,                // Analog joystick /w momentary Ok button, pins: {aX,aY,Ok} (aX can be unused/-1, else used for back/next)
    Astro_ControlInputMode_Matrix2x2UpDownButtonsOkL,       // 2x2 matrix keypad as momentary Up, Down, Ok, and Back(L) buttons, pins: {r0,r1,c0,c1}
    Astro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk,   // 3x4 numeric matrix keyboard, & optional rotary encoder /w momentary Ok button, pins: {r0,r1,r2,r3,c0,c1,c2,eA,eB,Ok} (eA can be unused/-1)
    Astro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR, // 3x4 numeric matrix keyboard, & optional rotary encoder /w momentary Ok, Back(L), and Next(R) buttons, pins: {r0,r1,r2,r3,c0,c1,c2,eA,eB,Ok,Bk,Nx} (eA can be unused/-1)
    Astro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk,   // 4x4 alpha-numeric matrix keyboard, & optional rotary encoder /w momentary Ok button, pins: {r0,r1,r2,r3,c0,c1,c2,c3,eA,eB,Ok} (eA can be unused/-1)
    Astro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR, // 4x4 alpha-numeric matrix keyboard, & optional rotary encoder /w momentary Ok, Back(L), and Next(R) buttons, pins: {r0,r1,r2,r3,c0,c1,c2,c3,eA,eB,Ok,Bk,Nx} (eA can be unused/-1)
    Astro_ControlInputMode_ResistiveTouch,                  // Resistive touchscreen, pins: {X+,X-,Y+,Y-} (X-/Y- analog, X+/Y+ digital)
    Astro_ControlInputMode_TouchScreen,                     // Full touchscreen (FT6206, or XPT2046 /w setup define), pins: FT6206: {}, XPT2046: {tCS,tIRQ} (tIRQ can be unused/-1, FT6206 hard-coded to use Wire for i2c)
    Astro_ControlInputMode_TFTTouch,                        // TFT Touch (XPT2046), using TFT_eSPI (Note: usage requires TFT display mode & editing TFT_eSPI/User_Setup.h & properly defining TOUCH_CS), pins: {tCS,tIRQ} (tIRQ can be unused/-1)
    Astro_ControlInputMode_RemoteControl,                   // Remote controlled (no input /w possibly disabled display), pins: {}

    Astro_ControlInputMode_Count,                           // Placeholder
    Astro_ControlInputMode_Undefined = -1                   // Placeholder
};

// Actuator Type
// Control actuator type. Specifies the various controllable equipment and their usage.
enum Astro_ActuatorType : signed char {
    Astro_ActuatorType_MountAxis,                           // Mount axis
    Astro_ActuatorType_Cover,                               // Cover
    Astro_ActuatorType_DewHeater,                           // Dew heater
    Astro_ActuatorType_CameraCooler,                        // Camera cooler
    Astro_ActuatorType_Fan,                                 // Fan
    Astro_ActuatorType_Focuser,                             // Focuser

    Astro_ActuatorType_Count,                               // Placeholder
    Astro_ActuatorType_Undefined = -1                       // Placeholder
};

// Sensor Type
// Sensor device type. Specifies the various sensors and the kinds of things they measure.
enum Astro_SensorType : signed char {
    Astro_SensorType_Temperature,                           // Temperature sensor
    Astro_SensorType_Humidity,                              // Humidity sensor
    Astro_SensorType_Position,                              // Position sensor
    Astro_SensorType_LimitSwitch,                           // Limit switch sensor
    Astro_SensorType_Rain,                                  // Rain/wet sensor
    Astro_SensorType_WindSpeed,                             // Wind speed sensor
    Astro_SensorType_Light,                                 // Light sensor
    Astro_SensorType_Voltage,                               // Voltage sensor
    Astro_SensorType_Current,                               // Current sensor
    Astro_SensorType_CameraTemperature,                     // Camera temperature sensor

    Astro_SensorType_Count,                                 // Placeholder
    Astro_SensorType_Undefined = -1                         // Placeholder
};

// Mount Type
// Telescope/tracker mount geometry.
enum Astro_MountType : signed char {
    Astro_MountType_Equatorial,                             // Equatorial mount
    Astro_MountType_AltAzimuth,                             // Altitude/azimuth mount
    Astro_MountType_SingleAxis,                             // Single-axis sidereal tracker

    Astro_MountType_Count,                                  // Placeholder
    Astro_MountType_Unknown = -1                            // Placeholder
};

// Power Rail
// Common power rails. Specifies an isolated operational power rail unit.
enum Astro_RailType : signed char {
    Astro_RailType_AC110V,                                  // ~110V AC-based power rail
    Astro_RailType_AC220V,                                  // ~220V AC-based power rail
    Astro_RailType_DC3V3,                                   // 3.3v DC-based power rail
    Astro_RailType_DC5V,                                    // 5v DC-based power rail
    Astro_RailType_DC12V,                                   // 12v DC-based power rail
    Astro_RailType_DC24V,                                   // 24v DC-based power rail
    Astro_RailType_DC48V,                                   // 48v DC-based power rail

    Astro_RailType_Count,                                   // Placeholder
    Astro_RailType_Undefined = -1,                          // Placeholder
    Astro_RailType_DefaultAC = Astro_RailType_AC110V        // Default AC rating for AC-based power rail (alias, feel free to change)
};

// Pin Mode
// Pin mode setting. Specifies what kind of pin and how it's used.
enum Astro_PinMode : signed char {
    Astro_PinMode_Digital_Input_Floating,                   // Digital input pin as floating/no-pull (pull-up/pull-down disabled, used during mux channel select, type alias for INPUT/GPIO_PuPd_NOPULL)
    Astro_PinMode_Digital_Input_PullUp,                     // Digital input pin with pull-up resistor (default pairing for active-low trigger, type alias for INPUT_PULLUP/GPIO_PuPd_UP)
    Astro_PinMode_Digital_Input_PullDown,                   // Digital input pin with pull-down resistor (or pull-up disabled if not avail, default pairing for active-high trigger, type alias for INPUT_PULLDOWN/GPIO_PuPd_DOWN)
    Astro_PinMode_Digital_Output_OpenDrain,                 // Digital output pin with open-drain NPN-based sink (default pairing for active-low trigger, type alias for OUTPUT/GPIO_OType_OD)
    Astro_PinMode_Digital_Output_PushPull,                  // Digital output pin with push-pull NPN+PNP-based sink+src (default pairing for active-high trigger, type alias for GPIO_OType_PP)
    Astro_PinMode_Analog_Input,                             // Analog input pin (type alias for INPUT)
    Astro_PinMode_Analog_Output,                            // Analog output pin (type alias for OUTPUT)

    Astro_PinMode_Count,                                    // Placeholder
    Astro_PinMode_Undefined = -1,                           // Placeholder
    Astro_PinMode_Digital_Input = Astro_PinMode_Digital_Input_Floating, // Default digital input (alias for Floating, type for INPUT)
    Astro_PinMode_Digital_Output = Astro_PinMode_Digital_Output_OpenDrain // Default digital output (alias for OpenDrain, type for OUTPUT)
};

// Enable Mode
// Actuator intensity/enablement calculation mode. Specifies how multiple activations get used together.
enum Astro_EnableMode : signed char {
    Astro_EnableMode_Highest,                               // Parallel activation using highest drive intensity
    Astro_EnableMode_Lowest,                                // Parallel activation using lowest drive intensity
    Astro_EnableMode_Average,                               // Parallel activation using averaged drive intensities
    Astro_EnableMode_Multiply,                              // Parallel activation using multiplied drive intensities

    Astro_EnableMode_InOrder,                               // Serial activation using in-order/fifo-queue drive intensities
    Astro_EnableMode_RevOrder,                              // Serial activation using reverse-order/lifo-stack drive intensities
    Astro_EnableMode_DescOrder,                             // Serial activation using highest-to-lowest/descending-order drive intensities
    Astro_EnableMode_AscOrder,                              // Serial activation using lowest-to-highest/ascending-order drive intensities

    Astro_EnableMode_Count,                                 // Placeholder
    Astro_EnableMode_Undefined = -1,                        // Placeholder
    Astro_EnableMode_Serial = Astro_EnableMode_InOrder      // Serial activation (alias for InOrder)
};

// Direction Mode
// Actuator intensity application mode. Specifies activation directionality and enablement.
enum Astro_DirectionMode : signed char {
    Astro_DirectionMode_Forward,                            // Standard/forward direction mode
    Astro_DirectionMode_Reverse,                            // Opposite/reverse direction mode
    Astro_DirectionMode_Stop,                               // Stationary/braking direction mode  (intensity undef)

    Astro_DirectionMode_Count,                              // Placeholder
    Astro_DirectionMode_Undefined = -1                      // Placeholder
};

// Trigger Status
// Common trigger statuses. Specifies enablement and tripped state.
enum Astro_TriggerState : signed char {
    Astro_TriggerState_Disabled,                            // Triggers disabled (not hooked up)
    Astro_TriggerState_NotTriggered,                        // Not triggered
    Astro_TriggerState_Triggered,                           // Triggered

    Astro_TriggerState_Count,                               // Placeholder
    Astro_TriggerState_Undefined = -1                       // Placeholder
};

// Thermal Control Mode
// Selects thermal-balancing goals for storage, observing, or safe stow.
enum Astro_ThermalMode : signed char {
    Astro_ThermalMode_DayStorage,                           // Day storage
    Astro_ThermalMode_NightObserving,                       // Night observing
    Astro_ThermalMode_SafeStowed,                           // Safe stowed

    Astro_ThermalMode_Count,                                // Placeholder
    Astro_ThermalMode_Undefined = -1                        // Placeholder
};

// Scheduler Stage
// Current stage of the automatic nighttime observation sequence.
enum Astro_SchedulerStage : signed char {
    Astro_SchedulerStage_DayStowed,                         // Day stowed
    Astro_SchedulerStage_Deploying,                         // Deploying
    Astro_SchedulerStage_Cooling,                           // Cooling
    Astro_SchedulerStage_Slewing,                           // Slewing
    Astro_SchedulerStage_Settling,                          // Settling
    Astro_SchedulerStage_Observing,                         // Observing
    Astro_SchedulerStage_Warming,                           // Warming
    Astro_SchedulerStage_Stowing,                           // Stowing
    Astro_SchedulerStage_SafeStowed,                        // Safe stowed
    Astro_SchedulerStage_Fault,                             // Hardware or motion fault requiring intervention

    Astro_SchedulerStage_Count,                             // Placeholder
    Astro_SchedulerStage_Undefined = -1                     // Placeholder
};

// Units Category
// Unit of measurement category. Specifies the kind of unit.
enum Astro_UnitsCategory : signed char {
    Astro_UnitsCategory_Raw,                                // Raw/dimensionless unit
    Astro_UnitsCategory_Angle,                              // Angle based unit
    Astro_UnitsCategory_Distance,                           // Distance/position based unit
    Astro_UnitsCategory_Percentile,                         // Percentile based unit
    Astro_UnitsCategory_Speed,                              // Speed/travel based unit
    Astro_UnitsCategory_Temperature,                        // Temperature based unit
    Astro_UnitsCategory_Humidity,                           // Humidity based unit
    Astro_UnitsCategory_Power,                              // Power based unit
    Astro_UnitsCategory_Voltage,                            // Voltage based unit
    Astro_UnitsCategory_Current,                            // Current based unit

    Astro_UnitsCategory_Count,                              // Placeholder
    Astro_UnitsCategory_Undefined = -1                      // Placeholder
};

// Units Type
// Unit of measurement type. Specifies the unit type associated with a measured value.
// Note: Rate units may only be in per second, use PER_X_TO_PER_Y defines to convert.
enum Astro_UnitsType : signed char {
    Astro_UnitsType_Raw_1,                                  // Normalized raw value mode [0,1=aRef]
    Astro_UnitsType_Angle_Degrees_360,                      // Degrees angle mode [0,%360)
    Astro_UnitsType_Angle_Radians_2pi,                      // Radians angle mode [0,%2pi)
    Astro_UnitsType_Distance_Meters,                        // Meters distance mode
    Astro_UnitsType_Distance_Feet,                          // Feet distance mode
    Astro_UnitsType_Percentile_100,                         // Percentile mode [0,100]
    Astro_UnitsType_Speed_MetersPerSec,                     // Meters per second speed mode
    Astro_UnitsType_Speed_FeetPerSec,                       // Feet per second speed mode
    Astro_UnitsType_Temperature_Celsius,                    // Celsius temperature mode
    Astro_UnitsType_Temperature_Fahrenheit,                 // Fahrenheit temperature mode
    Astro_UnitsType_Temperature_Kelvin,                     // Kelvin temperature mode
    Astro_UnitsType_Humidity_RH,                            // Relative humidity percent mode
    Astro_UnitsType_Power_Wattage,                          // Wattage power mode
    Astro_UnitsType_Voltage_Volts,                          // Voltage mode
    Astro_UnitsType_Current_Amperage,                       // Amperage current mode

    Astro_UnitsType_Count,                                  // Placeholder
    Astro_UnitsType_Power_JoulesPerSecond = Astro_UnitsType_Power_Wattage, // Joules per second power mode alias
    Astro_UnitsType_Undefined = -1                          // Placeholder
};

#define PER_SEC_TO_PER_MIN(t)       ((t) * (SECS_PER_MIN))  // Per seconds to per minutes
#define PER_SEC_TO_PER_HR(t)        ((t) * (SECS_PER_HOUR)) // Per seconds to per hour
#define PER_MIN_TO_PER_SEC(t)       ((t) / (SECS_PER_MIN))  // Per minutes to per seconds
#define PER_MIN_TO_PER_HR(t)        ((t) * (SECS_PER_MIN))  // Per minutes to per hour
#define PER_HR_TO_PER_SEC(t)        ((t) / (SECS_PER_HOUR)) // Per hour to per seconds
#define PER_HR_TO_PER_MIN(t)        ((t) / (SECS_PER_MIN))  // Per hour to per minutes

// Common forward decls
class Astruino;
class AstroScheduler;
class AstroLogger;
class AstroPublisher;
struct AstroIdentity;
struct AstroData;
struct AstroSubData;
struct AstroCalibrationData;
struct AstroTargetsLibData;
struct AstroObjectData;
struct AstroSystemData;
struct AstroPin;
struct AstroDigitalPin;
struct AstroAnalogPin;
struct AstroActivation;
struct AstroActivationHandle;
struct AstroMeasurement;
struct AstroSingleMeasurement;
class AstroObject;
class AstroSubObject;
class AstroDLinkObject;
class AstroAttachment;
class AstroActuatorAttachment;
class AstroSensorAttachment;
class AstroTriggerAttachment;
class AstroTrigger;
class AstroActuator;
class AstroSensor;
class AstroTarget;
class AstroMount;
class AstroRail;

// System sketches setup enums (for non-zero resolution)
#define SETUP_ENUM_Disabled                                 -1
#define SETUP_ENUM_None                                     -1
#define SETUP_ENUM_Count                                    -1
#define SETUP_ENUM_Undefined                                -1
#define SETUP_ENUM_Primary                                  10
#define SETUP_ENUM_Fallback                                 11
#define SETUP_ENUM_Minimal                                  20
#define SETUP_ENUM_Full                                     21
#define SETUP_ENUM_UART                                     30
#define SETUP_ENUM_I2C                                      31
#define SETUP_ENUM_SPI                                      32
#define SETUP_ENUM_LCD                                      40
#define SETUP_ENUM_Pixel                                    41
#define SETUP_ENUM_ST7735                                   42
#define SETUP_ENUM_TFT                                      43
#define SETUP_ENUM_Encoder                                  50
#define SETUP_ENUM_Buttons                                  51
#define SETUP_ENUM_Joystick                                 52
#define SETUP_ENUM_Matrix                                   53
#define SETUP_ENUM_Serial                                   60
#define SETUP_ENUM_Simhub                                   61
#define SETUP_ENUM_WiFi                                     62
#define SETUP_ENUM_Ethernet                                 63
#define SETUP_ENUM_Hostname                                 70
#define SETUP_ENUM_IPAddress                                71
// Display & controller setup enums (for non-zero resolution)
#define SETUP_ENUM_LCD16x2_EN                               100
#define SETUP_ENUM_LCD16x2_RS                               101
#define SETUP_ENUM_LCD20x4_EN                               102
#define SETUP_ENUM_LCD20x4_RS                               103
#define SETUP_ENUM_SSD1305                                  104
#define SETUP_ENUM_SSD1305_x32Ada                           105
#define SETUP_ENUM_SSD1305_x64Ada                           106
#define SETUP_ENUM_SSD1306                                  107
#define SETUP_ENUM_SH1106                                   108
#define SETUP_ENUM_CustomOLED                               109
#define SETUP_ENUM_SSD1607                                  110
#define SETUP_ENUM_IL3820                                   111
#define SETUP_ENUM_IL3820_V2                                112
#define SETUP_ENUM_ST7789                                   113
#define SETUP_ENUM_ILI9341                                  114
#define SETUP_ENUM_RotaryEncoderOk                          115
#define SETUP_ENUM_RotaryEncoderOkLR                        116
#define SETUP_ENUM_UpDownButtonsOk                          117
#define SETUP_ENUM_UpDownButtonsOkLR                        118
#define SETUP_ENUM_UpDownESP32TouchOk                       119
#define SETUP_ENUM_UpDownESP32TouchOkLR                     120
#define SETUP_ENUM_AnalogJoystickOk                         121
#define SETUP_ENUM_Matrix2x2UpDownButtonsOkL                122
#define SETUP_ENUM_Matrix3x4Keyboard_OptRotEncOk            123
#define SETUP_ENUM_Matrix3x4Keyboard_OptRotEncOkLR          124
#define SETUP_ENUM_Matrix4x4Keyboard_OptRotEncOk            125
#define SETUP_ENUM_Matrix4x4Keyboard_OptRotEncOkLR          126
#define SETUP_ENUM_ResistiveTouch                           127
#define SETUP_ENUM_TouchScreen                              128
#define SETUP_ENUM_TFTTouch                                 129
#define SETUP_ENUM_RemoteControl                            130
// Checks setup defines for equality, first param SETUP_XXX is substituted (possibly to 0), second param literal should be defined (for non-zero substitution)
#define IS_SETUP_AS(X,Y)                JOIN(SETUP_ENUM,X) == SETUP_ENUM_##Y
// Checks setup defines for inequality, first param SETUP_XXX is substituted (possibly to 0), second param literal should be defined (for non-zero substitution)
#define NOT_SETUP_AS(X,Y)               JOIN(SETUP_ENUM,X) != SETUP_ENUM_##Y

#endif // /ifndef AstroDefines_H
