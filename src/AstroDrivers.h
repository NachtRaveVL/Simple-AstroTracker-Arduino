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
    virtual void update() { ; }
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

// Step/Direction Axis Driver
// Drives common STEP/DIR telescope axis hardware while keeping position in motor steps.
// Motion is rate limited and advanced from the normal controller update loop.
class AstroStepDirAxisDriver : public AstroAxisDriver {
public:
    AstroStepDirAxisDriver(AstroDigitalPin stepPin = AstroDigitalPin(),
                           AstroDigitalPin directionPin = AstroDigitalPin(),
                           AstroDigitalPin enablePin = AstroDigitalPin(),
                           double stepsPerDegree = 200.0,
                           double maxStepsPerSecond = 800.0,
                           uint16_t pulseWidthMicros = 4,
                           uint16_t maxStepsPerUpdate = 32);

    virtual void setTargetDegrees(double targetDegrees) override;
    virtual void stop() override;
    virtual void update() override;
    virtual double getTargetDegrees() const override { return _targetDegrees; }
    virtual bool getPositionDegrees(double *positionDegreesOut) const override;

    void setPositionDegrees(double positionDegrees);
    void setStepsPerDegree(double stepsPerDegree);
    void setMaxStepsPerSecond(double maxStepsPerSecond);
    inline double getStepsPerDegree() const { return _stepsPerDegree; }
    inline double getMaxStepsPerSecond() const { return _maxStepsPerSecond; }
    inline int64_t getPositionSteps() const { return _positionSteps; }
    inline int64_t getTargetSteps() const { return _targetSteps; }

protected:
    AstroDigitalPin _stepPin;                                // STEP output pin
    AstroDigitalPin _directionPin;                           // DIR output pin
    AstroDigitalPin _enablePin;                              // Optional ENABLE output pin
    double _stepsPerDegree;                                  // Motor steps per axis degree after gearing
    double _maxStepsPerSecond;                               // Maximum commanded step rate
    double _stepRemainder;                                   // Fractional step allowance carried between updates
    double _targetDegrees;                                   // Current target angle, in degrees
    int64_t _positionSteps;                                  // Estimated current motor position, in steps
    int64_t _targetSteps;                                    // Current motor target, in steps
    millis_t _lastUpdate;                                    // Last motion update time
    uint16_t _pulseWidthMicros;                              // STEP pulse high time
    uint16_t _maxStepsPerUpdate;                             // Maximum blocking step burst per controller update

    void setEnabled(bool enabled);
    void pulseStep();
    int64_t degreesToSteps(double degrees) const;
};

#endif // /ifndef AstroDrivers_H
