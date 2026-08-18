/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Actuators
*/

#ifndef AstroActuators_H
#define AstroActuators_H

#include "AstroActivation.h"
#include "AstroObject.h"
#include "AstroPins.h"

// Actuator Base
// Base class for controllable outputs such as motors, relays, heaters, and other equipment.
class AstroActuator : public AstroObject, public AstroActuatorObjectInterface {
public:
    AstroActuator(Astro_ActuatorType actuatorType = Astro_ActuatorType_Generic,
                  aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    AstroActuator(const AstroObjectData *dataIn);
    virtual ~AstroActuator() { ; }

    virtual void setPower(float power) override;
    virtual void update() override;

    bool addActivationHandle(AstroActivationHandle *handle);
    bool removeActivationHandle(AstroActivationHandle *handle);
    void resolveActivations();

    inline Astro_ActuatorType getActuatorType() const { return _actuatorType; }
    virtual float getPower() const override { return _power; }
    inline void setEnableMode(Astro_EnableMode mode) { _enableMode = mode; }
    inline Astro_EnableMode getEnableMode() const { return _enableMode; }

protected:
    Astro_ActuatorType _actuatorType;                        // Actuator type
    Astro_EnableMode _enableMode;                            // Activation combination mode
    float _power;                                            // Current normalized output power
    AstroActivationHandle *_handles[ASTRO_ACTIVATION_HANDLE_SLOTS]; // Activation handle slots
};

// Callback Actuator
// Routes normalized actuator output through a user supplied callback.
class AstroCallbackActuator : public AstroActuator {
public:
    typedef void (*WriteCallback)(void *context, float power);

    AstroCallbackActuator(WriteCallback callback = nullptr, void *context = nullptr,
                          Astro_ActuatorType actuatorType = Astro_ActuatorType_Generic,
                          aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG)
        : AstroActuator(actuatorType, positionIndex), _callback(callback), _context(context)
    { ; }

    virtual void setPower(float power) override;

protected:
    WriteCallback _callback;                                 // Callback function
    void *_context;                                          // Callback user context
};

// Digital Actuator
// Drives a binary output pin from actuator activation state.
class AstroDigitalActuator : public AstroActuator {
public:
    AstroDigitalActuator(AstroDigitalPin outputPin = AstroDigitalPin(),
                         Astro_ActuatorType actuatorType = Astro_ActuatorType_Generic,
                         aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index

    virtual void setPower(float power) override;
    inline const AstroDigitalPin &getOutputPin() const { return _outputPin; }

protected:
    AstroDigitalPin _outputPin;                              // Output pin
};

// Analog Actuator
// Drives a normalized variable output through an analog/PWM pin.
class AstroAnalogActuator : public AstroActuator {
public:
    AstroAnalogActuator(AstroAnalogPin outputPin = AstroAnalogPin(),
                        Astro_ActuatorType actuatorType = Astro_ActuatorType_Generic,
                        aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index

    virtual void setPower(float power) override;
    inline const AstroAnalogPin &getOutputPin() const { return _outputPin; }

protected:
    AstroAnalogPin _outputPin;                               // Output pin
};

#endif // /ifndef AstroActuators_H
