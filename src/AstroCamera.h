/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Camera
*/

#ifndef AstroCamera_H
#define AstroCamera_H

class AstroCover;
class AstroCamera;

struct AstroCameraSubData;

#include "Astruino.h"


// Equipment Cover
// Generic open/close mechanism for telescope caps, roof panels, dome shutters, or similar enclosures.
// Cover control is a mount-owned sub-object rather than a registered object family.
// Optional open/closed limit sensors override simulated travel so scheduler state reflects real hardware.
class AstroCover : public AstroSubObject {
public:
    AstroCover(AstroObjInterface *parent = nullptr);

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

    inline bool isConfigured() const { return _actuator.isSet() || _openSensor.isSet() || _closedSensor.isSet(); }
    bool isOpen() const;
    bool isClosed() const;
    bool isMoving() const;
    inline bool isAligned() const { return !_faulted && !isMoving(); }
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

// Camera Control
// Mount-owned camera timing controller that drives a standard CameraShutter actuator.
// Interval mode pulses the shutter at a fixed cadence, while Exposure mode holds the
// shutter active once for the requested bulb exposure duration.
class AstroCamera : public AstroSubObject {
public:
    enum : signed char { Interval, Exposure, Unknown = -1 };

    AstroCamera(AstroObjInterface *parent = nullptr, int mode = Unknown);
    AstroCamera(AstroObjInterface *parent, const AstroCameraSubData *dataIn);

    void update();
    virtual void unresolveAny(AstroObject *object) override;

    void startObservation();
    void stopObservation();
    bool ready() const;

    void setMode(int mode);
    void setInterval(millis_t intervalMillis);
    void setExposureTime(millis_t exposureMillis);
    void setShutterPulseTime(millis_t shutterPulseMillis);
    template<class U> inline void setShutter(U shutter) { _shutter.setObject(shutter, true); }

    inline int getMode() const { return _mode; }
    inline millis_t getInterval() const { return _intervalMillis; }
    inline millis_t getExposureTime() const { return _exposureMillis; }
    inline millis_t getShutterPulseTime() const { return _shutterPulseMillis; }
    inline bool isObserving() const { return _observing; }
    inline AstroActuatorAttachment &getShutterAttachment() { return _shutter; }

    void saveToData(AstroCameraSubData *dataOut) const;

protected:
    int8_t _mode;                                           // Camera timing mode
    millis_t _intervalMillis;                               // Interval between interval captures
    millis_t _exposureMillis;                               // Bulb exposure duration
    millis_t _shutterPulseMillis;                           // Interval-mode shutter pulse duration
    bool _observing;                                        // Observation sequence active flag
    millis_t _lastCapture;                                  // Last capture start time
    AstroActuatorAttachment _shutter;                       // Camera shutter actuator attachment

    void triggerShutter(millis_t duration);
};


// Camera Serialization Data
struct AstroCameraSubData : public AstroSubData {
    char shutterName[ASTRO_NAME_MAXSIZE];                   // Camera shutter actuator
    millis_t intervalMillis;                                // Interval capture cadence, in milliseconds
    millis_t exposureMillis;                                // Bulb exposure duration, in milliseconds
    millis_t shutterPulseMillis;                            // Interval-mode shutter pulse duration, in milliseconds

    AstroCameraSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef AstroCamera_H
