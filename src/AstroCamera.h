/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Observation Devices
*/

#ifndef AstroCamera_H
#define AstroCamera_H

class AstroCover;
class AstroObservationDevice;
class AstroCameraTrigger;

struct AstroObservationDeviceData;

#include "Astruino.h"

// Creates observation device object from passed data (return ownership transfer - user code *must* delete returned object)
extern AstroObject *newObservationDeviceObjectFromData(const AstroObservationDeviceData *dataIn);


// Equipment Cover
// Generic open/close mechanism for telescope caps, roof panels, dome shutters, or similar enclosures.
// Cover control is part of the base Astruino system rather than a registered object family.
// Optional open/closed limit sensors override simulated travel so scheduler state reflects real hardware.
class AstroCover : public AstroSubObject {
public:
    AstroCover();

    void open();
    void close();
    void stop();
    void update();
    virtual void unresolveAny(AstroObject *object) override;
    void setTravelRate(float fractionPerSecond);
    void setTravelTimeout(double seconds);
    void setPosition(float position);
    template<class U> inline void setActuator(U actuator) { _actuator.setObject(actuator); }
    template<class U> inline void setOpenSensor(U sensor) { _openSensor.setObject(sensor); _openLimitActive = false; }
    template<class U> inline void setClosedSensor(U sensor) { _closedSensor.setObject(sensor); _closedLimitActive = false; }
    void clearFault();

    bool isOpen() const;
    bool isClosed() const;
    bool isMoving() const;
    inline bool isFaulted() const { return _faulted; }
    inline float getPosition() const { return _position; }

protected:
    float _position;                                        // Current normalized position
    float _target;                                          // Target normalized position
    float _travelRate;                                      // Normalized travel rate per second
    double _travelTimeout;                                  // Maximum continuous travel time, in seconds
    double _travelElapsed;                                  // Current movement elapsed time, in seconds
    bool _openLimitActive;                                  // Last open-limit sensor state
    bool _closedLimitActive;                                // Last closed-limit sensor state
    bool _faulted;                                          // Cover fault state flag
    AstroActuatorAttachment _actuator;                      // Attached actuator
    AstroSensorAttachment _openSensor;                      // Optional open-limit sensor attachment
    AstroSensorAttachment _closedSensor;                    // Optional closed-limit sensor attachment
    millis_t _lastUpdate;                                   // Last update time, in milliseconds

    bool pollLimitSensor(AstroSensorAttachment &sensor, bool *activeOut);
    void applyActuatorPower(float power);
};

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
