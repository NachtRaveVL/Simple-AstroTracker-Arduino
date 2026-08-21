# Astruino
Astruino: Simple Astro Tracker Automation Controller.

**Simple-AstroTracker-Arduino v0.7.2.0**

Simple automation controller for DIY astronomical tracking systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

This controller manages telescope mounts, motors, servos, covers, focusers, observation devices, environmental sensors, thermal control, scheduling, logging, and data collection for home-built astronomical tracking systems. It is aimed at ordinary Arduino-compatible hardware and common maker parts rather than expensive observatory control equipment.

Our Keep-It-Simple controller system:

* Can be used entirely offline with a reliable clock and known static location, or with optional GPS for automatic time and position.
  * WiFi, Ethernet, MQTT, and remote interfaces are optional and are not required for normal tracking.
* Supports equatorial, Alt/Az, and simple single-axis mount geometry.
  * Includes local sidereal time, Julian date handling, coordinate normalization, J2000 precession, and equatorial-to-horizontal conversion.
  * Mount movement is kept separate from motor hardware so steppers, servos, DC motors, and external controllers can be adapted without changing the astronomy code.
  * Park/unpark handling, per-axis software limits, optional position feedback, pulse guiding, and wrapped Alt/Az azimuth motion are supported by the mount layer.
* Includes `AstroLib` for compact astronomical target data.
  * Includes all 110 Messier objects, a useful set of bright stars, and the Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus, and Neptune.
  * Checked-out target data uses the same load/cache style as the crop library in Hydruino so the full catalog does not need to stay expanded in RAM.
  * Custom target slots and external target loading are supported.
* Supports familiar hobby electronics and Arduino-style I/O.
  * Digital and analog pins, active-low inputs, PWM outputs, pin muxing, callbacks, sensors, actuators, activation handles, triggers, measurements, and power rails follow the same general patterns as the sibling libraries.
  * Optional time and location providers allow RTC, GPS, network, radio, or application-supplied implementations without changing the tracking logic.
* Supports useful small-observatory equipment without assuming a professional installation.
  * `AstroFocuser` supports absolute and relative step positioning with optional feedback.
  * A generic cover can represent a telescope cap, sliding cover, roll-off roof, dome shutter, or another open/closed mechanism.
  * A simple camera trigger can operate a shutter pin, relay, external controller, or another recording device.
* Includes environmental and thermal balancing for nighttime operation.
  * Dew point can be calculated from ambient temperature and humidity.
  * Dew-heater output can maintain optics above the dew point without simply running a heater at full power.
  * Equipment heating, fans, and optional camera cooling can be coordinated through the same thermal state.
* Includes a nighttime scheduler for deployment, observing, and safe stow.
  * Unsafe conditions can interrupt operation and force the system toward a safe state.
  * Safe stow waits for the mount to park before closing the cover.
  * Mount and cover motion faults stop the observation sequence.
* Uses compact Flash-backed string tables and generated enum decoding.
  * Enum import decoding uses a generated minimum-discriminator trie instead of repeatedly comparing full strings.
* Includes system event logging and polling-frame data publishing.
  * Output sinks remain optional so an offline controller does not require networking or external storage.
* Includes the same `full`, `min`, `shared`, display/input, screen, and tcMenu source layout as the related controller libraries.
  * The shared tcMenu adapter layer is present.
  * Astruino-specific menus and overview screens are still TODO work while the astronomy controller and equipment model are established.

Made primarily for Arduino microcontrollers / build environments, but should also fit PlatformIO, Espressif, Teensy, STM32, Pico/RP2040/RP2350, GIGA, Portenta, and similar modern MCU platforms. Smaller boards may still be usable with the catalog, UI, networking, and other optional features trimmed down.

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## About

We want to make astronomical tracking and small observatory automation more accessible to DIY'ers by using the widely available low-cost microcontrollers, sensors, motor drivers, and mechanical parts that are already common in maker projects.

A useful astronomy project does not have to begin with a commercial mount controller or a fully automated observatory. A basic system can be an Arduino-compatible MCU, a clock, known latitude and longitude, one or two motorized axes, and motor drivers. From there, home switches, encoders, a focuser, a cover, environmental sensors, and camera control can be added as the mechanics become more capable.

Astruino is meant to handle the controller logic while leaving the physical build open. A tracker made from 3D printed parts, a converted manual mount, a garage-built roll-off roof, or a small backyard telescope can all use the same astronomy and scheduling layer. The goal is to make it easier to learn the sky, control systems, mechanics, sensors, and embedded programming without requiring the internet or astronomy-specific controller hardware.

