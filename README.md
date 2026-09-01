# Astruino
Astruino: Simple Astro Tracker Automation Controller.

**Simple-AstroTracker-Arduino v0.7.2.0**

Simple automation controller for DIY astronomical tracking systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

Astruino provides the controller layer for home-built astronomical trackers and small observatory systems. It manages registered mounts, axis drivers, covers, cameras, focusers, environmental sensors, thermal control, scheduling, logging, publishing, and persistent configuration while keeping the actual mechanical and electrical implementation open to the builder.

Our Keep-It-Simple controller system:

* Runs locally without requiring a network connection.
  * A reliable clock and known installation location are enough for normal astronomical tracking.
  * GPS, WiFi, Ethernet, MQTT, remote control, and UI support are optional additions.
* Supports equatorial, Alt/Az, and single-axis mount geometries.
  * Includes Julian date handling, sidereal-time calculations, J2000 precession, equatorial-to-horizontal conversion, pulse guiding, park positions, per-axis software limits, and optional position feedback.
  * Mount geometry remains separate from motor hardware.
* Includes common axis-driver paths.
  * `AstroCallbackAxisDriver` adapts an existing motor/controller library.
  * `AstroServoAxisDriver` maps an axis target onto a normalized servo-style output.
  * `AstroStepDirAxisDriver` drives ordinary STEP/DIR hardware from the normal controller update loop.
* Includes a compact astronomical target library.
  * Built-in data includes all 110 Messier objects, useful bright-star targets, and the Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus, and Neptune.
  * Fixed targets use stored J2000 coordinates; moving solar-system targets are resolved for the requested UTC time.
  * SD-card and EEPROM target-data workflows are supported for builds that do not keep the catalog in Flash.
* Uses registered system objects plus lightweight sub-objects and attachments.
  * Registered object families are actuators, sensors, targets, mounts, and power rails.
  * Covers, cameras, thermal balancing, axis drivers, and triggers remain attached or mount-owned helpers instead of parallel top-level object families.
* Supports common small-observatory equipment.
  * `AstroCover` handles a generic open/close mechanism with optional open/closed limit sensors and travel timeout protection.
  * `AstroCamera` provides interval and exposure timing around a shutter actuator.
  * `AstroFocuser` provides absolute and relative position control with optional feedback callbacks.
* Includes environmental and thermal balancing.
  * Dew point, optics heating, electronics heating, camera cooling, and fan output can be coordinated by the mount-owned `AstroThermalBalancer`.
* Includes automatic nighttime scheduling for every registered mount.
  * Current internal tracking stages are `Init`, `Warm`, `Deploy`, `Acquire`, `Track`, and `Stow`.
  * Storm triggers force stow behavior.
  * Cover closure waits for the mount to be parked and the camera thermal state to be safe to stow.
* Includes event logging and polling-frame sensor publishing.
  * SD-card logging/publishing is supported locally.
  * WiFiStorage and MQTT paths are optional when enabled.

Made primarily for Arduino microcontrollers and build environments, but the source is also intended to fit PlatformIO, Espressif, Teensy, STM32, Pico/RP2040/RP2350, GIGA, Portenta, and similar modern MCU platforms. Smaller boards may require GUI, networking, debug output, or built-in data to be trimmed.

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## About

Astruino is intended for garage-built trackers, converted manual mounts, 3D-printed mechanisms, small backyard telescopes, roll-off roofs, dome shutters, and similar DIY astronomy projects.

The controller owns the shared system services. Mounts and other equipment are created as registered objects through the factory helpers. Each `AstroMount` then owns its cover, camera controller, and thermal balancer as sub-objects. This keeps the astronomy, system registration, scheduling, and physical hardware layers separate instead of forcing one fixed telescope design.

A useful mental model is:

**Target + UTC Time + System Location → Mount Geometry → Axis Driver → Physical Motion**

and, alongside it:

**Sensors → Measurements → Triggers / Thermal Logic → Actuators**

The scheduler coordinates those pieces across the observing night.

