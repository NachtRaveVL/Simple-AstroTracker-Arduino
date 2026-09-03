/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Actuators
*/

#ifndef AstroActuators_H
#define AstroActuators_H

class AstroActuator;
class AstroCallbackActuator;
class AstroDigitalActuator;
class AstroRelayMotorActuator;

struct AstroActuatorData;

#include "Astruino.h"

// Creates actuator object from passed actuator data (return ownership transfer - user code *must* delete returned object)
extern AstroActuator *newActuatorObjectFromData(const AstroActuatorData *dataIn);

// Actuator Base
// Base class for controllable outputs such as motors, relays, heaters, and other equipment.
class AstroActuator : public AstroObject,
                      public AstroActuatorObjectInterface,
                      public AstroParentRailAttachmentInterface,
                      public AstroParentMountAttachmentInterface {
public:
    const enum : signed char { Base, Callback, Digital, RelayMotor, Analog, Focuser, Unknown = -1 } classType; // Actuator class type (custom RTTI)

    AstroActuator(Astro_ActuatorType actuatorType = Astro_ActuatorType_Undefined,
                  aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG,
                  int classTypeIn = Base); // Position index
    AstroActuator(const AstroActuatorData *dataIn);
    virtual ~AstroActuator() { ; }

    virtual void update() override;

    virtual bool getCanEnable() override;
    virtual float getDriveIntensity() const override { return _power; }
    virtual bool isEnabled(float tolerance = 0.0f) const override { return _enabled && fabsf(_power) >= tolerance - FLT_EPSILON; }

    virtual void setContinuousPowerUsage(AstroSingleMeasurement contPowerUsage) override;
    virtual const AstroSingleMeasurement &getContinuousPowerUsage() override;
    virtual AstroAttachment &getParentRailAttachment() override;
    virtual AstroAttachment &getParentMountAttachment() override;

    Signal<AstroActuator *, ASTRO_ACTUATOR_SIGNAL_SLOTS> &getActivationSignal();

    inline AstroActivationHandle enableActuator(Astro_DirectionMode direction, float intensity = 1.0f, millis_t duration = (millis_t)-1, bool force = false) { return AstroActivationHandle(::getSharedPtr<AstroActuator>(this), direction, intensity, duration, force); }
    inline AstroActivationHandle enableActuator(millis_t duration = (millis_t)-1, bool force = false) { return enableActuator(Astro_DirectionMode_Forward, 1.0f, duration, force); }

    inline Astro_ActuatorType getActuatorType() const { return _actuatorType; }
    inline void setEnableMode(Astro_EnableMode mode) { if (_enableMode != mode) { _enableMode = mode; setNeedsUpdate(); bumpRevisionIfNeeded(); } }
    inline Astro_EnableMode getEnableMode() const { return _enableMode; }
    inline void setNeedsUpdate() { _needsUpdate = true; }
    inline bool needsUpdate() const { return _needsUpdate; }

protected:
    bool _enabled;                                          // Enabled state flag
    Astro_ActuatorType _actuatorType;                        // Actuator type
    Astro_EnableMode _enableMode;                            // Activation combination mode
    float _power;                                            // Current normalized output power
    bool _needsUpdate;                                      // Stale flag for handle updates
    Vector<AstroActivationHandle *> _handles;               // Activation handles array
    AstroSingleMeasurement _contPowerUsage;                 // Continuous power draw
    AstroAttachment _parentRail;                            // Parent power rail attachment
    AstroAttachment _parentMount;                           // Parent mount attachment
    Signal<AstroActuator *, ASTRO_ACTUATOR_SIGNAL_SLOTS> _activateSignal; // Activation update signal

    virtual AstroData *allocateData() const override;
    virtual void saveToData(AstroData *dataOut) override;

    virtual void _enableActuator(float intensity = 1.0f) override;
    virtual void _disableActuator() override;
    virtual void handleActivation();

    friend struct AstroActivationHandle;
};

// Callback Actuator
// Routes normalized actuator output through a application supplied callback.
class AstroCallbackActuator : public AstroActuator {
public:
    typedef void (*WriteCallback)(void *context, float power);

    AstroCallbackActuator(WriteCallback callback = nullptr, void *context = nullptr,
                          Astro_ActuatorType actuatorType = Astro_ActuatorType_Undefined,
                          aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG)
        : AstroActuator(actuatorType, positionIndex, Callback), _callback(callback), _context(context)
    { ; }
    AstroCallbackActuator(const AstroActuatorData *dataIn)
        : AstroActuator(dataIn), _callback(nullptr), _context(nullptr) { ; }

protected:
    virtual void _enableActuator(float intensity = 1.0f) override;
    virtual void _disableActuator() override;

    WriteCallback _callback;                                 // Callback function
    void *_context;                                          // Callback context
};

