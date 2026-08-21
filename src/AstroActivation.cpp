/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Activations
*/

#include "AstroActivation.h"
#include "AstroActuators.h"

AstroActivationHandle::AstroActivationHandle(SharedPtr<AstroActuator> actuatorIn, Astro_DirectionMode direction,
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

AstroActivationHandle &AstroActivationHandle::operator=(SharedPtr<AstroActuator> actuatorIn)
{
    if (actuator != actuatorIn && isValid()) {
        if (actuator) { unset(); } else { checkTime = 0; }

        actuator = actuatorIn;

        if (actuator) {
            if (actuator->addActivationHandle(this)) {
                actuator->setNeedsUpdate();
            } else {
                actuator = nullptr;
            }
        }
    }
    return *this;
}

AstroActivationHandle &AstroActivationHandle::operator=(const AstroActivation &activationIn)
{
    activation = activationIn;
    return *this;
}

AstroActivationHandle &AstroActivationHandle::operator=(const AstroActivationHandle &handle)
{
    activation = handle.activation;
    return operator=(handle.actuator);
}

void AstroActivationHandle::unset()
{
    if (isActive()) { elapseTo(); }
    checkTime = 0;

    if (actuator) {
        SharedPtr<AstroActuator> oldActuator = actuator;
        actuator = nullptr;
        oldActuator->removeActivationHandle(this);
        oldActuator->setNeedsUpdate();
    }
}

void AstroActivationHandle::elapseBy(millis_t delta)
{
    if (delta && isValid() && isActive()) {
        if (!isUntimed()) {
            if (delta <= activation.duration) {
                activation.duration -= delta;
                checkTime += delta;
            } else {
                delta = activation.duration;
                activation.duration = 0;
                checkTime = 0;
                actuator->setNeedsUpdate();
            }
        }
        elapsed += delta;
    }
}

void AstroActivationHandle::elapseTo(millis_t time)
{
    if (isActive()) { elapseBy(time - checkTime); }
}
