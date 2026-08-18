/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Modules
*/

#ifndef AstroModules_H
#define AstroModules_H

#include "AstroCoordinates.h"
#include "AstroInlines.hh"

// Module Base
// Base lifecycle for optional hardware or service modules.
class AstroModule {
public:
    AstroModule();
    virtual ~AstroModule() { ; }

    virtual bool begin();
    virtual void update();

    inline bool isInitialized() const { return _initialized; }

protected:
    bool _initialized;                                      // Module initialization state
};

// Time Provider Interface
// Supplies UTC time without coupling tracking logic to RTC, GPS, NTP, or user code.
class AstroTimeProvider {
public:
    virtual ~AstroTimeProvider() { ; }
    virtual bool getUnixTime(int64_t *unixTimeOut) = 0;
};

// Location Provider Interface
// Supplies observer location without requiring GPS or networking.
class AstroLocationProvider {
public:
    virtual ~AstroLocationProvider() { ; }
    virtual bool getObserver(AstroObserver *observerOut) = 0;
};


// Manual Time Provider
// Stores a user supplied UTC time for systems that do not have an RTC, GPS, or network clock.
class AstroManualTimeProvider : public AstroTimeProvider {
public:
    AstroManualTimeProvider(int64_t unixTime = 0);

    virtual bool getUnixTime(int64_t *unixTimeOut) override;
    void setUnixTime(int64_t unixTime);
    inline int64_t getUnixTime() const { return _unixTime; }

protected:
    int64_t _unixTime;                                      // Stored UTC timestamp
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

// Callback Time Provider
// Supplies UTC time through user code.
class AstroCallbackTimeProvider : public AstroTimeProvider {
public:
    typedef bool (*TimeCallback)(void *context, int64_t *unixTimeOut);

    AstroCallbackTimeProvider(TimeCallback callback = nullptr, void *context = nullptr);
    virtual bool getUnixTime(int64_t *unixTimeOut) override;

protected:
    TimeCallback _callback;                                // Time provider callback
    void *_context;                                         // Callback user context
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
