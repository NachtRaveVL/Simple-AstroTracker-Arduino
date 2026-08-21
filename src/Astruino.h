/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Simple-AstroTracker-Arduino - Version 0.7.2.0
*/

#ifndef Astruino_H
#define Astruino_H

// Library Setup
// NOTE: It is recommended to use custom build flags instead of editing this file directly.

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

#ifdef ARDUINO
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#ifndef USE_SW_SERIAL
typedef HardwareSerial SerialClass;
#else
#include <SoftwareSerial.h>
#define ASTRO_USE_SOFTWARE_SERIAL
typedef SoftwareSerial SerialClass;
#endif

#if defined(ASTRO_ENABLE_WIFI)
#if defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>
#else
#include <WiFiNINA_Generic.h>
#define ASTRO_USE_WIFI_STORAGE
#endif
#define ASTRO_USE_WIFI
#define ASTRO_USE_NET
#elif defined(ASTRO_ENABLE_AT_WIFI)
#include <WiFiEspAT.h>
#define ASTRO_USE_AT_WIFI
#define ASTRO_USE_WIFI
#define ASTRO_USE_NET
#elif defined(ASTRO_ENABLE_ETHERNET)
#include <Ethernet.h>
#define ASTRO_USE_ETHERNET
#define ASTRO_USE_NET
#endif

#ifndef ASTRO_DISABLE_MULTITASKING
#include <TaskManagerIO.h>
#include <IoAbstraction.h>
#define ASTRO_USE_MULTITASKING
#else
#ifndef ASTRO_DISABLE_GUI
#define ASTRO_DISABLE_GUI
#endif
#endif

#ifdef ASTRO_ENABLE_GPS
#include <Adafruit_GPS.h>
#define ASTRO_USE_GPS
typedef Adafruit_GPS GPSClass;
#endif

#include <ArduinoJson.h>
#include <ArxContainer.h>
#include <ArxSmartPtr.h>
#include <I2C_eeprom.h>
#include <RTClib.h>
#include <TimeLib.h>
#ifdef ASTRO_ENABLE_MQTT
#include <MQTT.h>
#define ASTRO_USE_MQTT
#endif
#ifndef ASTRO_DISABLE_GUI
#include <tcMenu.h>
#define ASTRO_USE_GUI
#endif

#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SPI))
#define ASTRO_USE_SPI &SPI
#else
#define ASTRO_USE_SPI nullptr
#endif
#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_TWOWIRE))
#define ASTRO_USE_WIRE &Wire
#else
#define ASTRO_USE_WIRE nullptr
#endif
#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SERIAL1))
#define ASTRO_USE_SERIAL1 &Serial1
#else
#define ASTRO_USE_SERIAL1 nullptr
#endif
#endif // /ifdef ARDUINO

#ifdef NDEBUG
#ifdef ASTRO_ENABLE_DEBUG_OUTPUT
#undef ASTRO_ENABLE_DEBUG_OUTPUT
#endif
#ifdef ASTRO_ENABLE_VERBOSE_DEBUG
#undef ASTRO_ENABLE_VERBOSE_DEBUG
#endif
#ifdef ASTRO_ENABLE_DEBUG_ASSERTIONS
#undef ASTRO_ENABLE_DEBUG_ASSERTIONS
#endif
#endif

#if defined(ASTRO_ENABLE_DEBUG_OUTPUT) && defined(ASTRO_ENABLE_VERBOSE_DEBUG)
#define ASTRO_USE_VERBOSE_OUTPUT
#endif
#if defined(ASTRO_ENABLE_DEBUG_OUTPUT) && defined(ASTRO_ENABLE_DEBUG_ASSERTIONS)
#define ASTRO_SOFT_ASSERT(cond,msg)     astroSoftAssert((bool)(cond), AstroString((msg)), __FILE__, __func__, __LINE__)
#define ASTRO_HARD_ASSERT(cond,msg)     astroHardAssert((bool)(cond), AstroString((msg)), __FILE__, __func__, __LINE__)
#define ASTRO_USE_DEBUG_ASSERTIONS
#else
#define ASTRO_SOFT_ASSERT(cond,msg)     ((void)0)
#define ASTRO_HARD_ASSERT(cond,msg)     ((void)0)
#endif

#include "AstroCompat.h"
#include "AstroDefines.h"
#include "AstroStrings.h"
#include "AstroCoordinates.h"
#include "AstroInlines.hh"
#include "AstroCallback.hh"
#include "AstroInterfaces.h"
#include "AstroActivation.h"
#include "AstroAttachments.h"
#include "AstroData.h"
#include "AstroObject.h"
#include "AstroMeasurements.h"
#include "AstroPins.h"
#include "AstroUtils.h"
#include "AstroDatas.h"
#include "AstroStreams.h"
#include "AstroTriggers.h"
#include "AstroDrivers.h"
#include "AstroActuators.h"
#include "AstroSensors.h"
#include "AstroMounts.h"
#include "AstroRails.h"
#include "AstroModules.h"
#include "AstroCamera.h"
#include "AstroCover.h"
#include "AstroThermal.h"
#include "AstroTargets.h"
#include "AstroEphemeris.h"
#include "AstroLib.h"
#include "AstroScheduler.h"
#include "AstroLogger.h"
#include "AstroPublisher.h"
#include "AstroFactory.h"
#include "AstroInterfaces.hpp"

