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
{ ; }

void AstroServoAxisDriver::setTargetDegrees(double targetDegrees)
{
    _targetDegrees = astroConstrain(targetDegrees, _minDegrees, _maxDegrees);
    double range = _maxDegrees - _minDegrees;
    float amount = range > 0.0 ? (float)((_targetDegrees - _minDegrees) / range) : 0.0f;
    _outputPin.analogWrite(amount);
}