## Controller Setup

### MCU Requirements

There is no single minimum MCU for every Astruino build because the target catalog, UI, logging, networking, and enabled support libraries can change program size considerably.

As a practical planning target:

Minimum MCU: 256-512kB Flash, 16-24kB SRAM, 16MHz+  
Recommended: 512kB-1MB+ Flash, 24-32kB+ SRAM, 32-48MHz+

Modern 32-bit boards such as ESP32, RP2040/RP2350, Teensy 3.5+, STM32, GIGA, and Portenta-class devices are the natural starting point. Smaller MCUs may require externalized data, disabled GUI/networking, smaller object counts, and other build-size reductions.

Note: Pin-limited MCUs can use multiplexers or I2C-based expanders where appropriate. Motor timing and encoder requirements may also influence board choice more than Flash size alone.

Note: The astronomy calculations make regular use of floating point math. A board with good 32-bit performance is strongly preferred for multi-axis tracking, ephemeris work, UI, and logging at the same time.

### Installation

The easiest way to install this controller is to use the Arduino IDE library manager when available, or a package manager such as PlatformIO. Otherwise, download the controller and extract its files into a `Simple-AstroTracker-Arduino` folder in the Arduino custom libraries folder, typically found in `[My ]Documents\Arduino\libraries` on Windows or `~/Documents/Arduino/libraries/` on Linux/OSX.

From there, make a local copy of one of the example sketches based on the kind of tracker or observatory setup being built. `SimpleEquatorial` is the best starting point for a basic two-axis mount. `NightSession` shows how the mount, cover, camera, scheduler, logger, and environment pieces fit together.

Storage-constrained MCUs may need built-in data or GUI features disabled. The `DataWriter` example and development export tools show how catalog and string data can be prepared for external-data workflows.

### AstroLib and Tracking Data

Astruino includes a compact target catalog instead of keeping every object expanded in RAM. Fixed target coordinates are stored compactly, while moving solar-system targets resolve through the same `AstroTargetData` interface for the requested time.

The built-in library includes all Messier objects, a useful bright-star set, and major moving solar-system targets. Custom target slots and external loading keep the built-in catalog from becoming a hard limit.

Development/export sketches under `tests/` include:

* `AstroLibExportToCPP` for PROGMEM-ready target catalog entries.
* `EnumTrieExportToCPP` for generated enum string decoding.
* `EnumConversionTests` for round-trip string conversion checks.
* `JSONExportTests` for serializable target and pin data.

### Host Tests

Core logic and source checks can be run without a telescope or Arduino connected:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
python3 tests/validate_source.py
```

The host suite covers astronomy math, precession, moving-body sanity, catalog lookup/cache behavior, mount and cover control, focuser behavior, thermal balancing, sensors, actuators, activation handles, pins, muxing, measurements, triggers, power rails, factory creation, provider interfaces, controller lifecycle, serialization, reporting, enum conversions, and example syntax.

### Setup

#### Header Defines

There are several defines inside of the controller's main `Astruino.h` header file that allow for more fine-tuned control of the controller. They can be edited and uncommented directly, or supplied through custom build flags. Editing the installed header affects every sketch using that library installation, so custom build flags are preferred when practical.

From Astruino.h:

```Arduino
// Uncomment or -D this define to completely disable multitasking commands and libraries.
//#define ASTRO_DISABLE_MULTITASKING

// Uncomment or -D this define to disable tcMenu-based GUI control.
//#define ASTRO_DISABLE_GUI

// Uncomment or -D this define to enable the platform WiFi library.
//#define ASTRO_ENABLE_WIFI

// Uncomment or -D this define to enable serial AT-command WiFi support.
//#define ASTRO_ENABLE_AT_WIFI

// Uncomment or -D this define to enable the platform Ethernet library.
//#define ASTRO_ENABLE_ETHERNET

// Uncomment or -D this define to enable MQTT publishing support.
//#define ASTRO_ENABLE_MQTT

// Uncomment or -D this define to enable GPS-based time/location support.
//#define ASTRO_ENABLE_GPS

// Uncomment or -D this define to disable built-in Flash data and use external data storage.
//#define ASTRO_DISABLE_BUILTIN_DATA

// Uncomment or -D this define to enable serial debug output.
//#define ASTRO_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable verbose debug output.
//#define ASTRO_ENABLE_VERBOSE_DEBUG

