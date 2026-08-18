# Astruino
Astruino: Simple Astro Tracker Automation Controller.

**Simple-AstroTracker-Arduino v0.7.0.0**

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

Dependencies include: ArduinoJson, ArxContainer, ArxSmartPtr, I2C_EEPROM, IoAbstraction, RTClib, TaskManagerIO, Time, and platform-like SD/SPI/Wire libraries. Optional features may also use Adafruit GPS, MQTT, tcMenu, WiFi101, WiFiNINA_Generic, WiFiEspAT, or Ethernet. Additional tcMenu display/input dependencies depend on the UI hardware selected.

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

Certain setups require extra libraries depending on the selected hardware.

* **RTClib / Time** provide the normal Arduino-side time foundation. A fixed installation can also supply time through an `AstroTimeProvider`.
* **Adafruit GPS** is optional and can provide time and location when GPS is preferred over a fixed location.
* **tcMenu** and its display/input dependencies are optional. The common adapter layer is present, while Astruino-specific menus remain under development.
* **MQTT** and the selected WiFi/Ethernet library are only needed when network publishing or remote integration is enabled.
* **I2C_EEPROM** and platform storage libraries can be used by application or data-export workflows where external storage is desired.

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

The normal `update()` path uses the configured time provider or platform/system clock. `AstroManualTimeProvider`, `AstroFixedLocationProvider`, and callback providers can bridge RTC, GPS, NTP, radio, or application-specific sources without changing the tracking code.

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

Below are several examples of controller usage.

### Simple Equatorial Tracker Example

The `SimpleEquatorial` example shows the smallest useful two-axis tracking setup. Motor hardware is supplied through callbacks so the mount code does not depend on one specific driver library.

```Arduino
#include <Astruino.h>

Astruino astroController(Astro_MountType_Equatorial);

void setup()
{
    AstroObserver observer(49.2827, -123.1207, 70.0);

    astroController.init();
    astroController.setObserver(observer);

    auto &mount = astroController.getMount();
    mount.setTarget(Astro_Target_M42);
    mount.setAxisRates(6.0, 6.0);
    mount.track();

    astroController.launch();
}

void loop()
{
    astroController.update();
}
```

A real build still needs to connect the mount's axis target callbacks to stepper, servo, DC motor, or external motion-control hardware.

### Main System Examples

* **SimpleEquatorial** shows a small equatorial tracker using user-supplied motor target callbacks.
* **AstroLibLookup** shows target checkout, coordinate resolution, and return.
* **NightSession** shows a mount, cover, camera trigger, environmental input, logger, and scheduler working together.
* **ThermalCamera** shows the experimental camera/dew thermal-balancing path.
* **DataWriter** shows catalog serialization for an external-data workflow.

The examples intentionally keep final motor and sensor hardware behind callbacks and interfaces so they can be adapted to whatever parts are available.

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
