# Astruino
Astruino: Simple Astro Tracker Automation Controller.

**Simple-AstroTracker-Arduino v0.7.0**

Simple automation controller for DIY astronomical tracking systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

This controller manages telescope mounts, motors, servos, covers, observation equipment, sensors, thermal control, scheduling, logging, and data collection for home-built astronomical tracking systems. It is intended for ordinary Arduino-compatible hardware and common maker parts instead of specialized observatory control equipment.

The system is designed around the same Keep-It-Simple ideas used by Hydruino and Helioduino:

* Can be used entirely offline with a reliable clock and known static location.
  * GPS can provide time and location when wanted.
  * WiFi, Ethernet, MQTT, and remote interfaces are optional and are not required for normal tracking.
* Supports equatorial, Alt/Az, and simple single-axis mount geometry.
  * Right ascension and declination targets can be converted into local horizontal coordinates.
  * Local sidereal time, Julian dates, coordinate normalization, and J2000 precession are calculated in the library.
  * Mount movement is kept separate from motor hardware so steppers, servos, DC motors, and custom controllers can be adapted without changing astronomy code.
* Includes `AstroLib`, based on the same checkout/cache approach used by Hydruino's CropLib.
  * Includes all 110 Messier objects.
  * Includes a useful set of bright stars for alignment and basic observing.
  * Includes the Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus, and Neptune as moving targets.
  * Fixed target coordinates are stored compactly as integer seconds-of-time and arcseconds.
  * Checked-out target data is exposed through normal `AstroTargetData` objects and released when no longer needed.
  * Custom target slots and external target loading are supported so the built-in catalog is not a hard limit.
* Uses Flash/PROGMEM string storage for common library strings.
  * `SFP()` returns a normal memory-resident string when one is needed.
  * `CFP()` and `pgmAddrForStr()` provide direct Flash string access where appropriate.
  * Enum import decoding is generated as a compact minimum-discriminator tree instead of repeatedly comparing full strings.
* Supports familiar hobby electronics and Arduino-style I/O.
  * Digital and analog pins, active-low inputs, PWM outputs, pin muxing, callbacks, sensors, actuators, activation handles, triggers, measurements, and power rails use the same general patterns as the sibling libraries.
  * Optional time and location providers allow RTC, GPS, network, or user-supplied implementations without changing tracking logic.
  * Manual time and fixed-location providers are included for simple offline builds.
* Supports a generic `Cover` mechanism.
  * A cover can represent a telescope cap, sliding cover, bay opening, roll-off roof, dome shutter, or another open/closed mechanism.
  * The scheduler only needs to know the cover state and how to command its actuator.
* Supports generic observation devices and a simple camera trigger.
  * The trigger can drive a shutter pin, relay, external controller, or another recording device.
  * Observation control remains separate from the telescope mount so other instruments can use the same scheduling path.
* Includes Hydruino-style environmental balancing ideas for nighttime operation.
  * Dew point is calculated from ambient temperature and humidity.
  * Dew-heater output can maintain optics above the dew point without simply running a heater at full power.
  * Equipment heating, fans, and optional camera cooling are treated as parts of the same thermal state.
  * Camera thermal control is still considered early functionality and should be reviewed against real hardware before depending on it for expensive equipment.
* Includes a Helioduino-style scheduler adapted for nighttime observing.
  * Daytime defaults to stowed and covered.
  * Night operation can deploy the system, cool equipment, slew, settle, observe, warm up, and return to safe stow.
  * Unsafe weather can interrupt the sequence and force a safe stow.
  * Deployment and stow use configurable Sun-altitude thresholds.
* Includes system event logging and polling-frame data publishing.
  * Logger, Publisher, and Scheduler settings are stored as sub-data inside `AstroSystemData` in the same style as the sibling libraries.
  * Output sinks remain optional so an offline controller does not need networking or external storage.
* Includes the familiar `full`, `min`, `shared`, display/input, screen, and tcMenu source layout.
  * Astruino-specific UI work is intentionally incomplete in this release.
  * UI TODOs are retained by design while the controller and astronomy object model are established.

Made primarily for Arduino microcontrollers and Arduino-like build environments. Modern ESP32, RP2040/RP2350, Teensy, STM32, GIGA, Portenta, and similar devices are natural targets. Smaller controllers can still be useful when the enabled feature set and built-in data are kept reasonable.

## About

Astruino is aimed at DIY'ers, students, experimenters, and hobby astronomers who want to build tracking equipment from common parts. A useful first project should not require a commercial mount controller, cloud account, internet connection, or expensive astronomy-specific electronics.

A basic system can be an Arduino-compatible MCU, an RTC, known latitude and longitude, one or two motorized axes, motor drivers, and home or limit switches. That is enough to work with sidereal time, celestial coordinates, gearing, mechanical error, sensors, and control systems while building something that moves under the real night sky.

