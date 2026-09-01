/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Modules
*/

#ifndef AstroModules_H
#define AstroModules_H

class AstroCalibrations;
class AstroObjectRegistration;
class AstroPinHandlers;

#include "Astruino.h"
#include "AstroPins.h"

// Calibrations Storage
// Stores user calibration data, which calibrates the various sensors output to
// an usable input value.
class AstroCalibrations {
public:
    ~AstroCalibrations();

    // Adds/updates user calibration data to storage, returning success flag
    bool setUserCalibrationData(const AstroCalibrationData *calibrationData);

    // Drops/removes user calibration data from storage, returning success flag
    bool dropUserCalibrationData(const AstroCalibrationData *calibrationData);

    // Returns user calibration data instance in storage
    const AstroCalibrationData *getUserCalibrationData(akey_t key) const;

    // Returns if there are any user calibrations in storage
    inline bool hasUserCalibrations() const { return _calibrationData.size(); };

protected:
    Map<akey_t, AstroCalibrationData *, ASTRO_CAL_CALIBS_MAXSIZE> _calibrationData; // Loaded user calibration data
};


// Object Registration Storage
// Stores objects in main system store, which is used for SharedPtr<> lookups as well as
// notifying appropriate modules upon entry-to/exit-from the system.
class AstroObjectRegistration {
public:
    // Adds object to system, returning success
    bool registerObject(SharedPtr<AstroObject> obj);
    // Removes object from system, returning success
    bool unregisterObject(SharedPtr<AstroObject> obj);

    // Searches for object by id key (nullptr return = no obj by that id, position index may use ASTRO_POS_SEARCH* defines)
    SharedPtr<AstroObject> objectById(AstroIdentity id) const;

    // Finds first position either open or taken, given the id type
    aposi_t firstPosition(AstroIdentity id, bool taken);
    // Finds first position taken, given the id type
    inline aposi_t firstPositionTaken(AstroIdentity id) { return firstPosition(id, true); }
    // Finds first position open, given the id type
    inline aposi_t firstPositionOpen(AstroIdentity id) { return firstPosition(id, false); }

protected:
    Map<akey_t, SharedPtr<AstroObject>, ASTRO_SYS_OBJECTS_MAXSIZE> _objects; // Shared object collection, key'ed by AstroIdentity

    SharedPtr<AstroObject> objectById_Col(const AstroIdentity &id) const;
};


// Pin Handlers Storage
// Stores various pin-related system data on a shared pin # basis. Covers:
// - Pin locks: used for async shared resource management
// - Pin muxers: used for i/o pin multiplexing across a shared address bus
// - Pin expanders: used for i/o virtual pin expanding across an i2c interface
// - Pin OneWire: used for digital sensor pin's OneWire owner
class AstroPinHandlers {
public:
    // Attempts to get a lock on pin #, to prevent multi-device comm overlap (e.g. for OneWire comms).
    bool tryGetPinLock(pintype_t pin, millis_t wait = 150);
    // Returns a locked pin lock for the given pin. Only call if pin lock was successfully locked.
    inline void returnPinLock(pintype_t pin) { _pinLocks.erase(pin); }

    // Sets pin muxer for pin #.
    inline void setPinMuxer(pintype_t pin, SharedPtr<AstroPinMuxer> pinMuxer) { _pinMuxers[pin] = pinMuxer; }
    // Returns pin muxer for pin #.
    inline SharedPtr<AstroPinMuxer> getPinMuxer(pintype_t pin) { return _pinMuxers[pin]; }
    // Deactivates all pin muxers. Called before selecting another channel if pin muxers are assumed
    // to have a shared address bus (based on ASTRO_MUXERS_SHARED_ADDR_BUS setting).
    void deactivatePinMuxers();

#ifdef ASTRO_USE_MULTITASKING

    // Sets pin expander for index.
    inline void setPinExpander(aposi_t index, SharedPtr<AstroPinExpander> pinExpander) { _pinExpanders[index] = pinExpander; }
    // Returns expander for index.
    inline SharedPtr<AstroPinExpander> getPinExpander(aposi_t index) { return _pinExpanders[index]; }

#endif // /ifdef ASTRO_USE_MULTITASKING

    // OneWire instance for given pin (lazily instantiated)
    OneWire *getOneWireForPin(pintype_t pin);
    // Drops OneWire instance for given pin (if created)
    void dropOneWireForPin(pintype_t pin);

protected:
    Map<pintype_t, OneWire *, ASTRO_SYS_ONEWIRES_MAXSIZE> _pinOneWire; // Pin OneWire mapping
    Map<pintype_t, pintype_t, ASTRO_SYS_PINLOCKS_MAXSIZE> _pinLocks; // Pin locks mapping (existence = locked)
    Map<pintype_t, SharedPtr<AstroPinMuxer>, ASTRO_SYS_PINMUXERS_MAXSIZE> _pinMuxers; // Pin muxers mapping
#ifdef ASTRO_USE_MULTITASKING
    Map<aposi_t, SharedPtr<AstroPinExpander>, ASTRO_SYS_PINEXPANDERS_MAXSIZE> _pinExpanders; // Pin expanders mapping
#endif
};

#endif // /ifndef AstroModules_H