// Uncomment or -D this define to enable debug assertions.
//#define ASTRO_ENABLE_DEBUG_ASSERTIONS
```

Networking is opt-in. A build with none of the network defines enabled is a normal supported configuration.

Astruino-specific menu behavior is still TODO work, so the shared UI header does not yet expose the full set of project-specific UI defines found in the older controller libraries.

#### External Libraries

Astruino uses the following controller-side libraries depending on the enabled hardware and features:

* **ArduinoJson** for JSON configuration data.
* **ArxContainer** and **ArxSmartPtr** for container and shared-pointer support on Arduino targets.
* **I2C_EEPROM** for external I2C EEPROM storage.
* **RTClib** and **Time** for RTC and system time handling. The controller synchronizes system time from a configured RTC when one is present; fixed installations may also set TimeLib directly.
* **TaskManagerIO** and **IoAbstraction** for multitasking and I/O support when multitasking is enabled.
* **Adafruit GPS** when GPS-derived time or location is enabled.
* **MQTT** when MQTT publishing is enabled.
* **SD** plus the platform SPI/Wire support for local storage and buses.
* **WiFi101**, **WiFiNINA_Generic**, **WiFiEspAT**, or **Ethernet** when the matching optional network path is enabled.

Networking is optional. Normal telescope tracking does not require a network connection.

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

Astruino-specific menus and overview screens are still TODO work. The shared tcMenu adapter files are already present so the project-specific UI can be built on the same plumbing as Hydruino and Helioduino.

### Initialization

There are several initialization settings exposed through the controller for mount type, system mode, measurement mode, observer location, and optional time/environment providers.

#### Class Instantiation

The controller object is normally instantiated near the top of the sketch. The constructor selects the primary mount geometry.

From Astruino.h, in class Astruino:

```Arduino
Astruino(Astro_MountType mountType = Astro_MountType_Equatorial);
```

For example:

```Arduino
Astruino astroController(Astro_MountType_Equatorial);
```

#### Controller Initialization

A call to `init()` is expected near the top of `setup()`. The default system uses tracking mode and metric measurements.

From Astruino.h, in class Astruino:

```Arduino
void init(Astro_SystemMode systemMode = Astro_SystemMode_Tracking,
          Astro_MeasurementMode measurementMode = Astro_MeasurementMode_Metric);
```

A fixed installation can then provide its observer location directly:

```Arduino
AstroObserver observer(49.2827, -123.1207, 70.0);

