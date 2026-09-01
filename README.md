# Astruino
Astruino: Simple Astro Tracker Automation Controller.

**Simple-AstroTracker-Arduino v0.7.2.0**

Simple automation controller for DIY astronomical tracking systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

This controller manages mounts, axis drivers, covers, cameras, focusers, environmental sensors, thermal control, targets, scheduling, logging, publishing, and persistent configuration for home-built astronomical tracking systems. The actual mechanical and electrical implementation remains open to the builder, while Astruino provides the common controller, object, attachment, scheduling, and persistence layers.

Our Keep-It-Simple controller system:

* Can be used entirely offline with an RTC module and optional GPS module (or known static location) for accurate time keeping and astronomical positioning, or used online through enabled on-board WiFi/Ethernet or an external ESP-AT WiFi module.
  * Astronomical tracking uses the system UTC time and location to resolve sidereal time, target position, and mount geometry without requiring a network connection.
* Exportable system configuration to EEPROM, SD card, or WiFiStorage external storage device.
  * Saved in pretty-print JSON for human-readability & easy text editing, or in raw binary for compactness & speed.
  * Auto-save and fallback save modes are available through the controller data model.
* Supports interval-based sensor data publishing and system event logging to an MQTT IoT broker or to external storage in .csv/.txt format (/w date in filename, segmented daily).
  * Network publishing is optional; local scheduling, tracking, logging, and target lookup do not require an internet connection.
* Uses registered system objects plus lightweight attachments and mount-owned subobjects.
  * Registered object families include actuators, sensors, targets, mounts, and power rails.
  * Covers, cameras, thermal balancing, triggers, and axis drivers remain attached or mount-owned helpers instead of parallel top-level object families.
* Supports equatorial, Alt/Az, and single-axis mount geometries.
  * Mount geometry remains separate from motor hardware so STEP/DIR, servo, callback, or other compatible driver implementations can be used.
* Includes a compact astronomical target library.
  * Built-in data includes all 110 Messier objects, useful bright-star targets, and major solar-system targets.
  * Fixed targets use stored J2000 coordinates while moving solar-system targets are resolved for the requested UTC time.
* Supports common small-observatory equipment.
  * Covers, camera shutters, focusers, dew/optics heating, camera cooling, and ventilation can participate in the same scheduler and safety sequencing.
* Actuator & Sensor pins can be multiplexed or expanded along with control input pins through supported I/O abstraction hardware where appropriate.
* Library/target data can remain built into onboard Flash or be exported onto external storage to save compiled sketch size.

Made primarily for Arduino microcontrollers / build environments, but intended to fit PlatformIO, Espressif, Teensy, STM32, Pico/RP2040/RP2350, GIGA, Portenta, and similar MCU platforms. Smaller boards may require GUI, networking, debug output, or built-in data to be trimmed.

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## About

We want to make astronomical tracking and small observatory automation more accessible to DIY'ers by utilizing the widely-available low-cost IoT and IoT-like microcontrollers (MCUs) of today.

With modern MCUs providing useful timers, floating-point performance, storage, communications, and large numbers of I/O pins at low cost, it becomes practical to build capable telescope and observatory controllers without committing to one fixed mechanical design or expensive proprietary controller. Astruino is intended for garage-built trackers, converted manual mounts, 3D-printed mechanisms, small backyard telescopes, roll-off roofs, dome shutters, and similar DIY astronomy projects.

Astruino is a MCU-based solution primarily written for Arduino and Arduino-like MCU devices. The controller owns shared system services while mounts and other equipment are created through the same object/factory model used by the sibling controller libraries. Mount geometry, target calculations, attached equipment, scheduler behavior, and physical motor hardware remain separated so the same controller design can be reused across very different builds.

## Controller Setup

### MCU Requirements

There is no single minimum MCU for every Astruino build because enabled UI, networking, logging, target-data storage, and object counts change the program and memory requirements considerably.

As a practical starting point:

Minimum planning target: 256–512kB Flash, 16–24kB SRAM, 16MHz+

Recommended: 512kB–1MB+ Flash, 24–32kB+ SRAM, 32–48MHz+

Modern 32-bit boards such as Pico RP2040/RP2350, ESP32, Teensy 3.5+, STM32, GIGA, and Portenta-class devices are the natural starting point when tracking, logging, UI, and networking are expected to run together.

Astronomy calculations use floating-point math regularly. Motor timing, encoder feedback, display load, scheduler activity, and communication traffic can matter more than Flash size alone when selecting the MCU.

