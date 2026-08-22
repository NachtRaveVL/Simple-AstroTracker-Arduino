/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Modules
*/

#ifndef AstroModules_H
#define AstroModules_H

class AstroObject;
struct AstroCalibrationData;

#include "AstroCoordinates.h"
#include "AstroInlines.hh"
#include "AstroObject.h"

// Calibrations Storage
// Stores user calibration data, which calibrates sensor output into usable values.
class AstroCalibrations {
public:
    ~AstroCalibrations();
    void clearUserCalibrationData();
    // Adds/updates user calibration data to the store, returning success flag.
    bool setUserCalibrationData(const AstroCalibrationData *calibrationData);

    // Drops/removes user calibration data from the store, returning success flag.
    bool dropUserCalibrationData(const AstroCalibrationData *calibrationData);

    // Returns user calibration data instance in store.
    const AstroCalibrationData *getUserCalibrationData(akey_t key) const;

    // Returns if there are user calibrations in the store.
    inline bool hasUserCalibrations() const { return _calibrationData.size(); };

protected:
    Map<akey_t, AstroCalibrationData *, ASTRO_SYS_OBJECTS_MAXSIZE> _calibrationData; // Loaded user calibration data
};

// Object Registration Storage
// Stores objects in the main system store, which is used for SharedPtr<> lookups and
// stable attachment resolution in the same manner as the sibling controller libraries.
class AstroObjectRegistration {
public:
    // Adds object to system, returning success.
    bool registerObject(SharedPtr<AstroObject> object);
    // Removes object from system, returning success.
    bool unregisterObject(SharedPtr<AstroObject> object);

    // Searches for object by id key (nullptr return = no object by that id, position index may use ASTRO_POS_SEARCH* defines).
    SharedPtr<AstroObject> objectById(AstroIdentity id) const;

    // Finds first position either open or taken, given the identity type.
    aposi_t firstPosition(AstroIdentity id, bool taken) const;
    inline aposi_t firstPositionTaken(AstroIdentity id) const { return firstPosition(id, true); }
    inline aposi_t firstPositionOpen(AstroIdentity id) const { return firstPosition(id, false); }

    // Updates registered system objects.
    void updateObjects();

protected:
    Map<akey_t, SharedPtr<AstroObject>, ASTRO_SYS_OBJECTS_MAXSIZE> _objects; // Shared object collection, keyed by AstroIdentity

    SharedPtr<AstroObject> objectById_Col(const AstroIdentity &id) const;
};

// Location Provider Interface
// Supplies observer location without requiring GPS or networking.
class AstroLocationProvider {
public:
    virtual ~AstroLocationProvider() { ; }
    virtual bool getObserver(AstroObserver *observerOut) = 0;
};


// Fixed Location Provider
// Stores a known observer location for fully offline or permanently installed systems.
class AstroFixedLocationProvider : public AstroLocationProvider {
public:
    AstroFixedLocationProvider(const AstroObserver &observer = AstroObserver());

    virtual bool getObserver(AstroObserver *observerOut) override;
    void setObserver(const AstroObserver &observer);
    inline const AstroObserver &getObserver() const { return _observer; }

protected:
    AstroObserver _observer;                                // Stored observer/location data
};

// Callback Location Provider
// Supplies observer coordinates through user code.
class AstroCallbackLocationProvider : public AstroLocationProvider {
public:
    typedef bool (*LocationCallback)(void *context, AstroObserver *observerOut);

    AstroCallbackLocationProvider(LocationCallback callback = nullptr, void *context = nullptr);
    virtual bool getObserver(AstroObserver *observerOut) override;

protected:
    LocationCallback _callback;                            // Location provider callback
    void *_context;                                         // Callback user context
};

#endif // /ifndef AstroModules_H
