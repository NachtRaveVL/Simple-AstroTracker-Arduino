/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets
*/

#ifndef AstroTargets_H
#define AstroTargets_H

class AstroTarget;
class AstroStaticTarget;
class AstroDynamicTarget;

struct AstroTargetData;
struct AstroStaticTargetData;
struct AstroDynamicTargetData;
struct AstroTargetsLibData;

#include "Astruino.h"

// Creates target object from passed target data (return ownership transfer - user code *must* delete returned object)
extern AstroTarget *newTargetObjectFromData(const AstroTargetData *dataIn);


// Target Base
// Registered target selection that checks out the corresponding catalog/ephemeris entry
// from the targets library and exposes its resolved coordinates to the tracking system.
class AstroTarget : public AstroObject {
public:
    const enum : signed char { Static, Dynamic, Unknown = -1 } classType; // Target class type (custom RTTI)
    inline bool isStaticClass() const { return classType == Static; }
    inline bool isDynamicClass() const { return classType == Dynamic; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    AstroTarget(Astro_TargetType targetType,
                aposi_t targetIndex,
                int classType = Unknown);
    AstroTarget(const AstroTargetData *dataIn);
    virtual ~AstroTarget();

    virtual void update() override;
    virtual void handleLowMemory() override;

    inline Astro_TargetType getTargetType() const { return _id.objTypeAs.targetType; }
    inline aposi_t getTargetIndex() const { return _id.posIndex; }

    const AstroTargetsLibData *getTargetsLibData();
    AstroEquatorialCoordinates getCoordinates(int64_t unixTime);
    bool isMovingTarget();

protected:
    const AstroTargetsLibData *_targetsData;                // Targets library data (checked out if !nullptr)

    virtual AstroData *allocateData() const override;
    virtual void saveToData(AstroData *dataOut) override;

    void checkoutTargetsLibData();
    void returnTargetsLibData();
    friend class AstroTargetsLibrary;
};


// Static Target
// Catalog target whose coordinates are precessed from its stored J2000 position.
class AstroStaticTarget : public AstroTarget {
public:
    AstroStaticTarget(Astro_TargetType targetType,
                      aposi_t targetIndex,
                      int classType = Static);
    AstroStaticTarget(const AstroStaticTargetData *dataIn);
};


// Dynamic Target
// Moving catalog target whose coordinates are resolved from the current UTC time.
class AstroDynamicTarget : public AstroTarget {
public:
    AstroDynamicTarget(Astro_TargetType targetType,
                       aposi_t targetIndex,
                       int classType = Dynamic);
    AstroDynamicTarget(const AstroDynamicTargetData *dataIn);
};


// Target Serialization Data
struct AstroTargetData : public AstroObjectData {
    AstroTargetData();
};

// Static Target Serialization Data
struct AstroStaticTargetData : public AstroTargetData {
    AstroStaticTargetData();
};

// Dynamic Target Serialization Data
struct AstroDynamicTargetData : public AstroTargetData {
    AstroDynamicTargetData();
};


// Targets Library Data
// Serializable catalog entry containing target identity, display name, and coordinate data.
struct AstroTargetsLibData : public AstroData {
    Astro_TargetType targetType;                            // Target type
    Astro_TargetClass targetClass;                          // Target class
    char catalogId[ASTRO_TARGET_ID_MAXSIZE];                // Catalog/object ID
    char targetName[ASTRO_TARGET_NAME_MAXSIZE];             // Display target name
    int32_t rightAscensionSeconds;                          // J2000 right ascension, in seconds of time
    int32_t declinationArcseconds;                          // J2000 declination, in arcseconds
    bool movingTarget;                                      // Target requires time-dependent ephemeris resolution

    AstroTargetsLibData();

    inline AstroEquatorialCoordinates getJ2000Coordinates() const {
        return AstroEquatorialCoordinates((double)rightAscensionSeconds / 3600.0,
                                           (double)declinationArcseconds / 3600.0);
    }

    AstroEquatorialCoordinates getCoordinates(int64_t unixTime) const;
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroTargets_H