## Controller Setup

### MCU Requirements

There is no single minimum MCU for every Astruino build because enabled UI, networking, logging, target-data storage, and object counts change the program and memory requirements considerably.

As a practical starting point:

Minimum planning target: 256-512kB Flash, 16-24kB SRAM, 16MHz+  
Recommended: 512kB-1MB+ Flash, 24-32kB+ SRAM, 32-48MHz+

Modern 32-bit boards such as ESP32, RP2040/RP2350, Teensy 3.5+, STM32, GIGA, and Portenta-class devices are the natural starting point when tracking, logging, UI, and networking are expected to run together.

Astronomy calculations use floating-point math regularly. Motor timing, encoder feedback, display load, and communication traffic can matter more than Flash size alone when selecting the MCU.

### Installation

Install through the Arduino Library Manager when available, through PlatformIO, or by placing the repository in the Arduino custom libraries directory as `Simple-AstroTracker-Arduino`.

The repository currently includes these example groups:

* `examples/SimpleEquatorial`
* `examples/AstroLibLookup`
* `examples/NightSession`
* `examples/ThermalCamera`
* `examples/DataWriter`

The public API is still moving on the `develop` branch. When working from `develop`, use the headers as the authoritative API reference if an older example sketch has not yet been updated to the current factory/registration model.

### Header Defines

The main `Astruino.h` header exposes optional build controls. Supplying them as build flags is generally preferable to editing the installed library header.

```Arduino
//#define ASTRO_DISABLE_MULTITASKING
//#define ASTRO_DISABLE_GUI
//#define ASTRO_ENABLE_WIFI
//#define ASTRO_ENABLE_AT_WIFI
//#define ASTRO_ENABLE_ETHERNET
//#define ASTRO_ENABLE_MQTT
//#define ASTRO_ENABLE_GPS
//#define ASTRO_DISABLE_BUILTIN_DATA
//#define ASTRO_ENABLE_DEBUG_OUTPUT
//#define ASTRO_ENABLE_VERBOSE_DEBUG
//#define ASTRO_ENABLE_DEBUG_ASSERTIONS
```

Networking is opt-in. A normal offline tracker does not need any network define enabled.

### Main Dependencies

The exact dependency set depends on enabled features. `library.properties` currently includes the controller-family support libraries plus Astruino-specific sensor/data dependencies such as:

* ArduinoJson
* ArxContainer / ArxSmartPtr
* RTClib and Time
* TaskManagerIO and IoAbstraction
* I2C_EEPROM and SD
* DHT and OneWire
* Adafruit GPS when GPS support is enabled
* MQTT when MQTT publishing is enabled
* WiFi101, WiFiNINA_Generic, WiFiEspAT, or Ethernet for the selected network path
* tcMenu and its display/input support libraries when GUI support is enabled

### Controller Construction

The controller constructor configures controller-level hardware services, not mount geometry:

```Arduino
Astruino astroController;
```

The full constructor can accept piezo, EEPROM, RTC, SD, network, GPS, control-input, and display setup information. Mounts are created separately after initialization.

### Initialization

A normal empty-system initialization is:

```Arduino
astroController.init();
```

The current signature is:

```Arduino
void init(Astro_SystemMode systemMode = Astro_SystemMode_Tracking,
          Astro_MeasurementMode measureMode = Astro_MeasurementMode_Default,
          Astro_DisplayOutputMode dispOutMode = Astro_DisplayOutputMode_Disabled,
          Astro_ControlInputMode ctrlInMode = Astro_ControlInputMode_Disabled);
```

A fixed installation can set its geographic position directly:

```Arduino
astroController.setSystemLocation(49.2827, -123.1207, 70.0);
```

The main lifecycle is:

1. Construct `Astruino`.
2. Call `init()` or one of the storage-backed initialization methods.
3. Configure location and registered equipment.
4. Call `launch()`.
5. Call `update()` continuously from `loop()`.

`launch()` enables the controller run loops. `suspend()` disables them without destroying the configured system.

