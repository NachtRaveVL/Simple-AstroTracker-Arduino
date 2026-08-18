/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Cover
*/

#ifndef AstroCover_H
#define AstroCover_H

#include "AstroActuators.h"
#include "AstroSensors.h"

// Equipment Cover
// Generic open/close mechanism for telescope caps, roof panels, dome shutters, or similar enclosures.
// Optional open/closed limit sensors override simulated travel so scheduler state reflects real hardware.
class AstroCover : public AstroObject {
public:
    using AstroObject::update;

    AstroCover(aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroCover(const AstroObjectData *dataIn);

    void open();
    void close();
    void stop();
    void update(double elapsedSeconds);
    void setTravelRate(float fractionPerSecond);
    void setTravelTimeout(double seconds);
    void setPosition(float position);
    void setActuator(AstroActuator *actuator);
    void setOpenSensor(AstroSensor *sensor);
    void setClosedSensor(AstroSensor *sensor);
    void clearFault();

    bool isOpen() const;
    bool isClosed() const;
    bool isMoving() const;
    inline bool isFaulted() const { return _faulted; }
    inline float getPosition() const { return _position; }

protected:
    float _position;                                         // Current normalized position
    float _target;                                           // Target normalized position
    float _travelRate;                                       // Normalized travel rate per second
    double _travelTimeout;                                   // Maximum continuous travel time, in seconds
    double _travelElapsed;                                   // Current movement elapsed time, in seconds
    bool _openLimitActive;                                   // Last open-limit sensor state
    bool _closedLimitActive;                                 // Last closed-limit sensor state
    bool _faulted;                                           // Cover fault state flag
    AstroActuator *_actuator;                                // Attached actuator, not owned
    AstroSensor *_openSensor;                                // Optional open-limit sensor, not owned
    AstroSensor *_closedSensor;                              // Optional closed-limit sensor, not owned

    bool pollLimitSensor(AstroSensor *sensor, bool *activeOut);
    void applyActuatorPower(float power);
};

#endif // /ifndef AstroCover_H
