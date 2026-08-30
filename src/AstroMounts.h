/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Mounts
*/

#ifndef AstroMounts_H
#define AstroMounts_H

class AstroMount;

struct AstroMountData;

#include "Astruino.h"
#include "AstroCoreLogic.h"
#include "AstroDrivers.h"

// Creates mount object from passed mount data (return ownership transfer - user code *must* delete returned object)
extern AstroMount *newMountObjectFromData(const AstroMountData *dataIn);

// Axis State
// Runtime state for one mount axis.
struct AstroAxisState {
    double positionDegrees;                                 // Current axis position, in degrees
    double targetDegrees;                                   // Current axis target, in degrees
    double maxRateDegreesPerSecond;                         // Maximum simulated/limited axis rate
    double minimumDegrees;                                  // Minimum allowed axis position
    double maximumDegrees;                                  // Maximum allowed axis position
    bool limitsEnabled;                                     // Axis limit checking enabled

    AstroAxisState(double maxRate = ASTRO_MOUNT_AXIS_RATE_DEGPS)
        : positionDegrees(0.0), targetDegrees(0.0), maxRateDegreesPerSecond(maxRate),
          minimumDegrees(0.0), maximumDegrees(0.0), limitsEnabled(false)
    { ; }

    inline bool withinLimits(double position) const {
        return !limitsEnabled || (position >= minimumDegrees && position <= maximumDegrees);
    }
};

// Telescope Mount
// Base tracking object for equatorial, alt/azimuth, and single-axis mounts.
class AstroMount : public AstroObject,
                   public AstroMountObjectInterface {
public:
    const enum : signed char { Mount, Unknown = -1 } classType; // Mount class type (custom RTTI)

    AstroMount(Astro_MountType mountType = Astro_MountType_Equatorial,
               aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroMount(const AstroMountData *dataIn);

    void setObserver(const AstroObserver &observer);
    virtual void setTarget(Astro_TargetType targetType) override;
    void setAxisRates(double primaryDegreesPerSecond, double secondaryDegreesPerSecond);
    void setAxisPosition(uint8_t axisIndex, double positionDegrees);
    void setAxisLimits(uint8_t axisIndex, double minimumDegrees, double maximumDegrees);
    void clearAxisLimits(uint8_t axisIndex);
    void setParkPosition(double primaryDegrees, double secondaryDegrees = 0.0);
    void setAxisDriver(uint8_t axisIndex, SharedPtr<AstroAxisDriver> driver);
    SharedPtr<AstroAxisDriver> getAxisDriver(uint8_t axisIndex) const;

    virtual void park() override;
    virtual void unpark() override;
    inline virtual void stow() override { park(); }
    virtual void track() override;
    void pulseGuide(uint8_t axisIndex, Astro_DirectionMode direction,
                    uint32_t durationMillis, double rateMultiple = ASTRO_MOUNT_GUIDE_RATE);
    void clearGuideOffset(uint8_t axisIndex = 0xff);

    virtual void update() override;
    void notifyDateChanged();
    virtual bool isAligned(double toleranceDegrees = ASTRO_SCH_ALIGN_TOL_DEG) const override;
    virtual bool isParked() const override { return _parked; }

    inline Astro_MountType getMountType() const { return _mountType; }
    inline Astro_TargetType getTargetType() const { return _targetType; }
    inline bool isTracking() const { return _tracking; }
    inline bool isParking() const { return _parking; }
    inline bool isLimitHit() const { return _limitHit; }
    inline const AstroAxisState &getAxisState(uint8_t axisIndex) const { return axisIndex == 0 ? _primaryAxis : _secondaryAxis; }

protected:
    Astro_MountType _mountType;                             // Mount geometry
    AstroObserver _observer;                                // Observer location
    Astro_TargetType _targetType;                           // Active target
    AstroAxisState _primaryAxis;                            // Primary/right-ascension/azimuth axis
    AstroAxisState _secondaryAxis;                          // Secondary/declination/altitude axis
    double _parkPrimary;                                    // Primary park position
    double _parkSecondary;                                  // Secondary park position
    double _guidePrimary;                                   // Primary guide offset
    double _guideSecondary;                                 // Secondary guide offset
    bool _tracking;                                         // Tracking active flag
    bool _parking;                                          // Parking active flag
    bool _parked;                                           // Parked state flag
    bool _limitHit;                                         // Axis limit fault flag
    AstroAxisDriverAttachment _primaryDriver;               // Primary axis driver
    AstroAxisDriverAttachment _secondaryDriver;             // Secondary axis driver
    millis_t _lastUpdate;                                   // Last update time

    bool applyAxisTarget(uint8_t axisIndex, double targetDegrees);
    void updateTarget(int64_t unixTime, double elapsedSeconds);
    bool isAtParkPosition(double toleranceDegrees = ASTRO_SCH_ALIGN_TOL_DEG) const;
    bool updateAxisPosition(uint8_t axisIndex);
    void moveAxis(AstroAxisState *axis, double elapsedSeconds, bool wrappedAxis = false);

    virtual AstroData *allocateData() const override;
    virtual void saveToData(AstroData *dataOut) override;
};


// Mount Serialization Data
struct AstroMountData : public AstroObjectData {
    Astro_TargetType targetType;                            // Active target
    double primaryAxisRate;                                 // Primary axis rate limit, degrees per second
    double secondaryAxisRate;                               // Secondary axis rate limit, degrees per second
    double parkPrimary;                                     // Primary park position, degrees
    double parkSecondary;                                   // Secondary park position, degrees
    double primaryMinimum;                                  // Primary axis minimum, degrees
    double primaryMaximum;                                  // Primary axis maximum, degrees
    double secondaryMinimum;                                // Secondary axis minimum, degrees
    double secondaryMaximum;                                // Secondary axis maximum, degrees
    bool primaryLimitsEnabled;                              // Primary axis limits enabled
    bool secondaryLimitsEnabled;                            // Secondary axis limits enabled

    AstroMountData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroMounts_H
