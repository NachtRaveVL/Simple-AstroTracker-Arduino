/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Drivers
*/

#include "AstroDrivers.h"

AstroCallbackAxisDriver::AstroCallbackAxisDriver(TargetCallback targetCallback,
                                                 StopCallback stopCallback,
                                                 void *context)
    : _targetCallback(targetCallback), _stopCallback(stopCallback), _positionCallback(nullptr),
      _context(context), _targetDegrees(0.0)
{ ; }

void AstroCallbackAxisDriver::setTargetDegrees(double targetDegrees)
{
    _targetDegrees = targetDegrees;
    if (_targetCallback) { _targetCallback(_context, targetDegrees); }
}

void AstroCallbackAxisDriver::stop()
{
    if (_stopCallback) { _stopCallback(_context); }
}

bool AstroCallbackAxisDriver::getPositionDegrees(double *positionDegreesOut) const
{
    return _positionCallback && positionDegreesOut ? _positionCallback(_context, positionDegreesOut) : false;
}

void AstroCallbackAxisDriver::setPositionCallback(PositionCallback positionCallback)
{
    _positionCallback = positionCallback;
}

AstroServoAxisDriver::AstroServoAxisDriver(AstroAnalogPin outputPin, double minDegrees, double maxDegrees)
    : _outputPin(outputPin), _minDegrees(minDegrees), _maxDegrees(maxDegrees), _targetDegrees(minDegrees)
{
    _outputPin.init();
}

void AstroServoAxisDriver::setTargetDegrees(double targetDegrees)
{
    _targetDegrees = astroConstrain(targetDegrees, _minDegrees, _maxDegrees);
    double range = _maxDegrees - _minDegrees;
    float amount = range > 0.0 ? (float)((_targetDegrees - _minDegrees) / range) : 0.0f;
    _outputPin.analogWrite(amount);
}

AstroStepDirAxisDriver::AstroStepDirAxisDriver(AstroDigitalPin stepPin,
                                               AstroDigitalPin directionPin,
                                               AstroDigitalPin enablePin,
                                               double stepsPerDegree,
                                               double maxStepsPerSecond,
                                               uint16_t pulseWidthMicros,
                                               uint16_t maxStepsPerUpdate)
    : _stepPin(stepPin), _directionPin(directionPin), _enablePin(enablePin),
      _stepsPerDegree(stepsPerDegree > 0.0 ? stepsPerDegree : 1.0),
      _maxStepsPerSecond(maxStepsPerSecond > 0.0 ? maxStepsPerSecond : 1.0),
      _stepRemainder(0.0), _targetDegrees(0.0), _positionSteps(0), _targetSteps(0),
      _lastUpdate(0), _pulseWidthMicros(pulseWidthMicros ? pulseWidthMicros : 1),
      _maxStepsPerUpdate(maxStepsPerUpdate ? maxStepsPerUpdate : 1)
{
    _stepPin.init();
    _directionPin.init();
    _enablePin.init();
    _stepPin.deactivate();
    _directionPin.deactivate();
    setEnabled(false);
}

int64_t AstroStepDirAxisDriver::degreesToSteps(double degrees) const
{
    const double steps = degrees * _stepsPerDegree;
    return (int64_t)(steps + (steps >= 0.0 ? 0.5 : -0.5));
}

void AstroStepDirAxisDriver::setTargetDegrees(double targetDegrees)
{
    _targetDegrees = targetDegrees;
    _targetSteps = degreesToSteps(targetDegrees);
    if (_targetSteps != _positionSteps) { setEnabled(true); }
}

void AstroStepDirAxisDriver::setPositionDegrees(double positionDegrees)
{
    _positionSteps = degreesToSteps(positionDegrees);
    _targetSteps = _positionSteps;
    _targetDegrees = (double)_targetSteps / _stepsPerDegree;
    _stepRemainder = 0.0;
    _lastUpdate = millis();
    setEnabled(false);
}

void AstroStepDirAxisDriver::setStepsPerDegree(double stepsPerDegree)
{
    if (stepsPerDegree <= 0.0) { return; }
    const double positionDegrees = (double)_positionSteps / _stepsPerDegree;
    const double targetDegrees = _targetDegrees;
    _stepsPerDegree = stepsPerDegree;
    _positionSteps = degreesToSteps(positionDegrees);
    _targetSteps = degreesToSteps(targetDegrees);
}

void AstroStepDirAxisDriver::setMaxStepsPerSecond(double maxStepsPerSecond)
{
    if (maxStepsPerSecond > 0.0) { _maxStepsPerSecond = maxStepsPerSecond; }
}

void AstroStepDirAxisDriver::setEnabled(bool enabled)
{
    if (!_enablePin.isValid()) { return; }
    if (enabled) { _enablePin.activate(); }
    else { _enablePin.deactivate(); }
}

void AstroStepDirAxisDriver::pulseStep()
{
    _stepPin.activate();
#ifdef ARDUINO
    delayMicroseconds(_pulseWidthMicros);
#endif
    _stepPin.deactivate();
}

void AstroStepDirAxisDriver::update()
{
    const millis_t now = millis();
    if (!_lastUpdate) {
        _lastUpdate = now;
        return;
    }

    const millis_t elapsed = now - _lastUpdate;
    _lastUpdate = now;
    if (!elapsed || _targetSteps == _positionSteps) {
        if (_targetSteps == _positionSteps) { setEnabled(false); }
        return;
    }

    double stepAllowance = (_maxStepsPerSecond * (double)elapsed / 1000.0) + _stepRemainder;
    uint32_t stepsToTake = (uint32_t)stepAllowance;
    _stepRemainder = stepAllowance - stepsToTake;
    if (!stepsToTake) { return; }
    if (stepsToTake > _maxStepsPerUpdate) { stepsToTake = _maxStepsPerUpdate; }

    const int64_t delta = _targetSteps - _positionSteps;
    const bool forward = delta > 0;
    uint64_t stepsRemaining = (uint64_t)(forward ? delta : -delta);
    if ((uint64_t)stepsToTake > stepsRemaining) { stepsToTake = (uint32_t)stepsRemaining; }

    if (forward) { _directionPin.activate(); }
    else { _directionPin.deactivate(); }
    setEnabled(true);

    for (uint32_t step = 0; step < stepsToTake; ++step) {
        pulseStep();
        _positionSteps += forward ? 1 : -1;
    }

    if (_positionSteps == _targetSteps) { setEnabled(false); }
}

void AstroStepDirAxisDriver::stop()
{
    _targetSteps = _positionSteps;
    _targetDegrees = (double)_positionSteps / _stepsPerDegree;
    _stepRemainder = 0.0;
    setEnabled(false);
}

bool AstroStepDirAxisDriver::getPositionDegrees(double *positionDegreesOut) const
{
    if (!positionDegreesOut || _stepsPerDegree <= 0.0) { return false; }
    *positionDegreesOut = (double)_positionSteps / _stepsPerDegree;
    return true;
}