astroController.init();
astroController.setObserver(observer);
astroController.launch();
```

The normal `update()` path uses the synchronized system clock. When an RTC is configured, the controller lazily initializes it as the TimeLib synchronization source. Observer location remains separate and can be updated through `setObserver()` by fixed setup code, GPS, or another optional location source.

### Event Logging & Data Publishing

The controller exposes its scheduler, logger, and publisher as public subsystem instances, matching the related controller libraries:

```Arduino
astroController.scheduler;
astroController.logger;
astroController.publisher;
```

`AstroLogger` records system events through an optional sink. `AstroPublisher` gathers sensor measurements into polling-frame columns and emits completed frames through an optional sink. Astruino does not require MQTT or network storage for either subsystem.

## Hookup Callouts

Many of the electronic and mechanical parts used by astronomical trackers have specific wiring and setup requirements. The controller can help coordinate them, but correct electrical interfaces and mechanical limits still have to be provided by the build.

### General

* Check the logic voltage of the MCU, sensors, motor drivers, and external controllers before connecting them.
* Do not power telescope motors, heaters, solenoids, or similar loads directly from MCU pins.
* Use suitable motor drivers, isolated relay interfaces, fusing, and power supplies for the equipment being controlled.
* A software limit should not be treated as the only protection against a mount driving into a hard mechanical stop.

### Serial UART

UART is useful for GPS modules, serial motor controllers, Bluetooth/ESP-AT modules, and other simple peripherals.

* `RX` and `TX` normally cross between the two devices.
* Ensure the signal levels are compatible before connecting 5v and 3.3v equipment.
* Long observatory cable runs may need differential or otherwise more noise-resistant interfaces than raw TTL UART.

### SPI Bus

SPI is commonly used for SD storage, displays, touch controllers, and other higher-speed peripherals.

* Each normal SPI device needs its own `CS` line.
* Keep motor wiring and other electrically noisy loads away from high-speed data wiring where practical.
* Check display and SD module voltage requirements before connecting them to a 3.3v-only controller.

### I2C Bus

I2C is commonly useful for RTC modules, EEPROM, environmental sensors, displays, and I/O expanders.

* Devices sharing the bus need compatible addresses.
* Pull-up voltage must be safe for every device on the bus.
* Long cable runs and observatory wiring can require lower bus speed, bus buffering, or a different physical interface.

### Analog IO

Analog inputs can be used for light sensors, position feedback, current sensing, joysticks, and other simple measurement devices.

* Never exceed the MCU's analog input voltage.
* Use appropriate scaling, calibration, and filtering for position or environmental measurements.
* For important axis position feedback, verify the complete travel range and failure behavior before enabling unattended motion.

### Mounts, Motors, and Covers

* Motor drivers should provide the current and voltage handling required by the actual motor.
* Home and limit switches are strongly recommended on mechanisms that can damage themselves at the end of travel.
* Covers, shutters, and roofs should have independent physical protection where a motion failure could damage the telescope or building.
* Position feedback and software limits improve control, but do not replace appropriate mechanical design.

### Sensors

Weather and environmental sensors can help decide whether observing is safe, but a hobby controller should not be the only protection for expensive equipment in severe weather.

Temperature and humidity sensors are useful for dew control. Rain, wind, and enclosure sensors can be used to interrupt a session and request a safe stow. Sensors should be mounted and calibrated for the actual installation.

### Networking & Wireless

* Networking is 100% optional for normal tracking.
* WiFi, Ethernet, MQTT, or remote UI can be enabled when remote control or data integration is wanted.
* Loss of network connectivity should remove only the feature that depends on it, not the basic local tracking process.
* Do not depend on a cloud service as the only way to park or make a local installation safe.

## Memory Callouts

* Total object count, catalog use, GUI support, logging, and networking all affect Flash and SRAM use.
* Built-in target and string data are designed to stay compact, but they still contribute to compiled image size.
* Disabling GUI, networking, debug features, or other unused support allows the compiler to strip code that is not needed.
* External-data workflows can move catalog/string data away from onboard Flash on constrained devices.
* Modern 32-bit MCUs with ample Flash and RAM are recommended when tracking, UI, logging, environmental control, and networking are all enabled together.

## Example Usage

Below are several of the main examples of controller usage. The example sketches in the repository remain the source of truth.

### Simple Equatorial Tracker Example

`SimpleEquatorial` is the basic two-axis tracking example and the best small starting point for a normal telescope mount.

```Arduino
// Simple-AstroTracker-Arduino Equatorial Tracking Example
//
// A very small equatorial setup using two user supplied axis drivers. The callbacks here
// only show where the actual stepper/servo calls would be made. This lets the example
// work with whatever motor hardware and gear reduction the tracker was built around.

#include <Astruino.h>

// Observer Settings
#define SETUP_LATITUDE                  49.2827             // Latitude, degrees (+N / -S)
#define SETUP_LONGITUDE                -123.1207            // Longitude, degrees (+E / -W)
#define SETUP_ELEVATION                70.0                 // Elevation, meters
#define SETUP_START_UNIX               1787101200LL         // Replace with RTC/GPS/NTP time in a real setup

Astruino astroController(Astro_MountType_Equatorial);

static double axisTargets[2] = {0.0, 0.0};
static uint8_t axisIndices[2] = {0, 1};

void setAxisTarget(void *context, double targetDegrees)
{
    uint8_t axisIndex = context ? *(uint8_t *)context : 0;
    if (axisIndex >= 2) { return; }
    axisTargets[axisIndex] = targetDegrees;

    // Send targetDegrees to the stepper/servo controller here.
    // The mount does not care what kind of motor is attached to each axis.
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    AstroObserver observer(SETUP_LATITUDE, SETUP_LONGITUDE, SETUP_ELEVATION);
    astroController.init();
    astroController.setObserver(observer);
    setTime((time_t)SETUP_START_UNIX);

    auto primaryDriver = astroController.addCallbackAxisDriver(setAxisTarget, nullptr, &axisIndices[0]);
    auto secondaryDriver = astroController.addCallbackAxisDriver(setAxisTarget, nullptr, &axisIndices[1]);

    auto &mount = astroController.getMount();
    mount.setAxisDriver(0, primaryDriver);
    mount.setAxisDriver(1, secondaryDriver);
    mount.setTarget(Astro_Target_M42);
    mount.setAxisRates(6.0, 6.0);
    mount.setStowPosition(0.0, 0.0);
    mount.track();

    astroController.launch();
    Serial.println(F("Astruino equatorial tracker started"));
}