### Current Mount Setup Pattern

Mount geometry is selected when the mount object is created:

```Arduino
Astruino astroController;

void setup()
{
    astroController.init();
    astroController.setSystemLocation(49.2827, -123.1207, 70.0);

    auto mount = astroController.addMount(Astro_MountType_Equatorial);

    auto primaryDriver = astroController.addMountAxisStepper(
        2, 3, 4,          // STEP, DIR, ENABLE
        1280.0,           // steps per axis degree after gearing/microstepping
        4000.0);          // maximum step rate

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

The scheduler sees registered mounts and coordinates their nightly state. Application code can still call `park()`, `unpark()`, `track()`, set axis positions/limits, or replace the axis drivers as needed.

## Mounts and Axis Drivers

`AstroMount` supports:

* `Astro_MountType_Equatorial`
* `Astro_MountType_AltAzimuth`
* `Astro_MountType_SingleAxis`

Each mount tracks current and target position, configured rate, optional software limits, park position, guide offsets, and optional axis-driver feedback.

If a driver supplies position feedback through `getPositionDegrees()`, that reading updates the mount axis state. If feedback is unavailable, Astruino advances an internal rate-limited position estimate toward the current target.

### Callback Driver

`AstroCallbackAxisDriver` is the most flexible bridge to an existing motor-control library or external controller. It accepts target and stop callbacks plus optional position feedback.

### Servo Driver

`AstroServoAxisDriver` maps a target angle over a configured angular range onto an analog/PWM-style output.

### STEP/DIR Driver

`AstroStepDirAxisDriver` drives a STEP pin, DIR pin, and optional ENABLE pin. It tracks motor position in steps, supports configurable steps-per-degree and maximum steps per second, and advances motion from `update()`.

This is useful for ordinary DIY STEP/DIR systems. Builds that require very high pulse rates, acceleration planning, or tightly timed hardware stepping can instead use `AstroCallbackAxisDriver` with a dedicated motor library/controller.

## Covers, Camera, and Focuser

### Cover

Every mount owns an `AstroCover` sub-object:

```Arduino
auto coverMotor = astroController.addCoverMotorRelay(8, 9, true);
auto openLimit = astroController.addLimitSwitch(10, true);
auto closedLimit = astroController.addLimitSwitch(11, true);

