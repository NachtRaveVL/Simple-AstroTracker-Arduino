/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Activations
*/

#include "Astruino.h"

AstroActivationHandle::AstroActivationHandle(SharedPtr<AstroActuator> actuatorIn, Astro_DirectionMode direction, float intensity, millis_t duration, bool force)
    : actuator(nullptr), activation(direction, constrain(intensity, 0.0f, 1.0f), duration, (force ? Astro_ActivationFlags_Forced : Astro_ActivationFlags_None)),
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

        if (actuator) { actuator->_handles.push_back(this); actuator->setNeedsUpdate(); }
    }
    return *this;
}

void AstroActivationHandle::unset()
{
    if (isActive()) { elapseTo(); }
    checkTime = 0;

    if (actuator) {
        for (auto handleIter = actuator->_handles.end() - 1; handleIter != actuator->_handles.begin() - 1; --handleIter) {
            if ((*handleIter) == this) {
                actuator->_handles.erase(handleIter);
                break;
            }
        }
        actuator->setNeedsUpdate();
        actuator = nullptr;
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