void loop()
{
    astroController.update();

    static millis_t lastReport = 0;
    millis_t now = millis();
    if (now - lastReport >= 5000) {
        lastReport = now;

        Serial.print(F("RA axis target: "));
        Serial.print(axisTargets[0], 4);
        Serial.print(F(" deg, DEC axis target: "));
        Serial.print(axisTargets[1], 4);
        Serial.println(F(" deg"));
    }
}
```

### Night Session Example

`NightSession` shows the mount, cover, observation trigger, environmental state, logging, and scheduler working together through a complete nighttime sequence.

```Arduino
// Simple-AstroTracker-Arduino Night Session Example
//
// Demonstrates the scheduler driving a simple homemade setup with a mount, powered
// cover, camera trigger, weather safety input, and basic environmental readings.

#include <Astruino.h>

#define SETUP_LATITUDE                  49.2827
#define SETUP_LONGITUDE                -123.1207
#define SETUP_ELEVATION                70.0
#define SETUP_START_UNIX               1787101200LL

Astruino astroController(Astro_MountType_Equatorial);

static float coverPower = 0.0f;
static bool cameraActive = false;
static double ambientTemperatureC = 8.0;
static double humidityPercent = 78.0;
static double opticsTemperatureC = 8.5;
static double cameraSensorTemperatureC = -9.5;
static double cameraBodyTemperatureC = 7.0;
static double rainState = 0.0;

void driveCover(void *context, float power)
{
    (void)context;
    coverPower = power;
    // Drive a relay/H-bridge/servo here. Positive opens, negative closes.
}

void triggerCamera(void *context, bool active)
{
    (void)context;
    cameraActive = active;
    digitalWrite(LED_BUILTIN, active ? HIGH : LOW); // Replace with shutter/record pin if desired.
}

bool readValue(void *context, double *valueOut)
{
    if (!context || !valueOut) { return false; }
    *valueOut = *(double *)context;
    return true;
}

void logEvent(void *context, const AstroLogEvent &event)
{
    (void)context;
    Serial.print(event.prefix);
    Serial.println(event.message);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }
    pinMode(LED_BUILTIN, OUTPUT);

    AstroObserver observer(SETUP_LATITUDE, SETUP_LONGITUDE, SETUP_ELEVATION);
    astroController.init();
    astroController.setObserver(observer);
    setTime((time_t)SETUP_START_UNIX);

    auto coverActuator = astroController.addCallbackActuator(driveCover, Astro_ActuatorType_Cover);
    astroController.getCover().setActuator(coverActuator);
    astroController.getCover().setTravelRate(0.15f);

    auto ambientSensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Temperature,
                                                            Astro_UnitsType_Temperature_Celsius, &ambientTemperatureC);
    auto humiditySensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Humidity,
                                                             Astro_UnitsType_Humidity_RH, &humidityPercent);
    auto opticsSensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Temperature,
                                                           Astro_UnitsType_Temperature_Celsius, &opticsTemperatureC);
    auto cameraSensor = astroController.addCallbackSensor(readValue, Astro_SensorType_CameraTemperature,
                                                           Astro_UnitsType_Temperature_Celsius, &cameraSensorTemperatureC);
    auto cameraBodySensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Temperature,
                                                               Astro_UnitsType_Temperature_Celsius, &cameraBodyTemperatureC);
    auto rainSensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Rain,
                                                         Astro_UnitsType_Raw_1, &rainState);

    astroController.getThermalBalancer().setAmbientTemperatureSensor(ambientSensor);
    astroController.getThermalBalancer().setHumiditySensor(humiditySensor);
    astroController.getThermalBalancer().setOpticsTemperatureSensor(opticsSensor);
    astroController.getThermalBalancer().setCameraSensorTemperatureSensor(cameraSensor);
    astroController.getThermalBalancer().setCameraBodyTemperatureSensor(cameraBodySensor);

    auto rainTrigger = astroController.addThresholdTrigger(rainSensor, 0.5, false, 0.0, 1000);
    astroController.getScheduler().setSafetyTrigger(rainTrigger);

    astroController.getCamera().setTriggerCallback(triggerCamera);
    astroController.getCamera().setReady(true);
    astroController.getLogger().setSink(logEvent);
    astroController.getScheduler().setTarget(Astro_Target_M31);

    astroController.launch();
}