mount->getMountCover().setActuator(coverMotor);
mount->getMountCover().setOpenSensor(openLimit);
mount->getMountCover().setClosedSensor(closedLimit);
```

`AstroRelayMotorActuator` uses two binary outputs with signed drive convention: forward/open and reverse/close. `AstroCover` can also operate another compatible actuator through its attachment.

Open/closed sensors are optional. When present, they provide the real end-state indication. The cover also maintains a normalized travel estimate and movement timeout, and can enter a faulted state if commanded travel does not complete.

### Camera

Each mount owns an `AstroCamera` sub-object. It supports interval and exposure modes around an attached camera-shutter actuator.

```Arduino
auto shutter = astroController.addCameraShutterRelay(12, true);
mount->getCamera().setShutter(shutter);
mount->getCamera().setMode(AstroCamera::Interval);
mount->getCamera().setInterval(30000);
mount->getCamera().setShutterPulseTime(250);
```

The scheduler starts observation only when the camera controller reports ready.

### Focuser

`AstroFocuser` is a registered actuator object with integer-step positioning:

```Arduino
auto focuser = astroController.addFocuser(20000);
```

It supports absolute/relative movement, configurable limits, stop handling, and optional position callbacks.

## Sensors, Measurements, and Triggers

The current sensor classes are:

* `AstroValueSensor`
* `AstroCallbackSensor`
* `AstroDigitalSensor`
* `AstroAnalogSensor`

All sensor readings are represented through the measurement layer. Measurements include a UTC timestamp and polling frame; numeric measurements also retain units.

Digital sensors support optional ISR notification and minimum-stable-time filtering. Analog sensors read normalized pin values and can use user calibration data to convert that raw input into meaningful engineering units.

Current trigger classes are:

* `AstroMeasurementValueTrigger` - threshold comparison
* `AstroMeasurementRangeTrigger` - range comparison

Both are sensor-backed sub-objects and support de-trigger tolerance and optional de-trigger delay.

For example, a rain input can become a storming condition for one mount:

```Arduino
auto rain = astroController.addRainIndicator(13, true);
auto storm = astroController.addThresholdTrigger(rain, 0.5, false, 0.0, 1000);
mount->setStormingTrigger(storm);
```

The trigger reports the condition. The mount/scheduler decide what action follows.

## Thermal Control

Every mount owns an `AstroThermalBalancer`.

It can consume:

* Ambient temperature
* Humidity
* Optics temperature
* Camera sensor temperature
* Camera body temperature

and can drive:

* Dew heater
* Electronics heater
* Camera cooler
* Camera fan

The balancer provides `DayStorage`, `NightObserving`, and `SafeStowed` thermal modes. Dew point is calculated from ambient temperature and humidity, and the optics target can be held above dew point without simply driving a heater at full power.

Camera cooling is ramped toward the requested target. `cameraSafeToStow()` is used by the scheduler before enclosure closure.

## Scheduler

`AstroScheduler` is a public controller subsystem:

```Arduino
astroController.scheduler;
```

It creates a tracking process for each registered mount. The current internal sequence is:

`Init → Warm / Deploy / Stow → Acquire → Track → Stow`

The actual transition depends on twilight, configured triggers, cover state, mount alignment, and thermal state.

### Night and Twilight

The scheduler calculates sunrise/sunset from the controller's system location when location data is available. It keeps daily and next-day twilight values and updates them when the date changes.

### Pre-Dusk Warmup

A mount can have a heating trigger. When pre-dusk heating is due and a dew-heater actuator is linked to that mount, the scheduler can enter `Warm` before night deployment.

The pre-dusk interval is configured with:

```Arduino
astroController.scheduler.setPreDuskHeatingMins(10);
```

### Safe Stow

A mount's storming trigger forces the tracking process toward `Stow`.

During stow:

1. Camera observation is stopped.
2. The mount is commanded to park.
3. Thermal control moves to `SafeStowed` when storming, otherwise day-storage behavior.
4. The cover is only closed after the mount is parked and the camera is thermally safe to stow.

The cover and mount should still have appropriate physical protection. Software sequencing is not a replacement for hard limits, interlocks, fusing, or emergency controls.

## Targets and Catalog Data

`AstroTargetsLib` is the global `AstroTargetsLibrary` instance.

The library uses checkout/return reference counting so target records do not all need to remain expanded in RAM:

```Arduino
const AstroTargetsLibData *target = AstroTargetsLib.checkoutTargetsData(Astro_TargetType_M42);
if (target) {
    AstroEquatorialCoordinates coords = target->getCoordinates(unixNow());
    AstroTargetsLib.returnTargetsData(target);
}
```

`AstroTargetsLibData` contains target type/class, catalog ID, display name, compact J2000 coordinates, and the moving-target flag.

`AstroStaticTarget` resolves stored J2000 catalog positions with precession. `AstroDynamicTarget` represents time-dependent solar-system targets. A mount can select a target directly with `setTarget(Astro_TargetType)` and performs the necessary catalog checkout internally during tracking.

The target library can also use SD-card or EEPROM data when built-in Flash data is disabled or custom storage is preferred.

## Power Rails

Power rails model shared capacity; they are not electrical protection.

`AstroSimpleRail` limits the number of linked actuators allowed to be active at once.

`AstroRegulatedRail` uses a maximum-power setting, optional power-usage sensor, and limit trigger to decide whether another actuator can be activated.

Use real fuses, suitable conductors, current limiting, and properly rated power hardware regardless of the software rail model.

## Event Logging and Data Publishing

The controller exposes:

```Arduino
astroController.logger;
astroController.publisher;
```

`AstroLogger` supports filtered event logging and local SD-card logs. WiFiStorage logging is available on matching builds.

`AstroPublisher` organizes sensor data into polling-frame columns and can publish rows to SD card, optional WiFiStorage, and optional MQTT.

A network connection is not required for logging, publishing, target lookup, tracking, or scheduling.

## Persistence

Astruino separates live objects from serializable data structures.

System setup can be initialized or saved through:

* EEPROM
* SD card
* WiFiStorage when available
* JSON streams
* Binary streams

`AstroSystemData` stores system mode, measurement mode, UI modes, name, time-zone offset, polling interval, autosave configuration, network credentials, system location, and scheduler/logger/publisher sub-data.

Registered objects provide their own save-data structures. `AstroCalibrationData` stores the familiar linear `y = A*x + B` transformation for sensor/output calibration.

## Host Tests

Core tests can be run without telescope hardware:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

Host tests are useful for astronomy math, object behavior, scheduling, serialization, measurements, and regression coverage. They cannot validate motor polarity, real gear ratios, backlash, mechanical limits, cover travel, wiring, or weather behavior.

## Hookup Callouts

### General

* Verify MCU and peripheral logic voltage before connection.
* Do not power motors, heaters, relays, TECs, solenoids, or similar loads directly from MCU pins.
* Use suitable motor drivers, relay/MOSFET interfaces, fusing, grounding, isolation, and power supplies.
* Software axis limits and cover timeouts are secondary protections, not the only protections against mechanical damage.

### STEP/DIR Motors

* Confirm the driver's STEP/DIR/ENABLE logic levels and polarity.
* Calculate `stepsPerDegree` from motor steps, microstepping, gearing, and final axis reduction.
* Begin with a conservative maximum step rate and verify actual motion direction before enabling unattended tracking.
* High-performance motion may be better delegated to a dedicated stepper library/controller through the callback driver.

### Covers and Roofs

* Use open/closed limit switches where over-travel can damage the mechanism.
* Use independent physical interlocks when a roof or shutter could collide with the telescope.
* Verify the park position physically before allowing automatic closure.

### I2C / SPI / UART

* Check bus voltage, address/chip-select conflicts, and cable length.
* Long observatory runs may require slower buses, buffering, or differential interfaces.
* Keep high-current motor/heater wiring away from sensitive communication and sensor wiring where practical.

### Weather Inputs

Weather sensors can help make an automated system safer, but a hobby controller should not be the only protection for valuable equipment in severe weather. Validate sensor failure modes as well as normal readings.

## Memory Callouts

* Registered object count, target-data source, GUI, networking, logging, and debug features all affect Flash/SRAM use.
* `ASTRO_DISABLE_BUILTIN_DATA` supports external catalog-data workflows when Flash is constrained.
* Disabling unused GUI/network/debug functionality allows the compiler to remove code that is not needed.
* Larger 32-bit MCUs are recommended when several equipment and communication subsystems are active together.

## Source Layout

The major source groups currently include:

* `Astruino.*` - controller lifecycle, storage, run loops, device setup
* `AstroCoreLogic.*` - astronomy and reusable control math
* `AstroObject.*`, `AstroAttachments.*`, `AstroModules.*` - object identity, dynamic links, registration, calibration, pin handlers
* `AstroActuators.*`, `AstroSensors.*`, `AstroMeasurements.*`, `AstroTriggers.*` - equipment I/O and condition model
* `AstroDrivers.*`, `AstroMounts.*`, `AstroCamera.*`, `AstroThermal.*` - mount and observatory mechanisms
* `AstroTargets.*`, `AstroTargetsLibrary.*` - registered targets and catalog/ephemeris data
* `AstroRails.*` - software power-capacity coordination
* `AstroScheduler.*`, `AstroLogger.*`, `AstroPublisher.*` - system orchestration and reporting
* `AstroData.*`, `AstroDatas.*` - persistent data and calibration
* `shared/`, `min/`, `full/` - shared UI infrastructure and controller-family UI layouts

## License

Astruino is released under the MIT License. See `LICENSE` for details.
