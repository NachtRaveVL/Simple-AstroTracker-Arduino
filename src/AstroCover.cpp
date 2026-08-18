/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Cover
*/

#include "AstroCover.h"

AstroCover::AstroCover(aposi_t positionIndex)
    : AstroObject(AstroIdentity(AstroIdentity::Cover, 0, positionIndex)), _position(0.0f), _target(0.0f), _travelRate(0.2f), _actuator(nullptr)
{ ; }

AstroCover::AstroCover(const AstroObjectData *dataIn)
    : AstroObject(dataIn), _position(0.0f), _target(0.0f), _travelRate(0.2f), _actuator(nullptr)
{ ; }

void AstroCover::open() { _target = 1.0f; }
void AstroCover::close() { _target = 0.0f; }
void AstroCover::setTravelRate(float fractionPerSecond) { _travelRate = fractionPerSecond > 0.0f ? fractionPerSecond : 0.0f; }
void AstroCover::setPosition(float position) { _position = position < 0.0f ? 0.0f : (position > 1.0f ? 1.0f : position); }
void AstroCover::setActuator(AstroActuator *actuator) { _actuator = actuator; }

void AstroCover::update(double elapsedSeconds)
{
    if (_actuator) { _actuator->setPower(_position < _target ? 1.0f : (_position > _target ? -1.0f : 0.0f)); }
    float step = _travelRate * (float)elapsedSeconds;
    if (_position < _target) { _position = _position + step > _target ? _target : _position + step; }
    else if (_position > _target) { _position = _position - step < _target ? _target : _position - step; }
    if (_actuator && _position == _target) { _actuator->setPower(0.0f); }
}