More hardware can be added as the project grows. Encoders can improve position feedback. A humidity sensor can add dew protection. A servo can operate a cover. A relay can trigger a camera. A rain or wind sensor can force a safe stow. GPS can remove manual time and location setup. None of these are required to get started.

## Controller Setup

### Requirements

There is no single minimum MCU for every Astruino build because the catalog, UI, logging, networking, and external libraries can change program size considerably.

For a normal build, a modern 32-bit Arduino-compatible MCU with at least a few hundred kilobytes of Flash and enough RAM for the selected libraries is recommended. ESP32, RP2040/RP2350, Teensy 3.5+, STM32, GIGA, and Portenta-class boards are good starting points.

Pin-limited controllers can use multiplexers or I/O expanders. The pin layer keeps channel information separate from the underlying sensor or actuator object so the same equipment model can be used with direct or expanded I/O.

### Installation

Install the controller through the package manager used by the build environment, or extract the project into a `Simple-AstroTracker-Arduino` folder in the Arduino custom libraries directory.

From there, copy one of the example sketches and change the location, time source, pins, motor callbacks, and sensors to match the hardware being built.

### Header Defines

There are several defines inside `Astruino.h` that control optional library features. They can be uncommented directly or supplied through custom compiler/build flags. Build flags are preferred when practical because editing the installed library affects every sketch that uses it.

```Arduino
// Completely disable multitasking support.
//#define ASTRO_DISABLE_MULTITASKING

// Disable tcMenu GUI support.
//#define ASTRO_DISABLE_GUI

// Enable the platform WiFi library.
//#define ASTRO_ENABLE_WIFI

// Enable external serial AT-command WiFi support.
//#define ASTRO_ENABLE_AT_WIFI

// Enable the platform Ethernet library.
//#define ASTRO_ENABLE_ETHERNET

// Enable MQTT publishing support.
//#define ASTRO_ENABLE_MQTT

// Enable Adafruit GPS support for optional time/location input.
//#define ASTRO_ENABLE_GPS

// Disable built-in library data when an external-data build supplies it another way.
//#define ASTRO_DISABLE_BUILTIN_DATA

// Enable serial/debug output support.
//#define ASTRO_ENABLE_DEBUG_OUTPUT

// Enable verbose debug output.
//#define ASTRO_ENABLE_VERBOSE_DEBUG

// Enable debug assertions.
//#define ASTRO_ENABLE_DEBUG_ASSERTIONS
```

Networking remains opt-in. A build with none of the network defines enabled is a normal supported configuration.

### Initialization

The controller follows the same basic lifecycle as Hydruino and Helioduino:

```Arduino
#include <Astruino.h>

Astruino astroController(Astro_MountType_Equatorial);

void setup()
{
    AstroObserver observer(49.2827, -123.1207, 70.0);

    astroController.init();
    astroController.setObserver(observer);
    astroController.getMount().setTarget(Astro_Target_M42);
    astroController.launch();
}

void loop()
{
    astroController.update();
}
```

The normal `update()` method uses the configured time provider, or the platform/system clock when no custom provider is installed. The latest safety and thermal readings can be supplied with `setSafeToObserve()` and `setThermalReadings()`.

For custom run loops and testing, the explicit update overload remains available:

```Arduino
astroController.update(unixTime, elapsedSeconds, sunAltitudeDegrees,
                       safeToObserve, thermalReadings);
```

### Offline Time and Location

A fixed installation does not need GPS or networking. Location can be assigned directly with `setObserver()`, and a clock can be supplied through a provider.

```Arduino
AstroManualTimeProvider timeProvider(initialUnixTime);
AstroFixedLocationProvider locationProvider(observer);

astroController.setTimeProvider(&timeProvider);
astroController.setObserver(locationProvider.getObserver());
```

`AstroCallbackTimeProvider` and `AstroCallbackLocationProvider` can bridge RTC, GPS, NTP, radio, or application-specific implementations.

## AstroLib

The built-in target library follows the same basic checked-out-object pattern as CropLib:

```Arduino
const AstroTargetData *target = astroLib.checkoutTargetData(Astro_Target_M31);

if (target) {
    AstroEquatorialCoordinates coordinates = target->getCoordinates(unixTime);

    // Use target and coordinates here.

    astroLib.returnTargetData(target);
}
```

Repeated checkouts share the same loaded target book. Returning the last normal checkout unloads the object again. User-modified target data can remain resident when appropriate. This avoids keeping every built-in object expanded in RAM.

Moving solar-system targets use the same `AstroTargetData` interface but resolve their coordinates for the requested time.

### Library and String Data Export

Development/export sketches are included under `tests/`:

* `AstroLibExportToCPP` exports checked-out target objects into PROGMEM-ready C++ catalog entries.
* `EnumTrieExportToCPP` emits the compact discriminator trees used by enum string decoding.
* `EnumConversionTests` checks the string conversions used by those generated decoders.
* `JSONExportTests` exercises the serializable target and pin data structures on an Arduino build.

