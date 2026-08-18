/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Actuators
*/

#include "AstroActuators.h"
#include <math.h>

AstroActuator::AstroActuator(Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroObject(AstroIdentity(actuatorType, positionIndex)), _actuatorType(actuatorType),
      _enableMode(Astro_EnableMode_Highest), _power(0.0f), _handles{nullptr}
{ ; }

AstroActuator::AstroActuator(const AstroObjectData *dataIn)
    : AstroObject(dataIn), _actuatorType(dataIn ? (Astro_ActuatorType)dataIn->objType : Astro_ActuatorType_Undefined),
      _enableMode(Astro_EnableMode_Highest), _power(0.0f), _handles{nullptr}
{ ; }

void AstroActuator::setPower(float power)
{
    _power = astroConstrain(power, -1.0f, 1.0f);
}

bool AstroActuator::addActivationHandle(AstroActivationHandle *handle)
{
    if (!handle) { return false; }
    for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS; ++i) {
        if (_handles[i] == handle) { return true; }
        if (!_handles[i]) {
            _handles[i] = handle;
            return true;
        }
    }
    return false;
}

bool AstroActuator::removeActivationHandle(AstroActivationHandle *handle)
{
    for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS; ++i) {
        if (_handles[i] == handle) {
            for (size_t j = i; j + 1 < ASTRO_ACTIVATION_HANDLE_SLOTS; ++j) { _handles[j] = _handles[j + 1]; }
            _handles[ASTRO_ACTIVATION_HANDLE_SLOTS - 1] = nullptr;
            return true;
        }
    }
    return false;
}

void AstroActuator::resolveActivations()
{
    float resolved = 0.0f;
    float total = 0.0f;
    int count = 0;

    if (_enableMode == Astro_EnableMode_Multiply) { resolved = 1.0f; }

    for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ++i) {
        AstroActivationHandle *handle = _handles[i];
        if (!handle->isValid() || handle->isDone()) { continue; }
        float drive = handle->getDriveIntensity();

        switch (_enableMode) {
            case Astro_EnableMode_Highest:
                if (fabs(drive) > fabs(resolved)) { resolved = drive; }
                break;
            case Astro_EnableMode_Lowest:
                if (!count || fabs(drive) < fabs(resolved)) { resolved = drive; }
                break;
            case Astro_EnableMode_Average:
                total += drive;
                resolved = total / (float)(count + 1);
                break;
            case Astro_EnableMode_Multiply:
                resolved *= drive;
                break;
            case Astro_EnableMode_InOrder:
                if (!count) { resolved = drive; }
                break;
            case Astro_EnableMode_RevOrder:
                resolved = drive;
                break;
            default:
                break;
        }
        ++count;
    }

    if (!count) { resolved = 0.0f; }
    setPower(resolved);
}

void AstroActuator::update()
{
    millis_t now = astroMillis();
    for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ++i) {
        _handles[i]->elapseTo(now);
    }
    resolveActivations();
}

void AstroCallbackActuator::setPower(float power)
{
    AstroActuator::setPower(power);
    if (_callback) { _callback(_context, _power); }
}

AstroDigitalActuator::AstroDigitalActuator(AstroDigitalPin outputPin, Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroActuator(actuatorType, positionIndex), _outputPin(outputPin)
{
    _outputPin.init();
}

void AstroDigitalActuator::setPower(float power)
{
    AstroActuator::setPower(power);
    if (_outputPin.isValid()) {
        if (_power != 0.0f) { _outputPin.activate(); }
        else { _outputPin.deactivate(); }
    }
}

AstroAnalogActuator::AstroAnalogActuator(AstroAnalogPin outputPin, Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroActuator(actuatorType, positionIndex), _outputPin(outputPin)
{
    _outputPin.init();
}

void AstroAnalogActuator::setPower(float power)
{
    AstroActuator::setPower(power);
    if (_outputPin.isValid()) { _outputPin.analogWrite(_power < 0.0f ? -_power : _power); }
}
