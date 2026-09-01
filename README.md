# Astruino
Astruino: Simple Astro Tracker Automation Controller.

**Simple-AstroTracker-Arduino v0.7.2.0**

Simple automation controller for DIY astronomical tracking systems.
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

This controller manages mounts, axis drivers, covers, cameras, focusers, environmental sensors, thermal control, targets, scheduling, logging, publishing, and persistent configuration for home-built astronomical tracking systems. The actual mechanical and electrical implementation remains open to the builder.

Our Keep-It-Simple controller system:

* Can be used entirely offline with RTC module and optional GPS module (or known static location) for accurate time keeping, or used online through enabled on-board WiFi/Ethernet or external ESP-AT WiFi module.
  * Uses offline astronomical calculations for target positioning and [SolarCalculator](https://github.com/jpb10/SolarCalculator) for sunrise, sunset, & transit calculations used by system scheduling.
* Exportable system configuration to EEPROM, SD card, or WiFiStorage external storage device.
  * Saved in pretty-print JSON for human-readability & easy text editing, or in raw binary for compactness & speed.
  * Auto-save, backup auto-save (for auto-recovery), and low external storage space cleanup (TODO) functionality.
  * Import string decode functions are pre-optimized with minimum spanning trie for ultra-fast text parsing & reduced loading times.
* Supports interval-based sensor data publishing and system event logging to MQTT IoT broker (for further IoT-integrated processing) or to external storage in .csv/.txt format (/w date in filename, segmented daily).
  * Can be extended to work with other JSON-based Web APIs or Client-like derivatives (for DB storage or server-endpoint support).
  * Can add a piezo buzzer for audible system warning/failure alerting (TODO), or a LCD/OLED/TFT display for current readings & recent logging messages (TODO).
* Enabled GUI works with a large variety of common Arduino-compatible LCD/OLED/TFT displays, touchscreens, matrix keypads, analog joysticks, rotary encoders, and momentary buttons (support by [tcMenu](https://github.com/davetcc/tcMenuLib)).
  * Contains at-a-glance system overview screen and interactive menu system for system configuration, sensor calibration, and more (TODO).
  * Critical system config menus can be pin-coded to prevent setup tampering.
  * Includes remote GUI menu access through enabled WiFi, Ethernet, Bluetooth, Serial, and/or Simhub connection via tcMenu's excellent [embedCONTROL](https://github.com/davetcc/tcMenu/releases) desktop application, available for Linux/OSX/Windows.
  * GUI I/O pins can be setup as fully interrupt driven (5-25ms latency), partially interrupt driven (only keys & buttons polled), or polling based (75-100ms+ latency), and can be automatically selected depending on pins used.
  * System examples can be compiled in:
    * Disabled UI mode, which removes all GUI code entirely, freeing a large amount of Flash size for constrained (<=256kB Flash) devices.
    * Minimal UI mode, which saves on compiled sketch size through optimized code stripping at the cost of having to modify/re-upload a new sketch to change most system settings (or to change system object structure).
    * Full UI mode, which uses large amounts of Flash space available on modern MCUs to provide everything all at once, with only major system (or static linked component) changes requiring a sketch modify/re-upload.
* Supports equatorial, Alt/Az, and single-axis mount geometries with separate mount geometry and axis-driver hardware.
* Includes built-in astronomical target data for the Messier catalog, useful bright stars, and major solar-system targets.
* Supports covers, camera shutters, focusers, dew/optics heating, camera cooling, ventilation, and shared power rails.
* Actuator & Sensor pins can be multiplexed or expanded along with any control input pins through 8/16-bit i2c expanders for pin-limited controllers.
* Library data can be built into onboard Flash or exported onto external storage to additionally save on compiled sketch size.

Made primarily for Arduino microcontrollers / build environments, but should work with PlatformIO, Espressif, Teensy, STM32, Pico, and others - although one might experience turbulence until the bug reports get ironed out.

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## About

We want to make astronomical trackers more accessible to DIY'ers by utilizing the widely-available low-cost IoT and IoT-like microcontrollers (MCUs) of today.

With the advances in miniaturization technology bringing us even more compact MCUs at even lower costs, it becomes a lot more possible to use one of these small devices to continuously resolve target positions, move telescope axes, coordinate attached equipment, and monitor environmental conditions. Astronomical tracking is a strong application for these devices, especially as a data logger, process monitor, and observatory controller. Professional controller systems can cost hundreds to even thousands of dollars, but DIY systems can wind up being a fraction of that cost.

Astruino is a MCU-based solution primarily written for Arduino and Arduino-like MCU devices. It allows one to combine hobbyist stepper/servo hardware, telescope mounts, cameras, focusers, environmental sensors, heaters, relays, and other widely available low-cost hardware into a functional DIY astronomical tracking system. Be it a converted manual mount, 3D printed tracker, backyard telescope, roll-off roof, or dome shutter, Astruino leaves the physical implementation open to the builder while providing the common automation and tracking layer.

## Controller Setup

### MCU Requirements

There is no single minimum MCU for every Astruino build because enabled UI, networking, logging, target-data storage, and object counts change the program and memory requirements considerably.

As a practical starting point:

Minimum planning target: 256–512kB Flash, 16–24kB SRAM, 16MHz+

Recommended: 512kB–1MB+ Flash, 24–32kB+ SRAM, 32–48MHz+

Modern 32-bit boards such as Pico RP2040/RP2350, ESP32, Teensy 3.5+, STM32, GIGA, and Portenta-class devices are the natural starting point when tracking, logging, UI, and networking are expected to run together.

Astronomy calculations use floating-point math regularly. Motor timing, encoder feedback, display load, and communication traffic can matter more than Flash size alone when selecting the MCU.

### Installation

The easiest way to install this controller is to utilize the Arduino IDE library manager, or through a package manager such as PlatformIO. Otherwise, simply download this controller and extract its files into a `Simple-AstroTracker-Arduino` folder in your Arduino custom libraries folder, typically found in your `[My ]Documents\Arduino\libraries` folder (Windows), or `~/Documents/Arduino/libraries/` folder (Linux/OSX).

From there, you can make a local copy of one of the example sketches based on the kind of system setup you want to use. If you are unsure of which, we recommend the Simple Equatorial Example, as it is the smallest complete tracker example and provides a straightforward starting point for mount and axis-driver setup.

Storage constrained MCUs (< 512kB Flash, particularly <= 256kB) may need further setup file/max-sizes tweaking, and possibly external storage hardware (such as EEPROM or SD Card - see the Data Writer example for more details). Modern MCUs with lots of Flash storage can instead use the larger tracking, scheduling, camera, thermal, and UI features together.

### Target Data and Tracking

Astruino includes built-in astronomical target data for all 110 Messier objects, useful bright-star targets, and the Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus, and Neptune.

Fixed targets use stored J2000 coordinates and are resolved for the requested UTC time. Moving solar-system targets are time-dependent. Mount geometry and physical motor control remain separate so the same target can be tracked through equatorial, Alt/Az, or single-axis configurations using the appropriate axis drivers.

### Host Tests

Core logic and source-data checks can be run without an Arduino connected:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

### Setup

#### Header Defines

There are several defines inside of the controller's main `Astruino[UI].h` header file that allow for more fine-tuned control of the controller. You may edit and uncomment these lines directly, or supply them via custom build flags. While editing the main header file isn't ideal, it is often easiest. Note that editing the controller's main header file directly will affect all projects compiled on your system using those modified controller files.

Alternatively, you may also refer to <https://forum.arduino.cc/index.php?topic=602603.0> on how to define custom build flags manually via modifying the platform[.local].txt file, or with the Arduino CLI (preferred way going forward).

For the older platform.local.txt file override approach, create platform.local.txt alongside platform.txt located in %applocaldata%\Arduino15\packages\{platform}\hardware\{arch}\{version}\ (replacing %applocaldata%\Arduino15 with ~/Library/Arduino15 for OSX, and ~/.arduino15 for Linux), with the contents: `compiler.cpp.extra_flags=-Dname` (replacing `name` with full name of below define). Note that it will affect all builds for that platform until again changed/removed. Some build systems may require directly editing platform.txt and adding onto the end of its CPP build recipe, e.g. Teensy & `recipe.cpp.o.pattern=<bunch-of-stuff> -Dname`.

From Astruino.h:
```Arduino
// Uncomment or -D this define to completely disable usage of any multitasking commands and libraries. Not recommended.
//#define ASTRO_DISABLE_MULTITASKING              // https://github.com/davetcc/TaskManagerIO

// Uncomment or -D this define to disable usage of tcMenu library, which will disable all GUI control. Not recommended.
//#define ASTRO_DISABLE_GUI                       // https://github.com/davetcc/tcMenu

// Uncomment or -D this define to enable usage of the platform WiFi library, which enables networking capabilities.
//#define ASTRO_ENABLE_WIFI                       // https://reference.arduino.cc/reference/en/libraries/wifi/

// Uncomment or -D this define to enable usage of the external serial AT WiFi library, which enables networking capabilities.
//#define ASTRO_ENABLE_AT_WIFI                    // https://github.com/jandrassy/WiFiEspAT

// Uncomment or -D this define to enable usage of the platform Ethernet library, which enables networking capabilities.
//#define ASTRO_ENABLE_ETHERNET                   // https://reference.arduino.cc/reference/en/libraries/ethernet/

// Uncomment or -D this define to enable usage of the Arduino MQTT library, which enables IoT data publishing capabilities.
//#define ASTRO_ENABLE_MQTT                       // https://github.com/256dpi/arduino-mqtt

// Uncomment or -D this define to enable usage of the Adafruit GPS library, which enables GPS capabilities.
//#define ASTRO_ENABLE_GPS                        // https://github.com/adafruit/Adafruit_GPS

// Uncomment or -D this define to enable external data storage (SD card or EEPROM) to save on sketch size. Required for constrained devices.
//#define ASTRO_DISABLE_BUILTIN_DATA              // Disables library data existing in Flash, see DataWriter example for exporting details

// Uncomment or -D this define to enable debug output (treats Serial output as attached to serial monitor, waiting on start for connection).
//#define ASTRO_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable verbose debug output (note: adds considerable size to compiled sketch).
//#define ASTRO_ENABLE_VERBOSE_DEBUG

// Uncomment or -D this define to enable debug assertions (note: adds significant size to compiled sketch).
//#define ASTRO_ENABLE_DEBUG_ASSERTIONS
```

From shared/AstruinoUI.h:
```Arduino
// Uncomment or -D this define to enable usage of the XPT2046_Touchscreen library, in place of the Adafruit FT6206 library.
//#define ASTRO_UI_ENABLE_XPT2046TS               // https://github.com/PaulStoffregen/XPT2046_Touchscreen

// Uncomment or -D this define to enable usage of the StChromaArt LDTC framebuffer capable canvas in place of default U8g2Drawable canvas (STM32/mbed only, note: requires advanced setup)
//#define ASTRO_UI_ENABLE_STCHROMA_LDTC

// Uncomment or -D this define to enable usage of the StChromaArt BSP touch screen interrogator in place of the default AdaLibTouchInterrogator (STM32/mbed only, note: requires advanced setup, see tcMenu_Extra_BspUserSettings.h)
//#define ASTRO_UI_ENABLE_BSP_TOUCH

// Uncomment or -D this define to enable usage of the debug menu
//#define ASTRO_UI_ENABLE_DEBUG_MENU
```

#### External Libraries

Astruino uses the following controller-side libraries depending on the enabled hardware and features:

* **ArduinoJson** for JSON configuration data.
* **ArxContainer** and **ArxSmartPtr** for container and shared-pointer support on Arduino targets.
* **DHT sensor library** and **Adafruit Unified Sensor** for DHT environmental sensors.
* **I2C_EEPROM** for external I2C EEPROM storage.
* **RTClib** and **Time** for RTC and system time handling.
* **SolarCalculator** for offline solar position, sunrise, sunset, and transit calculations.
* **TaskManagerIO**, **IoAbstraction**, and **SimpleCollections** for multitasking and I/O support when multitasking is enabled.
* **Adafruit GPS** when GPS support is enabled.
* **MQTT** when MQTT publishing is enabled.
* **SD** plus the platform SPI/Wire support for local storage and buses.
* **WiFi101**, **WiFiNINA_Generic**, **WiFiEspAT**, or **Ethernet** when the matching optional network path is enabled.

Networking is optional. An offline Astruino system does not need a WiFi, Ethernet, or MQTT library.

#### External UI Libraries

The optional tcMenu UI layer can use the same display and input libraries across the controller family:

* **tcMenu** for the menu, remote-control, and display abstraction layer.
* **Adafruit GFX**, **Adafruit ILI9341**, and **Adafruit ST7735 and ST7789 Library** for supported color displays.
* **Adafruit FT6206**, **Adafruit TouchScreen**, and optional **XPT2046_Touchscreen** for touch input.
* **LiquidCrystalIO** for character LCD displays.
* **U8g2** for monochrome OLED and LCD displays.
* **TFT_eSPI** for supported advanced TFT configurations.
* **tcUnicodeHelper** for Unicode-capable tcMenu display paths.

* **U8g2** custom display setups use the selected U8g2 device class and are statically linked to that display configuration.
* **TFT_eSPI** uses its `TFT_eSPI\User_Setup.h` configuration and therefore requires a rebuild when that hardware setup changes.
* **BSP LCD / BSP Touch** support can use the included ChromaArt/BSP adapter layer on supported STM32/mbed targets. This is an advanced hardware-specific path.
* **ST7789 custom TFT / TFT_eSPI** setups use statically configured screen dimensions and require a rebuild when those values change.

### Initialization

There are several initialization mode settings exposed through this controller that are used for more fine-tuned control.

#### Class Instantiation

The controller's class object must first be instantiated, commonly at the top of the sketch where pin setups are defined (or exposed through some other mechanism), which makes a call to the controller's class constructor. The constructor allows one to set the module's various devices and how they are connected, with defaults providing no device specified.

From Astruino.h, in class Astruino:
```Arduino
    // Controller constructor. Typically called during class instantiation, before setup().
    Astruino(pintype_t piezoBuzzerPin = -1,                         // Piezo buzzer pin, else -1
             Astro_EEPROMType eepromType = Astro_EEPROMType_None,   // EEPROM device type/size, else None
             DeviceSetup eepromSetup = DeviceSetup(),               // EEPROM device setup (i2c only)
             Astro_RTCType rtcType = Astro_RTCType_None,            // RTC device type, else None
             DeviceSetup rtcSetup = DeviceSetup(),                  // RTC device setup (i2c only)
             DeviceSetup sdSetup = DeviceSetup(),                   // SD card device setup (spi only)
             DeviceSetup netSetup = DeviceSetup(),                  // Network device setup (spi/uart)
             DeviceSetup gpsSetup = DeviceSetup(),                  // GPS device setup (uart/i2c/spi)
             pintype_t *ctrlInputPins = nullptr,                    // Control input pins, else nullptr
             DeviceSetup displaySetup = DeviceSetup());             // Display device setup (i2c/spi)
```

#### Controller Initialization

Additionally, a call is expected to be provided to the controller class object's `init[From…](…)` method, commonly called inside of the sketch's `setup()` function. This allows one to set the controller's system type (Tracking, Balancing, or Manual), units of measurement (Metric, Imperial, or Scientific), control input mode, and display output mode. The default mode of the controller, if left unspecified, is a Tracking system set to Metric units, without any input control or output display.

From Astruino.h, in class Astruino:
```Arduino
    // Initializes default empty system. Typically called near top of setup().
    // See individual enums for more info.
    void init(Astro_SystemMode systemMode = Astro_SystemMode_Tracking,                  // What astronomy operating mode should be used
              Astro_MeasurementMode measureMode = Astro_MeasurementMode_Default,        // What units of measurement should be used
              Astro_DisplayOutputMode dispOutMode = Astro_DisplayOutputMode_Disabled,   // What display output mode should be used
              Astro_ControlInputMode ctrlInMode = Astro_ControlInputMode_Disabled);     // What control input mode should be used

    // Initializes system from EEPROM save, returning success flag
    // Set system data address with setSystemEEPROMAddress
    bool initFromEEPROM(bool jsonFormat = false);
    // Initializes system from SD card file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool initFromSDCard(bool jsonFormat = true);
#ifdef ASTRO_USE_WIFI_STORAGE
    // Initializes system from a WiFiStorage file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool initFromWiFiStorage(bool jsonFormat = true);
#endif
    // Initializes system from custom JSON-based stream, returning success flag
    bool initFromJSONStream(Stream *streamIn);
    // Initializes system from custom binary stream, returning success flag
    bool initFromBinaryStream(Stream *streamIn);
```

The controller can also be initialized from a saved configuration, such as from an EEPROM or SD card, or other JSON or Binary stream. A saved configuration of the system can be made via the controller class object's `saveTo…(…)` methods, or called automatically on timer by setting an Autosave mode/interval.

From Astruino.h, in class Astruino:
```Arduino
    // Saves current system setup to EEPROM save, returning success flag
    // Set system data address with setSystemEEPROMAddress
    bool saveToEEPROM(bool jsonFormat = false);
    // Saves current system setup to SD card file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool saveToSDCard(bool jsonFormat = true);
#ifdef ASTRO_USE_WIFI_STORAGE
    // Saves current system setup to WiFiStorage file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool saveToWiFiStorage(bool jsonFormat = true);
#endif
    // Saves current system setup to custom JSON-based stream, returning success flag
    bool saveToJSONStream(Stream *streamOut, bool compact = true);
    // Saves current system setup to custom binary stream, returning success flag
    bool saveToBinaryStream(Stream *streamOut);
```

### Event Logging & Data Publishing

The controller can, after initialization, be set to produce logs and data files that can be further used by other applications. Log entries are timestamped and can keep track of when targets are acquired, when devices enable/disable, scheduler stage changes, etc., while data files can be read into plotting applications or exported to a database for further processing. The passed file prefix is typically the subfolder that such files should reside under and is appended with the year, month, and date (in YYMMDD format).

Note: You can also get the same logging output sent to the Serial device by defining `ASTRO_ENABLE_DEBUG_OUTPUT`, described above in Header Defines.

Note: Files on FAT32-based SD cards are limited to 8 character file/folder names and a 3 character extension.

From Astruino.h, in class Astruino:
```Arduino
    // Enables system logging to the SD card. Log file names will append YYMMDD.txt to the specified prefix. Returns success flag.
    inline bool enableSysLoggingToSDCard(String logFilePrefix);

    // Enables data publishing to the SD card. Data file names will append YYMMDD.csv to the specified prefix. Returns success flag.
    inline bool enableDataPublishingToSDCard(String dataFilePrefix);
```

## Hookup Callouts

Many of the various electronic components and systems this controller is designed to work with may have specific setup procedures and/or wiring requirements. While advanced users may find this section a refresher at best, the below callouts are highlighted in order to help prevent device damage and ensure proper controller operation.

### General

* The recommended Vcc power supply and logic level is 5v, with most newer MCUs restricted to 3.3v.
  * There are many devices that are 3.3v only and not 5v tolerant. Check your IC's datasheet for details.
* 5v device output pins that interface with any 3.3v device input pins that are not 5v tolerant (such as a 5v AVR interfacing with a 3.3v-only [serial ESP-AT WiFi module](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/), or a 3.3v MCU interfacing with a 5v analog sensor), will require a bi-directional logic level converter/shifter to use, especially for any high-speed digital data transfer lines.
  * Alternatively, using a 10kΩ resistor can often times be enough to 'convert' 5v to 3.3v, but the correct way is to utilize a 1kΩ resistor and a 2kΩ resistor (or any size with a 1:2 ratio) in a [simple voltage divider circuit](https://randomnerdtutorials.com/how-to-level-shift-5v-to-3-3v/).

### Serial UART

Serial UART uses individual communication lines for each device, with the receive `RX` pin of one being the transmit `TX` pin of the other - thus having to "flip wires" when connecting. However, devices can always be active and never have to share their access. UART runs at low to mid kHz speeds and is useful for simple device control, albeit somewhat clumsy at times.

* When wiring up modules that use Serial UART, make sure to flip `RX`/`TX` pins.
* Always ensure that any data output pins and data input pins have compatible voltages.

Serial UART Devices Supported: Bluetooth-AT modules, ESP-AT WiFi modules, NMEA-AT GPS modules

### SPI Bus

SPI devices can be chained together on the same shared data lines, which are typically labeled `COPI` (or `MOSI`), `CIPO` (or `MISO`), and `SCK`, often with an additional `CS` (or `SS`). Each SPI device requires its own individual cable-select `CS` wire as only one SPI device may be active at any given time - accomplished by pulling its `CS` line of that device low (aka active-low). SPI runs at MHz speeds and is useful for large data block transfers.

* The `CS` pin may be connected to any digital output pin, but it's common to use the `CS` (or `SS`) pin for the first device. Additional devices are not restricted to what pin they can or should use, but given it's not a data pin not using a choice interrupt-capable pin allows those to be used for interrupt driven mechanisms.
* Many low-cost SPI-based SD card modules on market only read SDHC sized SD cards (2GB to 32GB) formatted in FAT32 (filenames limited to 8 characters plus 3 character file extension).
  * Some SD cards simply will not play nicely with these modules and you may have to try another SD card manufacturer. We recommend 32GB SD cards due to overall lowest cost (smaller SD cards actually becoming _more_ expensive).
* Many various graphical displays may have an additional `DC` (or `RS`) pin, which is required to be connected to any open digital pin in addition to its `CS` pin.
  * There is often an additional `Reset` (or `RST`) pin that needs either wired to an open digital pin for MCU control, otherwise typically will need hard-tied to a HIGH signal (such as that from `Vcc`) in order for the display to function/turn-on.
  * There is also often an additional `LED` (or `BL`) pin that controls the backlight that can be either optionally wired to an open digital or analog pin for MCU control, otherwise can be hard-tied typically to a HIGH signal (such as that from `Vcc`) in order to stay always-on, or simply left disconnected for device default.
* Always ensure that any data output pins and data input pins have compatible voltages.

SPI Devices Supported: SD card modules, NMEA GPS modules, 128x128+ LCD/OLED/TFT graphical displays, XPT2046 touchscreens

### I2C Bus

I2C (aka I²C, IIC, TwoWire, TWI) devices can be chained together on the same shared data lines (no flipping of wires), which are typically labeled `SCL` and `SDA`. Only different kinds of I2C devices can be used on the same data line together using factory default settings, otherwise manual addressing must be performed. I2C runs at mid to high kHz speeds and is useful for advanced device control.

* When more than one I2C device of the same kind is to be used on the same data line, each device must be set to use a different address. This is accomplished via the A0-A2 (sometimes A0-A5) pins/pads on the physical device that must be set either open or closed (typically via a de-solderable resistor, or by shorting a pin/pad). Check your specific breakout's datasheet for details.
* Note that not all the I2C libraries used support multi-addressable I2C devices at this time (read as: may only use one). Currently, this restriction applies to: RTC devices.
* Always ensure that any data output pins and data input pins have compatible voltages.

I2C Devices Supported: DS*/PCF* RTC modules, AT24C* EEPROM modules, NMEA GPS modules, 16x2/20x4 LCD displays, 128x32/128x64 OLED displays, FT6206 touchscreens, 8/16-bit pin expanders

### OneWire Bus

OneWire devices can be chained together on the same shared data lines (no flipping of wires). Devices can be of the same or different types, require minimal setup (and often no soldering), and most can even operate in "parasite" power mode where they use the power from the data line (and an internal capacitor) to function (thus saving a `Vcc` line, only requiring `Data` and return `GND`). OneWire runs only in the low kb/s speeds and is useful for light-weight digital sensors.

* Typically, sensors are limited to 20 devices along a maximum 100m of wire.
* When more than one OneWire device is on the same device line, each device registers itself an enumeration index (0 - N) along with its own 64-bit unique identifier (UUID, with last byte being CRC). The device can then be referenced via this UUID by the system in the future indefinitely, or enumeration index so long as the device doesn't change its line position.
* Always ensure that any data output pins and data input pins have compatible voltages.

OneWire Devices Supported: DHT* 1W air temp/humidity sensors

### Analog IO

* All analog sensors will need to have the same operational voltage range as the controller supports. Many analog sensors are set to use 0v to 5v by default, but some can go -5v to +5v, some even up to 5.5v.
  * Note: Altering default factory calibration settings may require addition tools for setting up a new calibration, such as special calibration fluids/procedures/etc. Refer to the datasheet of your device for details.
* The `AREF` (or `IOREF`) pin, which controls the upper-bound of this range, by default if left not-connected (NC) is the same voltage as the MCU. Analog sensors must not exceed this voltage limit.
  * 5v analog sensor output signals connecting to 3.3v MCUs that are not 5v tolerant **must** either be: [level converted](https://randomnerdtutorials.com/how-to-level-shift-5v-to-3-3v/) in order to connect, or configured to output 0v to `AREF` (or `IOREF`) in voltage calibration output range (if able to calibrate - see note above).
  * Warning: Too high of applied voltage to any pin incapable of receiving such high a voltage risks permanent damage to that device. _Always_ ensure that the applied voltage level coming out a device is supported when going back into another. Some breakouts/IC's have 5v tolerance built-in, some do not. Refer to the datasheet of your MCU/device for details.
  * Note: Typically a 3.3v output signal will _not_ need level converted up to 5v for a 5v digital input to operate (read as: 3.3v is plenty enough to trigger HIGH on 5v device inputs).
* The SAM/SAMD family of MCUs (e.g. Due, Zero, MKR, Nano 33, etc.) as well as many more modern MCUs support different bit resolutions for analog/PWM pins (tied to overridable `DAC_RESOLUTION` & `ADC_RESOLUTION` defines), with some (e.g. Pico, ESP32, etc.) supporting any pin being digital or analog w/o restriction. Refer to the datasheet of your MCU for details.

### Sensors

* Many different kinds of hobbyist sensors label their analog output `AO` (or `Ao`) - however, always check your specific sensor's datasheet, as some may have non-standard pin designations.
  * Again, make sure all analog sensors are calibrated to output the same 0v - `AREF` (or `IOREF`) volts in range.
* Sensor pins used for event triggering when measurements go above/below a pre-set tolerance - many of which are deceptively labeled `DO` (or `Do`), despite having nothing to do with being `D`ata lines of any kind - can be safely ignored, as the software implementation of such mechanism is more than sufficient.
  * Often these connections are used to drive other hardware-only based solutions that aren't a part of Astruino's use case, but can still be connected up using an `AstroDigitalSensor` that triggers upon specific conditions, possibly using an ISR-capable pin if desired.
  * `AstroDigitalSensor` state changes use a configurable stable-time filter before a new level is accepted. The default is 100ms. Use `setStateStableTime()` to adjust it, or set the stable time to 0 to disable the filter.

### STEP/DIR Motors

* Confirm the driver's STEP/DIR/ENABLE logic levels and polarity.
* Calculate `stepsPerDegree` from motor steps, microstepping, gearing, and final axis reduction.
* Begin with a conservative maximum step rate and verify actual motion direction before enabling unattended tracking.
* High-performance motion may be better delegated to a dedicated stepper library/controller through the callback driver.

### Covers and Roofs

* Use open/closed limit switches where over-travel can damage the mechanism.
* Use independent physical interlocks when a roof or shutter could collide with the telescope.
* Verify the park position physically before allowing automatic closure.

### Weather Inputs

Weather sensors can help make an automated system safer, but a hobby controller should not be the only protection for valuable equipment in severe weather. Validate sensor failure modes as well as normal readings.

### Networking & Wireless

* Networking of any kind is 100% optional. Base controller operation works offline using an RTC and optional GPS or known static location.
  * WiFi or Ethernet can be enabled when remote control, MQTT, network time, or network storage is wanted.
* Devices with built-in WiFi or Ethernet can enable such through header/build defines while other devices can utilize an external [serial ESP WiFi module](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/) on any open Serial line.
  * Warning: While WiFi password is encrypted into system settings data, it should not be considered secure.
* Serial Bluetooth-AT modules can be used on any open Serial port to provide remote device control (only).
* MQTT requires remotely accessible broker daemon in order to publish sensor data (setup separately).
* UDP time server requires remotely accessible time & date API service in order to sync time (TODO).
  * RTC not required / used in reserve when UDP service enabled.
* Note: Geo-location APIs require external 3rd party monthly subscription fees, thus isn't included as a feature.

## Memory Callouts

* The total number of objects and different kinds of objects (mounts, targets, actuators, sensors, rails, etc.) that the controller can support at once depends on how much free Flash storage and SRAM your MCU has available.
  * For our supported microcontroller range, on the low end we have devices with 256kB of Flash and at least 16kB of SRAM, while on the upper end we have more modern devices with 1MB+ of Flash and 32kB+ of SRAM. Devices with < 24kB of SRAM may struggle with system builds and may be limited to minimal system setups (such as no WiFi, no data publishing, no built-in library data, only minimal-to-no GUI, etc.), while other newer devices with more capacity build with everything enabled.
* For AVR, SAM, and other build architectures that do not have C++0x11 STL (standard container library) support, there are a series of *`_MAXSIZE` defines nearer to the top of `Astro[UI]Defines.h` that can be modified to adjust how much memory space is allocated for the various static array structures the controller instead uses.
* To save on the cost of code size for constrained devices, focus on not enabling that which you won't need, which has the benefit of being able to utilize code stripping to remove sections of code that don't get used.
  * There are also header defines that can strip out certain libraries and functionality, such as ones that disable the GUI, multi-tasking subsystems, etc.
* To further save on code size cost, see the Data Writer Example on how to externalize library data onto an SD Card or EEPROM.
  * Note: Upgrading between versions or changing custom/program data may require you to re-build and re-deploy to such external device.

## Mounts and Axis Drivers

`AstroMount` supports:

* `Astro_MountType_Equatorial`
* `Astro_MountType_AltAzimuth`
* `Astro_MountType_SingleAxis`

Each mount tracks current and target position, configured rate, optional software limits, park position, guide offsets, and optional axis-driver feedback.

Mount geometry remains separate from the motor implementation. Driver paths include:

* `AstroCallbackAxisDriver` for adapting an existing motor library or external controller.
* `AstroServoAxisDriver` for normalized servo-style positioning.
* `AstroStepDirAxisDriver` for ordinary STEP/DIR hardware advanced from the normal controller update loop.

If a driver supplies position feedback through `getPositionDegrees()`, that reading updates the mount axis state. If feedback is unavailable, Astruino can maintain an internal rate-limited position estimate toward the target.

## Covers, Cameras, and Focusers

Each mount owns an `AstroCover` and `AstroCamera` sub-object.

`AstroCover` handles a generic open/close mechanism with optional open/closed sensors, a travel estimate, and movement timeout protection. Physical hard limits and collision interlocks should still be used where mechanism damage is possible.

`AstroCamera` provides shutter timing for interval and exposure workflows and participates in scheduler readiness/stow sequencing.

`AstroFocuser` is a registered actuator object providing absolute and relative position control with configurable limits and optional position feedback callbacks.

## Sensors, Measurements, and Triggers

Astruino sensor readings use the controller's measurement and polling-frame model.

Sensor classes include value, callback, digital, and analog sensor paths. Digital sensors support minimum-stable-time filtering, while analog sensors can use user calibration data to convert raw input into engineering units.

Measurement value/range triggers convert sensor readings into state with configurable de-trigger tolerance and optional de-trigger delay. The trigger reports the condition; the mount, scheduler, or application code decides what action follows.

## Thermal Control

Each mount owns an `AstroThermalBalancer`.

The thermal layer can coordinate ambient temperature, humidity/dew-point information, optics heating, electronics heating, camera cooling, and fan output.

## Scheduler

`AstroScheduler` automatically coordinates registered mounts across the observing night.

The tracking stages are:

1. `Init`
2. `Warm`
3. `Deploy`
4. `Acquire`
5. `Track`
6. `Stow`

Storm triggers force stow behavior. Cover closure should only occur after the mount is parked and attached equipment reports a safe state.

Software sequencing is not a replacement for hard limits, collision interlocks, fusing, emergency controls, or weather-safe mechanical design.

## Targets and Catalog Data

`astroTargetsLib` is the global `AstroTargetsLibrary` instance.

Target records use checkout/return reference counting so the entire expanded catalog does not have to remain resident in SRAM. `AstroStaticTarget` resolves stored J2000 positions with precession, while `AstroDynamicTarget` represents time-dependent solar-system targets.

The target library can use built-in Flash data or external SD-card/EEPROM data when built-in data is disabled or custom storage is preferred.

## Power Rails

Power rails model shared software capacity; they are not electrical protection.

`AstroSimpleRail` limits the number of linked actuators allowed to be active at once.

`AstroRegulatedRail` uses a maximum-power setting, optional power-usage sensor, and limit trigger to decide whether another actuator can be activated.

Use real fuses, suitable conductors, current limiting, and properly rated power hardware regardless of the software rail model.

## Example Usage

Below are several examples of controller usage.

### Simple Equatorial Mount System Example

The Simple Equatorial Example shows how a basic Astruino system can be setup using a two-axis equatorial mount and ordinary STEP/DIR axis drivers. In this sketch only that which you actually use is built into the final compiled binary, making it an ideal lean choice for a straightforward tracker.

```Arduino
#include <Astruino.h>

Astruino astroController;

void setup()
{
    astroController.init();
    astroController.setSystemLocation(49.2827, -123.1207, 70.0);

    auto mount = astroController.addMount(Astro_MountType_Equatorial);

    auto primaryDriver = astroController.addMountAxisStepper(
        2, 3, 4,
        1280.0,
        4000.0);

    auto secondaryDriver = astroController.addMountAxisStepper(
        5, 6, 7,
        1280.0,
        4000.0);

    if (mount) {
        mount->setAxisDriver(0, primaryDriver);
        mount->setAxisDriver(1, secondaryDriver);
        mount->setTarget(Astro_TargetType_M42);
        mount->setParkPosition(0.0, 0.0);
    }

    astroController.launch();
}

void loop()
{
    astroController.update();
}
```

### Main System Examples

The supplied examples focus on different parts of an astronomical tracking system. New users should start with the Simple Equatorial Example above, then use the other examples as focused references when adding more controller features.

* **SimpleEquatorial** - Basic equatorial mount and axis-driver setup.
* **NightSession** - Scheduler-driven observing session behavior.
* **ThermalCamera** - Camera and thermal-control integration.
* **AstroLibLookup** - Astronomical target-library lookup and coordinate resolution.
* **DataWriter** - Target and string data export.

### Data Writer Example

The Data Writer Example can be used to offload exportable target and string data for external storage workflows and to help storage constrained MCUs compile system builds.

This Example doesn't run the Astruino controller in full. Instead, it exports the built-in target records as compact JSON along with the library strings used by the controller.

Astruino can operate with the target data kept in Flash. External storage is optional and is mainly useful when program space matters or custom target data is desired.

Note: Again, you can get logging output sent to the Serial device by defining `ASTRO_ENABLE_DEBUG_OUTPUT`, described above in Header Defines.
