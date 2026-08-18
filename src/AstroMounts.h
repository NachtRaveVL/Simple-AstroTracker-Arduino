/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Mounts
*/

#ifndef AstroMounts_H
#define AstroMounts_H

#include "AstroDrivers.h"
#include "AstroLib.h"
#include "AstroObject.h"

// Mount Axis State
// Stores one axis target, measured/estimated position, motion limit, and alignment state.
struct AstroAxisState {
    double positionDegrees;                                  // Current measured/estimated axis position, in degrees
    double targetDegrees;                                    // Current commanded axis target, in degrees
    double maxRateDegreesPerSecond;                          // Maximum simulated/open-loop slew rate, in degrees per second
    double minimumDegrees;                                   // Software minimum axis position, in degrees
    double maximumDegrees;                                   // Software maximum axis position, in degrees
    bool limitsEnabled;                                      // Software limit checking enabled flag

    AstroAxisState(double maxRate = ASTRO_MOUNT_AXIS_RATE_DEGPS)
        : positionDegrees(0.0), targetDegrees(0.0), maxRateDegreesPerSecond(maxRate),
          minimumDegrees(-180.0), maximumDegrees(180.0), limitsEnabled(false)
    { ; }

    inline bool withinLimits(double degrees) const {
        return !limitsEnabled || (degrees >= minimumDegrees && degrees <= maximumDegrees);
    }
};

// Telescope Mount
// Resolves astronomical targets into axis targets for equatorial, Alt/Az, or simple
// single-axis mounts. Position feedback is optional so the same class can be used with
// open-loop steppers, servos, encoders, or an external motor controller.
class AstroMount : public AstroObject, public AstroMountObjectInterface {
public:
    using AstroObject::update;

    typedef void (*AxisTargetCallback)(void *context, uint8_t axisIndex, double targetDegrees);
    typedef bool (*AxisPositionCallback)(void *context, uint8_t axisIndex, double *positionDegreesOut);

    AstroMount(Astro_MountType mountType = Astro_MountType_Equatorial,
               aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    AstroMount(const AstroObjectData *dataIn);

    void setObserver(const AstroObserver &observer);
    inline const AstroObserver &getObserver() const { return _observer; }

    virtual void setTarget(Astro_TargetId targetId) override;
    inline Astro_TargetId getTarget() const { return _targetId; }

    void setAxisRates(double primaryDegreesPerSecond, double secondaryDegreesPerSecond);
    void setAxisPosition(uint8_t axisIndex, double positionDegrees);
    void setAxisLimits(uint8_t axisIndex, double minimumDegrees, double maximumDegrees);
    void clearAxisLimits(uint8_t axisIndex);
    void setParkPosition(double primaryDegrees, double secondaryDegrees);
    inline void setStowPosition(double primaryDegrees, double secondaryDegrees) { setParkPosition(primaryDegrees, secondaryDegrees); }
    void setAxisDriver(uint8_t axisIndex, AstroAxisDriver *driver);
    AstroAxisDriver *getAxisDriver(uint8_t axisIndex) const;
    void setAxisTargetCallback(AxisTargetCallback callback, void *context = nullptr);
    void setAxisPositionCallback(AxisPositionCallback callback, void *context = nullptr);

    virtual void park() override;
    virtual void unpark() override;
    virtual void stow() override { park(); }
    virtual void track() override;

    // Applies a persistent guide correction to one mount axis. Duration is converted
    // into angular offset at the requested multiple of sidereal rate.
    void pulseGuide(uint8_t axisIndex, Astro_DirectionMode direction, uint32_t durationMillis,
                    double rateMultiple = ASTRO_MOUNT_GUIDE_RATE);
    void clearGuideOffset(uint8_t axisIndex = 0xff);

    void update(int64_t unixTime, double elapsedSeconds);

    virtual bool isAligned(double toleranceDegrees = ASTRO_SCH_ALIGN_TOL_DEG) const override;
    virtual bool isParked() const override { return _parked; }
    inline bool isParking() const { return _parking; }
    inline bool isTracking() const { return _tracking; }
    inline bool isLimitHit() const { return _limitHit; }
    inline Astro_MountType getMountType() const { return _mountType; }
    inline const AstroAxisState &getPrimaryAxis() const { return _primaryAxis; }
    inline const AstroAxisState &getSecondaryAxis() const { return _secondaryAxis; }

protected:
    Astro_MountType _mountType;                              // Mount type
    AstroObserver _observer;                                 // Observer/location data
    Astro_TargetId _targetId;                                // Active target ID
    AstroAxisState _primaryAxis;                             // Primary axis state
    AstroAxisState _secondaryAxis;                           // Secondary axis state
    double _parkPrimary;                                     // Park primary-axis position, in degrees
    double _parkSecondary;                                   // Park secondary-axis position, in degrees
    double _guidePrimary;                                    // Persistent primary-axis guide offset, in degrees
    double _guideSecondary;                                  // Persistent secondary-axis guide offset, in degrees
    bool _tracking;                                          // Tracking enabled flag
    bool _parking;                                           // Park movement active flag
    bool _parked;                                            // Parked state flag
    bool _limitHit;                                          // Software limit fault flag
    AstroAxisDriver *_primaryDriver;                         // Primary axis driver, not owned
    AstroAxisDriver *_secondaryDriver;                       // Secondary axis driver, not owned
    AxisTargetCallback _axisTargetCallback;                  // Axis target callback
    void *_axisTargetContext;                                // Axis target callback context, not owned
    AxisPositionCallback _axisPositionCallback;              // Axis position feedback callback
    void *_axisPositionContext;                              // Axis position callback context, not owned

    void updateTarget(int64_t unixTime, double elapsedSeconds);
    bool applyAxisTarget(uint8_t axisIndex, double targetDegrees);
    bool updateAxisPosition(uint8_t axisIndex);
    bool isAtParkPosition(double toleranceDegrees = ASTRO_SCH_ALIGN_TOL_DEG) const;
    static void moveAxis(AstroAxisState *axis, double elapsedSeconds, bool wrappedAxis = false);
};

#endif // /ifndef AstroMounts_H
