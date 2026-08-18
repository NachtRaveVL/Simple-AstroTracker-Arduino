/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Observation Devices
*/

#ifndef AstroCamera_H
#define AstroCamera_H

#include "AstroObject.h"

// Observation Device Base
// Abstracts a camera, recorder, spectrometer, or other observing equipment.
class AstroObservationDevice : public AstroObservationDeviceInterface {
public:
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
    AstroCameraTrigger(const AstroObjectData *dataIn);

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
};

#endif // /ifndef AstroCamera_H