// Astruino Controller
// Main controller interface for DIY astronomical tracking systems. Networking, GPS,
// displays, and external storage remain optional so a small offline system can use the
// same controller lifecycle as a more fully equipped build.
class Astruino : public AstroFactory, public AstroObjectRegistration {
public:
    AstroScheduler scheduler;                              // Scheduler public instance
    AstroLogger logger;                                    // Logger public instance
    AstroPublisher publisher;                              // Publisher public instance

    // Controller constructor. Typically called during class instantiation before setup().
    Astruino(Astro_MountType mountType = Astro_MountType_Equatorial,
             Astro_RTCType rtcType = Astro_RTCType_None,
             AstroDeviceSetup rtcSetup = AstroDeviceSetup());
    ~Astruino();

    // Initializes a default system and applies controller-level settings.
    void init(Astro_SystemMode systemMode = Astro_SystemMode_Tracking,
              Astro_MeasurementMode measurementMode = Astro_MeasurementMode_Metric); // Measurement mode

    // System Settings.
    void setSystemName(const char *systemName);
    const char *getSystemName() const;
    void setSystemMode(Astro_SystemMode systemMode);
    Astro_SystemMode getSystemMode() const;
    void setMeasurementMode(Astro_MeasurementMode measurementMode);
    Astro_MeasurementMode getMeasurementMode() const;
    void setTimeZoneOffset(int8_t hoursOffset);
    time_t getTimeZoneOffset() const;
#ifdef ARDUINO
    void setRTCTime(DateTime time);
#endif
    void setPollingInterval(uint16_t pollingInterval);
    uint16_t getPollingInterval() const;

    // Sets the fixed observer/location used for astronomical calculations.
    void setObserver(const AstroObserver &observer);
    inline const AstroObserver &getObserver() const { return _systemData.observer; }

    // Removes object from system, returning success.
    bool unregisterObject(SharedPtr<AstroObject> object);

    // Launches the controller into operational mode.
    void launch();
    // Suspends operational updates without discarding configured state.
    void suspend();
    // Updates the running system.
    void update();

    inline bool isInitialized() const { return _initialized; }
    inline bool isSuspended() const { return _suspended; }

    inline AstroMount &getMount() { return *_mount; }
    inline AstroCover &getCover() { return *_cover; }
    inline AstroCameraTrigger &getCamera() { return *_camera; }
    inline AstroThermalBalancer &getThermalBalancer() { return _thermal; }
    inline AstroScheduler &getScheduler() { return scheduler; }
    inline AstroLogger &getLogger() { return logger; }
    inline AstroPublisher &getPublisher() { return publisher; }
    inline AstroSystemData &getSystemData() { return _systemData; }
    inline const AstroSystemData &getSystemData() const { return _systemData; }

#ifdef ARDUINO
    // RTC device setup configuration.
    inline const AstroDeviceSetup &getRTCSetup() const { return _rtcSetup; }
    // Real time clock instance (lazily instantiated, nullptr return -> failure/no device).
    AstroRTCInterface *getRTC(bool begin = true);
    // Whenever the system booted with the RTC battery failure flag set.
    inline bool getRTCBatteryFailure() const { return _rtcBattFail; }
#endif

    // Returns the currently active Astruino controller instance, if any.
    static inline Astruino *getActiveInstance() { return _activeInstance; }

protected:
    static Astruino *_activeInstance;                       // Active controller instance

    AstroSystemData _systemData;                            // Serialized controller setup data
    SharedPtr<AstroMount> _mount;                           // Primary telescope/tracker mount
    SharedPtr<AstroCover> _cover;                           // Observatory/enclosure cover
    SharedPtr<AstroCameraTrigger> _camera;                  // Default observation trigger device
    AstroThermalBalancer _thermal;                          // Environmental/thermal balancer
    const Astro_RTCType _rtcType;                           // RTC device type
    const AstroDeviceSetup _rtcSetup;                       // RTC device setup
#ifdef ARDUINO
    AstroRTCInterface *_rtc;                                // Real time clock instance (owned, lazy)
#endif
    bool _rtcBegan;                                         // Status of RTC begin() call
    bool _rtcBattFail;                                      // Status of RTC battery failure flag
    bool _initialized;                                      // Initialization state flag
    bool _suspended;                                        // Operational suspension flag

    void allocateRTC();
    void deallocateRTC();

    void applySystemData();

    friend SharedPtr<AstroObjInterface> AstroDLinkObject::resolveObject();
};

// Returns the currently active controller instance.
extern Astruino *getController();
// Returns the active system logger, when a controller exists.
extern AstroLogger *getLogger();
// Returns the active data publisher, when a controller exists.
extern AstroPublisher *getPublisher();
// Returns the active scheduler, when a controller exists.
extern AstroScheduler *getScheduler();

#include "Astruino.hpp"

#endif // /ifndef Astruino_H
