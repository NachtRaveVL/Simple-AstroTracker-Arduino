/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Actuators
*/

#ifndef AstroActuators_H
#define AstroActuators_H

struct AstroActuatorData;

#include "AstroActivation.h"
#include "AstroObject.h"
#include "AstroPins.h"

// Creates actuator object from passed actuator data (return ownership transfer - user code *must* delete returned object)
extern AstroActuator *newActuatorObjectFromData(const AstroActuatorData *dataIn);

// Actuator Base
// Base class for controllable outputs such as motors, relays, heaters, and other equipment.
class AstroActuator : public AstroObject, public AstroActuatorObjectInterface {
public:
    const enum : signed char { Base, Callback, Digital, RelayMotor, Analog, Focuser, Unknown = -1 } classType; // Actuator class type (custom RTTI)

    AstroActuator(Astro_ActuatorType actuatorType = Astro_ActuatorType_Undefined,
                  aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG,
                  int classTypeIn = Base); // Position index
    AstroActuator(const AstroActuatorData *dataIn);
    virtual ~AstroActuator() { ; }

    virtual void setPower(float power) override;
    virtual void update() override;

    bool addActivationHandle(AstroActivationHandle *handle);
    bool removeActivationHandle(AstroActivationHandle *handle);

    inline Astro_ActuatorType getActuatorType() const { return _actuatorType; }
    virtual float getPower() const override { return _power; }
    inline void setEnableMode(Astro_EnableMode mode) { if (_enableMode != mode) { _enableMode = mode; setNeedsUpdate(); } }
    inline Astro_EnableMode getEnableMode() const { return _enableMode; }
    inline void setNeedsUpdate() { _needsUpdate = true; }
    inline bool needsUpdate() const { return _needsUpdate; }

protected:
    Astro_ActuatorType _actuatorType;                        // Actuator type
    Astro_EnableMode _enableMode;                            // Activation combination mode
    float _power;                                            // Current normalized output power
    bool _needsUpdate;                                      // Stale flag for handle updates
    AstroActivationHandle *_handles[ASTRO_ACTIVATION_HANDLE_SLOTS]; // Activation handle slots

    virtual AstroData *allocateData() const override;
    virtual void saveToData(AstroData *dataOut) override;
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

    virtual void setPower(float power) override;

protected:
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

    virtual void setPower(float power) override;
    inline const AstroDigitalPin &getOutputPin() const { return _outputPin; }

protected:
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

    virtual void setPower(float power) override;
    inline const AstroDigitalPin &getForwardPin() const { return _forwardPin; }
    inline const AstroDigitalPin &getReversePin() const { return _reversePin; }

protected:
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

    virtual void setPower(float power) override;
    inline const AstroAnalogPin &getOutputPin() const { return _outputPin; }

protected:
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
    int32_t _maximumPosition;                               // Maximum allowed focuser position, in steps
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

    AstroActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroActuators_H