The `DataWriter` example shows the built-in target data in compact serialized form for users preparing their own external-storage workflow.

Common library text uses the `AstroStrings` Flash lookup layer. `SFP()` is the normal helper for code that needs a resident string, while `CFP()` gives a Flash pointer when the caller can use it directly.

## Objects, Pins, and Equipment

The framework keeps a similar object layering to Hydruino and Helioduino:

* `AstroObject` and `AstroIdentity` provide object keys, revisions, and linkages.
* `AstroPin`, `AstroDigitalPin`, and `AstroAnalogPin` provide serialized direct or virtual pin setup.
* `AstroActuator` uses activation handles to combine equipment requests.
* `AstroSensor` publishes measurements with units, timestamps, and polling frames.
* `AstroTrigger` provides threshold/range conditions with stable-time handling.
* `AstroRail` provides simple shared power-capacity accounting.
* `AstroFactory` contains the common object, pin, actuator, sensor, mount, rail, trigger, cover, camera, and driver builders.
* `AstroInterfaces.h` and `AstroInterfaces.hpp` provide the shared object, units, attachment, driver, trigger, pin, and observation-device interfaces used across the framework.

The astronomy side then builds on that layer rather than bypassing it.

## Cover

A `Cover` intentionally stays generic. The application can attach it to a relay pair, H-bridge, servo, or another actuator. The scheduler only reasons about open/closed position and the ability to drive toward the requested state.

This keeps the same code useful for a small telescope cap, a sliding bay cover, a roll-off roof, or a homemade dome shutter.

## Thermal Balancing

Nighttime observing can have several thermal goals at once. Optics should stay clear of dew. Electronics may need minimum-temperature protection. A cooled camera sensor may intentionally operate below ambient while the body or window still needs condensation control.

Astruino treats those as balancing outputs instead of independent on/off heater switches. The current camera-cooling path is intentionally generic and should be treated as early functionality until it has more real-hardware use.

## Scheduler, Logger, and Publisher

`AstroScheduler` controls the high-level day/night sequence. Its serialized configuration stores deploy/stow Sun-altitude limits, settling tolerance/time, and environment-report interval.

`AstroLogger` records system events through an optional sink and stores its log-level/file settings in `AstroLoggerSubData`.

`AstroPublisher` gathers sensor measurements into polling-frame columns and emits completed frames through an optional sink. Its storage settings are held in `AstroPublisherSubData`.

These subsystems are public members of the main controller, matching the sibling-library pattern:

```Arduino
astroController.scheduler;
astroController.logger;
astroController.publisher;
```

## Examples

* `SimpleEquatorial` shows a small equatorial tracker using motor target callbacks.
* `AstroLibLookup` shows target checkout, coordinate resolution, and return.
* `NightSession` shows a mount, cover, camera trigger, environmental input, logger, and scheduler working together.
* `ThermalCamera` shows the experimental camera/dew thermal-balancing path.
* `DataWriter` shows catalog serialization for an external-data workflow.

The examples intentionally use callbacks around the final motor/sensor hardware so they can be adapted to whatever parts are available instead of requiring one specific shield or motor library.

## User Interface

The Helioduino-style `full`, `min`, and `shared` tcMenu layout is included so the library can grow into the same UI architecture without reorganizing the project later.

Astruino UI files are still work in progress. Their TODO markers are intentional in v0.7.0. The tracking, catalog, scheduler, sensors, actuators, reporting, and offline operation do not depend on a completed GUI.

## Host Tests

Core logic and source checks can be run without a telescope or Arduino connected:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
python3 tests/validate_source.py
```

The host suite covers astronomy math, precession, moving-body sanity, catalog lookup/cache behavior, automation, mount/cover control, thermal balancing, sensors, actuators, activation handles, pins, muxing, measurements, triggers, power rails, factory creation, provider interfaces, controller lifecycle, serialization, reporting, enum round trips, generated trie freshness, source-parity guards, and example syntax.

`tests/generate_enum_trie.py` regenerates `AstroEnumTrie.h` directly from the shipping `toString()` vocabulary. Source validation fails if the checked-in decoder no longer matches generated output.

The source validator also guards several family conventions that are easy to accidentally regress, including the PROGMEM string API, core framework file depth, comments on stored members/enums, and the absence of positional `sscanf()` serialization.

## Accuracy

The astronomy calculations are intended for DIY pointing and tracking. The goal is correct, consistent coordinates with errors small compared with the normal mechanical limits of home-built mounts. The library does not try to replace professional high-precision astrometry or ephemeris software.

Alignment models, encoders, guiding, periodic-error correction, and other feedback layers can improve final pointing without making those systems mandatory for a first build.
