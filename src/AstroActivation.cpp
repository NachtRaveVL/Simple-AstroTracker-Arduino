/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Activations
*/

#include "AstroActivation.h"
#include "AstroActuators.h"

AstroActivationHandle::AstroActivationHandle(AstroActuator *actuatorIn, Astro_DirectionMode direction,
                                             float intensity, millis_t duration, bool force)
    : actuator(nullptr),
      activation(direction, intensity, duration, force ? Astro_ActivationFlags_Forced : Astro_ActivationFlags_None),
      checkTime(0), elapsed(0)
{
    operator=(actuatorIn);
}

AstroActivationHandle::AstroActivationHandle(const AstroActivationHandle &handle)
    : actuator(nullptr), activation(handle.activation), checkTime(0), elapsed(0)
{
    operator=(handle.actuator);
}

AstroActivationHandle::~AstroActivationHandle()
{
    if (actuator) { unset(); }
}

AstroActivationHandle &AstroActivationHandle::operator=(AstroActuator *actuatorIn)
{
    if (actuator != actuatorIn && isValid()) {
        if (actuator) { unset(); }
        actuator = actuatorIn;
        if (actuator && actuator->addActivationHandle(this)) { checkTime = astroNZMillis(); }
        else if (actuator) { actuator = nullptr; }
    }
    return *this;
}

AstroActivationHandle &AstroActivationHandle::operator=(const AstroActivation &activationIn)
{
    activation = activationIn;
    if (actuator && !checkTime) { checkTime = astroNZMillis(); }
    return *this;
}

AstroActivationHandle &AstroActivationHandle::operator=(const AstroActivationHandle &handle)
{
    activation = handle.activation;
    elapsed = 0;
    return operator=(handle.actuator);
}

void AstroActivationHandle::unset()
{
    if (isActive()) { elapseTo(); }
    checkTime = 0;
    if (actuator) {
        AstroActuator *oldActuator = actuator;
        actuator = nullptr;
        oldActuator->removeActivationHandle(this);
        oldActuator->resolveActivations();
    }
}

void AstroActivationHandle::elapseBy(millis_t delta)
{
    if (!delta || !isValid() || !isActive()) { return; }
    if (!isUntimed()) {
        if (delta < activation.duration) {
            activation.duration -= delta;
            checkTime += delta;
        } else {
            delta = activation.duration;
            activation.duration = 0;
            checkTime = 0;
        }
    } else {
        checkTime += delta;
    }
    elapsed += delta;
}

void AstroActivationHandle::elapseTo(millis_t time)
{
    if (isActive()) { elapseBy(time - checkTime); }
}
