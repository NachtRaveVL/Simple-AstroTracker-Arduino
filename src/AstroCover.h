/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Cover
*/

#ifndef AstroCover_H
#define AstroCover_H

#include "AstroActuators.h"

// Equipment Cover
// Generic open/close mechanism for telescope caps, roof panels, dome shutters, or similar enclosures.
class AstroCover : public AstroObject {
public:
    using AstroObject::update;

    AstroCover(aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroCover(const AstroObjectData *dataIn);

    void open();
    void close();
    void update(double elapsedSeconds);
    void setTravelRate(float fractionPerSecond);
    void setPosition(float position);
    void setActuator(AstroActuator *actuator);

    inline bool isOpen() const { return _position >= 0.999f; }
    inline bool isClosed() const { return _position <= 0.001f; }
    inline float getPosition() const { return _position; }

protected:
    float _position;                                         // Current normalized position
    float _target;                                           // Target normalized position
    float _travelRate;                                       // Normalized travel rate per second
    AstroActuator *_actuator;                                // Attached actuator, not owned
};

#endif // /ifndef AstroCover_H