### Installation

The easiest way to install this controller is to utilize the Arduino IDE library manager when available, or through a package manager such as PlatformIO. Otherwise, simply download this controller and extract its files into a `Simple-AstroTracker-Arduino` folder in your Arduino custom libraries folder, typically found in your `[My ]Documents\Arduino\libraries` folder (Windows), or `~/Documents/Arduino/libraries/` folder (Linux/OSX).

From there, make a local copy of one of the supplied example sketches based on the kind of system setup you want to use. If you are unsure of which, start with the Simple Equatorial Example because it demonstrates the normal controller lifecycle and mount/axis-driver relationship with the least surrounding equipment.

The current example set includes:

* **SimpleEquatorial** - Basic equatorial mount and axis-driver setup.
* **AstroLibLookup** - Astronomical target-library lookup and coordinate resolution.
* **NightSession** - Scheduler-driven observing session behavior.
* **ThermalCamera** - Camera and thermal-control integration.
* **DataWriter** - External library/target-data export workflows.

Storage constrained MCUs (< 512kB Flash, particularly <= 256kB) may need further setup file/max-size tweaking and possibly external storage hardware such as EEPROM or SD card. Modern MCUs with more Flash and SRAM are strongly preferred when tracking, GUI, logging, networking, and several equipment objects are expected to operate together.

### Target Data and Tracking

Astruino keeps target selection separate from mount geometry and physical motor control.

Fixed catalog targets are stored using J2000 coordinates and are resolved for the requested UTC time. Moving solar-system targets are calculated for the requested time instead of being treated as fixed catalog positions. A mount can select a target through `setTarget(Astro_TargetType)` and performs the necessary target-data checkout internally during tracking.

A useful mental model is:

**Target + UTC Time + System Location → Mount Geometry → Axis Driver → Physical Motion**

and, alongside it:

**Sensors → Measurements → Triggers / Thermal Logic → Actuators**

The scheduler coordinates those pieces across the observing night.

### Host Tests

Core logic can be run without telescope hardware connected:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

Host tests are useful for astronomy math, object behavior, scheduling, serialization, measurements, and regression coverage. They cannot validate motor polarity, real gear ratios, backlash, mechanical limits, cover travel, wiring, or weather behavior.

### Setup

#### Header Defines

There are several defines inside of the controller's main `Astruino.h` header file that allow for more fine-tuned control of the controller. You may edit and uncomment these lines directly, or supply them via custom build flags. While editing the main header file isn't ideal, it is often easiest. Note that editing the controller's main header file directly will affect all projects compiled on your system using those modified controller files.

Alternatively, you may also refer to <https://forum.arduino.cc/index.php?topic=602603.0> on how to define custom build flags manually via modifying the `platform[.local].txt` file, or with the Arduino CLI (preferred way going forward).

For the older `platform.local.txt` file override approach, create `platform.local.txt` alongside `platform.txt` located in `%applocaldata%\Arduino15\packages\{platform}\hardware\{arch}\{version}\` (replacing `%applocaldata%\Arduino15` with `~/Library/Arduino15` for OSX, and `~/.arduino15` for Linux), with the contents: `compiler.cpp.extra_flags=-Dname` (replacing `name` with full name of below define). Note that it will affect all builds for that platform until again changed/removed. Some build systems may require directly editing `platform.txt` and adding onto the end of its CPP build recipe, e.g. Teensy & `recipe.cpp.o.pattern=<bunch-of-stuff> -Dname`.

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

The controller's class object must first be instantiated, commonly at the top of the sketch where pin setups are defined. The constructor configures controller-level devices and interfaces, with defaults providing no optional device specified.

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

Additionally, a call is expected to be provided to the controller class object's `init[From…](…)` method, commonly called inside of the sketch's `setup()` function. This allows one to set the controller's system mode, units of measurement, control input mode, and display output mode.

From Astruino.h, in class Astruino:
```Arduino
    // Initializes default empty system. Typically called near top of setup().
    // See individual enums for more info.
    void init(Astro_SystemMode systemMode = Astro_SystemMode_Tracking,
              Astro_MeasurementMode measureMode = Astro_MeasurementMode_Default,
              Astro_DisplayOutputMode dispOutMode = Astro_DisplayOutputMode_Disabled,
              Astro_ControlInputMode ctrlInMode = Astro_ControlInputMode_Disabled);

    bool initFromEEPROM(bool jsonFormat = false);
    bool initFromSDCard(bool jsonFormat = true);