void loop()
{
    // Replace these example values with installed sensors.
    ambientTemperatureC -= 0.0002;

    astroController.update();

    static millis_t lastReport = 0;
    millis_t now = millis();
    if (now - lastReport >= 5000) {
        lastReport = now;
        Serial.print(F("Cover: "));
        Serial.print(astroController.getCover().getPosition(), 2);
        Serial.print(F(" power: "));
        Serial.print(coverPower, 2);
        Serial.print(F(" camera: "));
        Serial.println(cameraActive ? F("recording") : F("idle"));
    }
}
```

### Data Writer Example

`DataWriter` exports built-in target data for external-storage workflows. This is especially useful on storage-constrained controllers or when library data needs to be regenerated for another storage target.

```Arduino
// Simple-AstroTracker-Arduino Data Writer Example
//
// In this example we export the built-in AstroLib target records as compact JSON. This is
// useful when preparing external SD/EEPROM storage for a constrained controller, or when
// regenerating the built-in data after catalog changes.
//
// Astruino can operate with all built-in target data kept in Flash. External storage is
// optional and is mainly useful when program space matters or user catalog data is desired.
//
// The companion tests/AstroLibExportToCPP sketch performs the opposite development task:
// it exports checked-out target data as C++ PROGMEM cases for inclusion in the library.

#include <Astruino.h>

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("Astruino external data export"));

    Serial.println(F("=== Library strings ==="));
    for (int stringIndex = 0; stringIndex < AStr_Count; ++stringIndex) {
        Serial.print(stringIndex);
        Serial.print(':');
        Serial.println(SFP((Astro_String)stringIndex));
        yield();
    }

    Serial.println(F("=== AstroLib targets ==="));

    char jsonBuffer[256];
    char targetId[ASTRO_TARGET_ID_MAXSIZE];

    for (int targetIndex = 0; targetIndex < Astro_Target_Count; ++targetIndex) {
        Astro_TargetId target = (Astro_TargetId)targetIndex;
        const AstroTargetData *targetData = astroLib.checkoutTargetData(target);

        if (targetData) {
            if (astroTargetIdToString(target, targetId, sizeof(targetId)) &&
                targetData->toJSON(jsonBuffer, sizeof(jsonBuffer))) {
                Serial.print(targetId);
                Serial.print(':');
                Serial.println(jsonBuffer);
            }

            astroLib.returnTargetData(targetData);
        }

        yield();
    }

    Serial.println(F("Done!"));
}

void loop()
{ ; }
```

### Main System Examples

* **SimpleEquatorial** shows a small equatorial tracker using user-supplied motor target callbacks.
* **AstroLibLookup** shows target checkout, coordinate resolution, and return.
* **NightSession** shows a mount, cover, observation trigger, environmental input, logger, and scheduler working together.
* **ThermalCamera** shows the camera/dew thermal-balancing path.
* **DataWriter** exports target data for external-data workflows.

The examples keep final motor and sensor hardware behind callbacks and interfaces so they can be adapted to the parts used by the actual build.

## Astronomy Callouts

### AstroLib

Repeated target checkouts share the same loaded target object. Returning the last normal checkout releases it again. Moving solar-system targets use the same target interface but resolve coordinates for the requested time.

### Covers, Focusers, and Safe Stow

A cover intentionally stays generic so the same logic can operate a telescope cap, sliding cover, roll-off roof, dome shutter, or similar mechanism. The scheduler only needs a known state and a way to command movement.

The focuser is similarly hardware-neutral. It can command relative or absolute motion and can use external feedback when available.

Safe stow coordinates these pieces in the correct order. The mount parks before the cover is allowed to close, and motion faults stop the observation sequence.

### Thermal and Dew Control

Nighttime observing can have several thermal goals at once. Optics should stay clear of dew, electronics may need minimum-temperature protection, and a camera may need controlled cooling. Astruino treats these as coordinated balancing outputs instead of unrelated heater switches.

Camera cooling is still early functionality and should be reviewed carefully against real hardware before being trusted with expensive equipment.

### Accuracy

The astronomy calculations are intended for DIY pointing and tracking. The goal is correct, consistent coordinates with errors small compared with the normal mechanical limits of home-built mounts.

Astruino does not try to replace professional high-precision astrometry or ephemeris software. Alignment models, periodic-error correction, automatic meridian handling, and higher-order pointing corrections remain natural extensions for mounts that require more precision.

## License

Astruino is released under the MIT License. See `LICENSE` for details.
