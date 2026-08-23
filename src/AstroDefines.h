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
#define ASTRO_POS_MAXSIZE               32                  // Position indicies maximum size (max # of objs of same type)
#define ASTRO_URL_MAXSIZE               64                  // URL string maximum size (max url length)
#define ASTRO_JSON_DOC_SYSSIZE          256                 // JSON document chunk data bytes for reading in main system data (serialization buffer size)
#define ASTRO_JSON_DOC_DEFSIZE          192                 // Default JSON document chunk data bytes (serialization buffer size)
#define ASTRO_STRING_BUFFER_SIZE        32                  // Size in bytes of string serialization buffers
#define ASTRO_WIFISTREAM_BUFFER_SIZE    128                 // Size in bytes of WiFi serialization buffers
// The following sizes only apply to architectures that do not have STL support (AVR/SAM)
#define ASTRO_DEFAULT_MAXSIZE           8                   // Default maximum array/map size
#define ASTRO_ACTUATOR_SIGNAL_SLOTS     4                   // Maximum number of slots for actuator's activation signal
#define ASTRO_SENSOR_SIGNAL_SLOTS       6                   // Maximum number of slots for sensor's measurement signal
#define ASTRO_TRIGGER_SIGNAL_SLOTS      4                   // Maximum number of slots for trigger's state signal
#define ASTRO_BALANCER_SIGNAL_SLOTS     2                   // Maximum number of slots for balancer's state signal
#define ASTRO_BALANCER_STALE_FRAMES     3                   // Maximum sensor frames balancers will act on without a fresh reading
#define ASTRO_LOG_SIGNAL_SLOTS          2                   // Maximum number of slots for system log signal
#define ASTRO_PUBLISH_SIGNAL_SLOTS      2                   // Maximum number of slots for data publish signal
#define ASTRO_RESERVOIR_SIGNAL_SLOTS    2                   // Maximum number of slots for filled/empty signal
#define ASTRO_RAIL_SIGNAL_SLOTS         8                   // Maximum number of slots for rail capacity signal
#define ASTRO_SYS_OBJECTS_MAXSIZE       16                  // Maximum array size for system objects (max # of objects in system)
#define ASTRO_CAL_CALIBS_MAXSIZE        8                   // Maximum array size for calibration store objects (max # of different custom calibrations)
#define ASTRO_OBJ_LINKS_MAXSIZE         8                   // Maximum array size for object linkage list, per obj (max # of linked objects)
#define ASTRO_BAL_ACTUATORS_MAXSIZE     8                   // Maximum array size for balancer actuators list (max # of actuators used)
#define ASTRO_SCH_REQACTS_MAXSIZE       4                   // Maximum array size for scheduler required actuators list (max # of actuators active per process stage)
#define ASTRO_SYS_ONEWIRES_MAXSIZE      2                   // Maximum array size for pin OneWire list (max # of OneWire comm pins)
#define ASTRO_SYS_PINLOCKS_MAXSIZE      2                   // Maximum array size for pin locks list (max # of locks)
#define ASTRO_SYS_PINMUXERS_MAXSIZE     2                   // Maximum array size for pin muxers list (max # of muxers)
#define ASTRO_SYS_PINEXPANDERS_MAXSIZE  2                   // Maximum array size for pin expanders list (max # of expanders)


#define ASTRO_CONTROL_LOOP_INTERVAL     100                 // Run interval of main control loop, in milliseconds
#define ASTRO_DATA_LOOP_INTERVAL        2000                // Default run interval of data loop, in milliseconds (customizable later)
#define ASTRO_MISC_LOOP_INTERVAL        250                 // Run interval of misc loop, in milliseconds

#define ASTRO_ACT_PUMPCALC_UPDATEMS     250                 // Minimum time millis needing to pass before a pump reports/writes changed volume to reservoir (reduces error accumulation)
#define ASTRO_ACT_PUMPCALC_MINFLOWRATE  0.05f               // What percentage of continuous flow rate an instantaneous flow rate sensor must achieve before it is used in pump/volume calculations (reduces near-zero error jitters)

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
    Astro_DisplayOutputMode_CustomOLED,                     // Custom OLED, using U8g2 (i2c or SPI, note: custom device/size defined statically by HYDRO_UI_CUSTOM_OLED_I2C / HYDRO_UI_CUSTOM_OLED_SPI)
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

// Units Type
// Unit of measurement type. Specifies the unit type associated with a measured value.
// Note: Rate units may only be in per minute, use PER_X_TO_PER_Y defines to convert.
enum Astro_UnitsType : signed char {
    Astro_UnitsType_Raw_1,                                  // Normalized raw value mode [0,1=aRef]
    Astro_UnitsType_Percentile_100,                         // Percentile mode [0,100]
    Astro_UnitsType_Alkalinity_pH_14,                       // pH value alkalinity mode [0,14]
    Astro_UnitsType_Concentration_EC_5,                     // Siemens electrical conductivity concentration mode [0,5] (aka mS/cm)
    Astro_UnitsType_Concentration_PPM_500,                  // Parts-per-million 500 concentration mode [0,2500] (NaCl-based, common for US)
    Astro_UnitsType_Concentration_PPM_640,                  // Parts-per-million 640 concentration mode [0,3200] (common for EU)
    Astro_UnitsType_Concentration_PPM_700,                  // Parts-per-million 700 concentration mode [0,3500] (KCl-based, common for AU)
    Astro_UnitsType_Distance_Feet,                          // Feet distance mode
    Astro_UnitsType_Distance_Meters,                        // Meters distance mode
    Astro_UnitsType_LiqDilution_MilliLiterPerGallon,        // Milli liter per gallon dilution mode
    Astro_UnitsType_LiqDilution_MilliLiterPerLiter,         // Milli liter per liter dilution mode
    Astro_UnitsType_LiqFlowRate_GallonsPerMin,              // Gallons per minute liquid flow rate mode
    Astro_UnitsType_LiqFlowRate_LitersPerMin,               // Liters per minute liquid flow rate mode
    Astro_UnitsType_LiqVolume_Gallons,                      // Gallons liquid volume mode
    Astro_UnitsType_LiqVolume_Liters,                       // Liters liquid volume mode
    Astro_UnitsType_Power_Amperage,                         // Amperage current power mode
    Astro_UnitsType_Power_Wattage,                          // Wattage power mode
    Astro_UnitsType_Temperature_Celsius,                    // Celsius temperature mode
    Astro_UnitsType_Temperature_Fahrenheit,                 // Fahrenheit temperature mode
    Astro_UnitsType_Temperature_Kelvin,                     // Kelvin temperature mode
    Astro_UnitsType_Weight_Kilograms,                       // Kilograms weight mode
    Astro_UnitsType_Weight_Pounds,                          // Pounds weight mode

    Astro_UnitsType_Count,                                  // Placeholder
    Astro_UnitsType_Concentration_TDS = Astro_UnitsType_Concentration_EC_5, // Standard TDS concentration mode alias
    Astro_UnitsType_Concentration_PPM = Astro_UnitsType_Concentration_PPM_500, // Standard PPM concentration mode alias
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
struct AstroCustomAdditiveData;
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
