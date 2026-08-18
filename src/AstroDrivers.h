/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Drivers
*/

#ifndef AstroDrivers_H
#define AstroDrivers_H

#include "AstroPins.h"

// Axis Driver Base
// Abstracts the hardware used to move one telescope mount axis.
class AstroAxisDriver : public AstroDriverObjectInterface {
public:
    virtual ~AstroAxisDriver() { ; }

    virtual void setTargetDegrees(double targetDegrees) = 0;
    virtual void stop() = 0;
    virtual double getTargetDegrees() const = 0;
    virtual bool getPositionDegrees(double *positionDegreesOut) const { (void)positionDegreesOut; return false; }
};

// Callback Axis Driver
// Sends axis targets and stop requests through application supplied callbacks.
class AstroCallbackAxisDriver : public AstroAxisDriver {
public:
    typedef void (*TargetCallback)(void *context, double targetDegrees);
    typedef void (*StopCallback)(void *context);
    typedef bool (*PositionCallback)(void *context, double *positionDegreesOut);

    AstroCallbackAxisDriver(TargetCallback targetCallback = nullptr,
                            StopCallback stopCallback = nullptr,
                            void *context = nullptr);        // Context, not owned

    virtual void setTargetDegrees(double targetDegrees) override;
    virtual void stop() override;
    virtual double getTargetDegrees() const override { return _targetDegrees; }
    virtual bool getPositionDegrees(double *positionDegreesOut) const override;
    void setPositionCallback(PositionCallback positionCallback);

protected:
    TargetCallback _targetCallback;                          // Target update callback
    StopCallback _stopCallback;                              // Stop callback
    PositionCallback _positionCallback;                      // Optional position feedback callback
    void *_context;                                          // Callback context
    double _targetDegrees;                                   // Current target angle, in degrees
};

// Servo Axis Driver
// Maps a mount-axis target angle onto a normalized hobby-servo output.
class AstroServoAxisDriver : public AstroAxisDriver {
public:
    AstroServoAxisDriver(AstroAnalogPin outputPin = AstroAnalogPin(),
                         double minDegrees = 0.0, double maxDegrees = 180.0); // Min degrees

    virtual void setTargetDegrees(double targetDegrees) override;
    virtual void stop() override { ; }
    virtual double getTargetDegrees() const override { return _targetDegrees; }

protected:
    AstroAnalogPin _outputPin;                               // Output pin
    double _minDegrees;                                      // Minimum output angle, in degrees
    double _maxDegrees;                                      // Maximum output angle, in degrees
    double _targetDegrees;                                   // Current target angle, in degrees
};

#endif // /ifndef AstroDrivers_H
