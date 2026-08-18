/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Mounts
*/

#ifndef AstroMounts_H
#define AstroMounts_H

#include "AstroLib.h"
#include "AstroObject.h"

// Mount Axis State
// Stores one axis target, measured position, and alignment state.
struct AstroAxisState {
    double positionDegrees;                                  // Position degrees
    double targetDegrees;                                    // Target degrees
    double maxRateDegreesPerSecond;                          // Max rate degrees per second

    AstroAxisState(double maxRate = 4.0)
        : positionDegrees(0.0), targetDegrees(0.0), maxRateDegreesPerSecond(maxRate)
    { ; }
};

// Telescope Mount
// Resolves astronomical targets into axis targets for equatorial or Alt/Az mounts.
class AstroMount : public AstroObject, public AstroMountObjectInterface {
public:
    using AstroObject::update;

    typedef void (*AxisTargetCallback)(void *context, uint8_t axisIndex, double targetDegrees);
    AstroMount(Astro_MountType mountType = Astro_MountType_Equatorial,
               aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    AstroMount(const AstroObjectData *dataIn);

    void setObserver(const AstroObserver &observer);
    inline const AstroObserver &getObserver() const { return _observer; }

    virtual void setTarget(Astro_TargetId targetId) override;
    inline Astro_TargetId getTarget() const { return _targetId; }

    void setAxisRates(double primaryDegreesPerSecond, double secondaryDegreesPerSecond);
    void setStowPosition(double primaryDegrees, double secondaryDegrees);
    void setAxisTargetCallback(AxisTargetCallback callback, void *context = nullptr);
    virtual void stow() override;
    virtual void track() override;
    void update(int64_t unixTime, double elapsedSeconds);

    virtual bool isAligned(double toleranceDegrees = 0.25) const override;
    inline Astro_MountType getMountType() const { return _mountType; }
    inline bool isTracking() const { return _tracking; }
    inline const AstroAxisState &getPrimaryAxis() const { return _primaryAxis; }
    inline const AstroAxisState &getSecondaryAxis() const { return _secondaryAxis; }

protected:
    Astro_MountType _mountType;                              // Mount type
    AstroObserver _observer;                                 // Observer/location data
    Astro_TargetId _targetId;                                // Active target ID
    AstroAxisState _primaryAxis;                             // Primary axis
    AstroAxisState _secondaryAxis;                           // Secondary axis
    double _stowPrimary;                                     // Stow primary
    double _stowSecondary;                                   // Stow secondary
    bool _tracking;                                          // Tracking
    AxisTargetCallback _axisTargetCallback;                  // Axis target callback
    void *_axisTargetContext;                                // Axis target context, not owned

    void updateTarget(int64_t unixTime);
    static void moveAxis(AstroAxisState *axis, double elapsedSeconds);
};

#endif // /ifndef AstroMounts_H
