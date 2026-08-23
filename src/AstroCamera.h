/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Observation Devices
*/

#ifndef AstroCamera_H
#define AstroCamera_H

struct AstroObservationDeviceData;

#include "AstroObject.h"

// Creates observation device object from passed data (return ownership transfer - user code *must* delete returned object)
extern AstroObject *newObservationDeviceObjectFromData(const AstroObservationDeviceData *dataIn);

// Observation Device Base
// Abstracts a camera, recorder, spectrometer, or other observing equipment.
class AstroObservationDevice : public AstroObservationDeviceInterface {
public:
    const enum : signed char { CameraTrigger, Unknown = -1 } classType; // Observation device class type (custom RTTI)

    AstroObservationDevice(int classTypeIn = Unknown) : classType((decltype(CameraTrigger))classTypeIn) { ; }
    virtual ~AstroObservationDevice() { ; }
    virtual bool ready() const = 0;
    virtual void startObservation() = 0;
    virtual void stopObservation() = 0;
};

// Camera Trigger
// Provides a simple start/stop observation signal suitable for shutters, relays, or adapters.
class AstroCameraTrigger : public AstroObject, public AstroObservationDevice {
public:
    typedef void (*TriggerCallback)(void *context, bool active);

    AstroCameraTrigger(TriggerCallback callback = nullptr, void *context = nullptr,
                       aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    AstroCameraTrigger(const AstroObservationDeviceData *dataIn);

    virtual bool ready() const override { return _ready; }
    virtual void startObservation() override;
    virtual void stopObservation() override;

    void setReady(bool ready);
    void setTriggerCallback(TriggerCallback callback, void *context = nullptr);
    inline bool isCapturing() const { return _capturing; }

protected:
    TriggerCallback _callback;                               // Callback function
    void *_context;                                          // Callback user context
    bool _ready;                                             // Device ready state
    bool _capturing;                                         // Observation active state

    virtual AstroData *allocateData() const override;
    virtual void saveToData(AstroData *dataOut) override;
};


// Observation Device Serialization Data
struct AstroObservationDeviceData : public AstroObjectData {
    AstroObservationDeviceData();
};

#endif // /ifndef AstroCamera_H