// Digital Actuator
// Drives a binary output pin from actuator activation state.
class AstroDigitalActuator : public AstroActuator {
public:
    AstroDigitalActuator(AstroDigitalPin outputPin = AstroDigitalPin(),
                         Astro_ActuatorType actuatorType = Astro_ActuatorType_Undefined,
                         aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    AstroDigitalActuator(const AstroActuatorData *dataIn);
    virtual ~AstroDigitalActuator();

    virtual bool getCanEnable() override;
    inline const AstroDigitalPin &getOutputPin() const { return _outputPin; }

protected:
    virtual void _enableActuator(float intensity = 1.0f) override;
    virtual void _disableActuator() override;

    AstroDigitalPin _outputPin;                              // Output pin

    virtual void saveToData(AstroData *dataOut) override;
};

// Relay Motor Actuator
// Drives a forward/reverse motor through two binary outputs, matching the signed power
// convention used by covers and other bidirectional equipment.
class AstroRelayMotorActuator : public AstroActuator {
public:
    AstroRelayMotorActuator(AstroDigitalPin forwardPin = AstroDigitalPin(),
                            AstroDigitalPin reversePin = AstroDigitalPin(),
                            Astro_ActuatorType actuatorType = Astro_ActuatorType_Undefined,
                            aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    AstroRelayMotorActuator(const AstroActuatorData *dataIn);
    virtual ~AstroRelayMotorActuator();

    virtual bool getCanEnable() override;
    inline const AstroDigitalPin &getForwardPin() const { return _forwardPin; }
    inline const AstroDigitalPin &getReversePin() const { return _reversePin; }

protected:
    virtual void _enableActuator(float intensity = 1.0f) override;
    virtual void _disableActuator() override;

    AstroDigitalPin _forwardPin;                             // Forward/open output pin
    AstroDigitalPin _reversePin;                             // Reverse/close output pin

    virtual void saveToData(AstroData *dataOut) override;
};

// Analog Actuator
// Drives a normalized variable output through an analog/PWM pin.
class AstroAnalogActuator : public AstroActuator {
public:
    AstroAnalogActuator(AstroAnalogPin outputPin = AstroAnalogPin(),
                        Astro_ActuatorType actuatorType = Astro_ActuatorType_Undefined,
                        aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    AstroAnalogActuator(const AstroActuatorData *dataIn);
    virtual ~AstroAnalogActuator();

    virtual bool getCanEnable() override;
    inline const AstroAnalogPin &getOutputPin() const { return _outputPin; }

protected:
    virtual void _enableActuator(float intensity = 1.0f) override;
    virtual void _disableActuator() override;

    AstroAnalogPin _outputPin;                               // Output pin

    virtual void saveToData(AstroData *dataOut) override;
};

// Telescope Focuser
// Absolute-position focuser wrapper intended for stepper focusers and external focus
// controllers. Positions are expressed as integer device steps, matching common focuser
// hardware and avoiding floating-point drift in repeated relative moves.
class AstroFocuser : public AstroActuator, public AstroFocuserObjectInterface {
public:
    typedef void (*MoveCallback)(void *context, int32_t targetPosition);
    typedef void (*StopCallback)(void *context);
    typedef bool (*PositionCallback)(void *context, int32_t *positionOut);

    AstroFocuser(int32_t maximumPosition = 10000,
                 aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroFocuser(const AstroActuatorData *dataIn, int32_t maximumPosition = 10000);

    virtual void update() override;
    virtual void moveTo(int32_t position) override;
    virtual void moveBy(int32_t steps) override;
    virtual void halt() override;

    void setMoveCallback(MoveCallback callback, void *context = nullptr);
    void setStopCallback(StopCallback callback);
    void setPositionCallback(PositionCallback callback);
    void setPosition(int32_t position);
    void setLimits(int32_t minimumPosition, int32_t maximumPosition);

    virtual int32_t getPosition() const override { return _position; }
    virtual int32_t getTargetPosition() const override { return _targetPosition; }
    virtual bool isMoving() const override { return _moving; }
    inline int32_t getMinimumPosition() const { return _minimumPosition; }
    inline int32_t getMaximumPosition() const { return _maximumPosition; }

protected:
    int32_t _position;                                      // Current measured/estimated focuser position, in steps
    int32_t _targetPosition;                                // Current focuser target position, in steps
    int32_t _minimumPosition;                               // Minimum allowed focuser position, in steps
    int32_t _maximumPosition;                               // Maximum focuser position, in steps
    bool _moving;                                           // Focuser movement active flag
    MoveCallback _moveCallback;                             // Absolute move callback
    StopCallback _stopCallback;                             // Stop/halt callback
    PositionCallback _positionCallback;                     // Optional position feedback callback
    void *_context;                                         // Callback context, not owned

    virtual void saveToData(AstroData *dataOut) override;
};

// Actuator Serialization Data
struct AstroActuatorData : public AstroObjectData {
    Astro_EnableMode enableMode;                            // Activation enablement mode
    AstroPinData outputPin;                                 // Primary output pin
    AstroPinData outputPin2;                                // Secondary motor output pin
    int32_t minimumPosition;                                // Minimum focuser position, in steps
    int32_t maximumPosition;                                // Maximum focuser position, in steps
    AstroMeasurementData contPowerUsage;                    // Continuous power usage
    char railName[ASTRO_NAME_MAXSIZE];                      // Parent rail
    char mountName[ASTRO_NAME_MAXSIZE];                     // Parent mount
    aposi_t mountAxisIndex;                                 // Parent mount axis index

    AstroActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroActuators_H