#ifdef ASTRO_USE_WIFI_STORAGE
    bool initFromWiFiStorage(bool jsonFormat = true);
#endif
    bool initFromJSONStream(Stream *streamIn);
    bool initFromBinaryStream(Stream *streamIn);
```

The controller can also be initialized from a saved configuration, such as from EEPROM or SD card, or another JSON/Binary stream. A saved configuration can be made through the matching `saveTo…(…)` methods or through configured autosave behavior.

A normal lifecycle is:

```Arduino
Astruino astroController;

void setup()
{
    astroController.init();
    astroController.setSystemLocation(49.2827, -123.1207, 70.0);

    // Add mounts, drivers, sensors, and other equipment here.

    astroController.launch();
}

void loop()
{
    astroController.update();
}
```

### Event Logging & Data Publishing

The controller can, after initialization, produce logs and sensor data that can be used by other applications. Log entries are timestamped and can track scheduler stages, mount/equipment state changes, warnings, and other controller events, while published data can be read into plotting applications or exported to a database for further processing.

Note: The same logging output can also be sent to the Serial device by defining `ASTRO_ENABLE_DEBUG_OUTPUT`, described above in Header Defines.

The controller exposes:

```Arduino
astroController.scheduler;
astroController.logger;
astroController.publisher;
```

Local SD-card logging and publishing are available through:

```Arduino
astroController.enableSysLoggingToSDCard("logs/as");
astroController.enableDataPublishingToSDCard("data/as");
```

WiFiStorage and MQTT publishing are available when the matching feature paths are enabled.

## Mounts and Axis Drivers

`AstroMount` supports:

* `Astro_MountType_Equatorial`
* `Astro_MountType_AltAzimuth`
* `Astro_MountType_SingleAxis`

Each mount tracks current and target position, configured rate, optional software limits, park position, guide offsets, and optional axis-driver feedback.

Mount geometry remains separate from the motor implementation. Current driver paths include:

* `AstroCallbackAxisDriver` for adapting an existing motor library or external controller.
* `AstroServoAxisDriver` for normalized servo-style positioning.
* `AstroStepDirAxisDriver` for ordinary STEP/DIR hardware advanced from the normal controller update loop.

If a driver supplies position feedback through `getPositionDegrees()`, that reading updates the mount axis state. If feedback is unavailable, Astruino can maintain an internal rate-limited position estimate toward the current target.

## Covers, Cameras, and Focusers

Each mount owns an `AstroCover` and `AstroCamera` sub-object.

`AstroCover` handles a generic open/close mechanism with optional open/closed sensors, a travel estimate, and movement timeout protection. Physical hard limits and collision interlocks should still be used where mechanism damage is possible.

`AstroCamera` provides shutter timing for interval and exposure workflows and participates in scheduler readiness/stow sequencing.

`AstroFocuser` is a registered actuator object providing absolute and relative position control with configurable limits and optional position feedback callbacks.

## Sensors, Measurements, and Triggers

Astruino sensor readings use the same measurement/polling-frame model as the sibling controller libraries.

Current sensor classes include value, callback, digital, and analog sensor paths. Digital sensors support minimum-stable-time filtering, while analog sensors can use user calibration data to convert raw input into engineering units.

Measurement value/range triggers convert sensor readings into state with configurable de-trigger tolerance and optional de-trigger delay. The trigger reports the condition; the mount, scheduler, or application code decides what action follows.

## Thermal Control

Each mount owns an `AstroThermalBalancer`.

The thermal layer can coordinate ambient temperature, humidity/dew-point information, optics heating, electronics heating, camera cooling, and fan output. This allows thermal equipment to remain part of the mount's operating state rather than becoming another unrelated scheduler or object hierarchy.

## Scheduler

`AstroScheduler` automatically coordinates registered mounts across the observing night.

Current internal tracking stages are:

1. `Init`
2. `Warm`
3. `Deploy`
4. `Acquire`
5. `Track`
6. `Stow`

Storm triggers force stow behavior. Cover closure should only occur after the mount is parked and attached equipment reports a safe state.

Software sequencing is not a replacement for hard limits, collision interlocks, fusing, emergency controls, or weather-safe mechanical design.

## Targets and Catalog Data

`AstroTargetsLib` is the global `AstroTargetsLibrary` instance.

Target records use checkout/return reference counting so the entire expanded catalog does not have to remain resident in SRAM. `AstroStaticTarget` resolves stored J2000 positions with precession, while `AstroDynamicTarget` represents time-dependent solar-system targets.

The target library can use built-in Flash data or external SD-card/EEPROM data when built-in data is disabled or custom storage is preferred.

## Power Rails

Power rails model shared software capacity; they are not electrical protection.

`AstroSimpleRail` limits the number of linked actuators allowed to be active at once.

`AstroRegulatedRail` uses a maximum-power setting, optional power-usage sensor, and limit trigger to decide whether another actuator can be activated.

Use real fuses, suitable conductors, current limiting, and properly rated power hardware regardless of the software rail model.

## Persistence

Astruino separates live objects from serializable data structures.

System setup can be initialized or saved through:

* EEPROM
* SD card
* WiFiStorage when available
* JSON streams
* Binary streams

Attachments and registered-object data are persisted through the same identity/data approach used by the sibling controller libraries.

## Hookup Callouts

Many of the electronic components and systems this controller is designed to work with have specific setup procedures and wiring requirements. The below callouts are intended to help prevent device damage and ensure reliable controller operation.

### General

* Verify MCU and peripheral logic voltage before connection.
* Do not power motors, heaters, relays, TECs, solenoids, or similar loads directly from MCU pins.
* Use suitable motor drivers, relay/MOSFET interfaces, fusing, grounding, isolation, and power supplies.
* Software axis limits and cover timeouts are secondary protections, not the only protections against mechanical damage.

### Serial UART

* When wiring modules that use Serial UART, connect device TX to controller RX and device RX to controller TX.
* Always ensure that data output/input logic voltages are compatible.

Serial UART devices can include Bluetooth-AT modules, ESP-AT WiFi modules, GPS modules, and application-specific motor/controller interfaces.

### SPI Bus

* Each SPI device normally requires its own `CS`/`SS` line while sharing the main data/clock lines.
* SD-card, display, and other device requirements vary by module and MCU.
* Always ensure that data output/input logic voltages are compatible.

### I2C Bus

* Devices sharing an I2C bus must use non-conflicting addresses.
* Check pull-up voltage and total bus length/capacitance, especially in observatory installations with longer cable runs.
* Long runs may require slower bus speeds, buffering, or differential interfaces.

### OneWire Bus

* OneWire sensors can share a data line when the selected device/library supports the required topology.
* Verify pull-up voltage and cable length for the installation.
* Keep high-current motor/heater wiring away from sensitive sensor lines where practical.

### STEP/DIR Motors

* Confirm the motor driver's STEP/DIR/ENABLE logic levels and polarity.
* Calculate `stepsPerDegree` from motor steps, microstepping, gearing, and final axis reduction.
* Begin with a conservative maximum step rate and verify actual motion direction before unattended tracking.
* High-performance motion may be better delegated to a dedicated stepper controller through the callback driver.

### Covers and Roofs

* Use open/closed limit switches where over-travel can damage the mechanism.
* Use independent physical interlocks when a roof or shutter could collide with the telescope.
* Verify the park position physically before allowing automatic closure.

### Weather Inputs

Weather sensors can improve automated safety, but a hobby controller should not be the only protection for valuable equipment in severe weather. Validate sensor failure modes as well as normal readings.

### Networking & Wireless

* Networking is optional. Base controller operation works offline using an RTC and GPS or known static location.
* WiFi or Ethernet can be enabled when remote control, MQTT, network storage, or other remote functionality is wanted.
* MQTT requires an accessible broker configured separately from Astruino.

## Memory Callouts

* Registered object count, target-data source, GUI, networking, logging, and debug features all affect Flash/SRAM usage.
* `ASTRO_DISABLE_BUILTIN_DATA` supports external target/library-data workflows when Flash is constrained.
* Disabling unused GUI/network/debug functionality allows the compiler to strip code that is not needed.
* Larger 32-bit MCUs are recommended when several equipment and communication subsystems are active together.
* On architectures without normal STL support, the `ASTRO_*_MAXSIZE` values in the defines headers control several fixed-capacity containers and may need tuning for the intended build.

## Example Usage

Below are several examples of controller usage.

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

The supplied examples provide more focused demonstrations of catalog lookup, scheduler-driven sessions, camera/thermal integration, and data export.

### Data Writer Example

The Data Writer Example is intended for builds that need library/target data moved out of onboard Flash. It can be used to prepare supported external data for SD card or EEPROM storage without requiring the full tracker system to be running.

If `ASTRO_DISABLE_BUILTIN_DATA` is used, make sure the corresponding external data has been generated and deployed before relying on target/library lookups.

## License

Astruino is released under the MIT License. See `LICENSE` for details.
