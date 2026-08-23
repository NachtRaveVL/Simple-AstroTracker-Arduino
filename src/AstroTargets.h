/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets
*/

#ifndef AstroTargets_H
#define AstroTargets_H

class AstroTarget;
struct AstroTargetData;

#include "Astruino.h"

// Creates target object from passed target data (return ownership transfer - user code *must* delete returned object)
extern AstroTarget *newTargetObjectFromData(const AstroTargetData *dataIn);


// Target Base
// This is the base class for all targets, which defines how the target is identified, etc.
class AstroTarget : public AstroObject,
                    public AstroTargetObjectInterface {
public:
    const enum : signed char { Static, Dynamic, Unknown = -1 } classType; // Target class type (custom RTTI)
    inline bool isStaticClass() const { return classType == Static; }
    inline bool isDynamicClass() const { return classType == Dynamic; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    AstroTarget(Astro_TargetType targetType,
                aposi_t targetIndex,
                Astro_SubstrateType substrateType,
                DateTime sowTime,
                int classType = Unknown);
    AstroTarget(const AstroTargetData *dataIn);
    virtual ~AstroTarget();

    virtual void update() override;
    virtual void handleLowMemory() override;

    inline Astro_TargetType getTargetType() const { return _id.objTypeAs.targetType; }
    inline aposi_t getTargetIndex() const { return _id.posIndex; }

    inline const AstroTargetsLibData *getTargetsLibData() const { return _targetsData; }

protected:
    const AstroTargetsLibData *_targetsData;                    // Targets library data (checked out if !nullptr)
    
    virtual AstroData *allocateData() const override;
    virtual void saveToData(AstroData *dataOut) override;

    void checkoutTargetsLibData();
    void returnTargetsLibData();
    friend class AstroTargetsLibrary;

    void handleCustomTargetUpdated(Astro_TargetType targetType);
};


// Simple Static Target
// Standard target object that stays relatively static.
class AstroStaticTarget : public AstroTarget {
public:
    AstroStaticTarget(Astro_TargetType targetType,
                   aposi_t targetIndex,
                   int classType = Static);
    AstroStaticTarget(const AstroStaticTargetData *dataIn);

    virtual bool needsFeeding(bool poll = false) override;
    virtual void notifyFeedingBegan() override;

    void setFeedTimeOn(TimeSpan timeOn);
    inline TimeSpan getFeedTimeOn() const { return TimeSpan(_feedTimingMins[0] * SECS_PER_MIN); }

    void setFeedTimeOff(TimeSpan timeOff);
    inline TimeSpan getFeedTimeOff() const { return TimeSpan(_feedTimingMins[1] * SECS_PER_MIN); }

    void setFeedingsPerDay(uint8_t feedingsPerDay);
    inline uint8_t getFeedingsPerDay() const { return _feedingsPerDay; }

    void setFeedingsPerWeek(uint8_t feedingsPerWeek);
    inline uint8_t getFeedingsPerWeek() const { return _feedingsPerWeek; }

    void setFeedInterval(TimeSpan feedInterval);
    inline TimeSpan getFeedInterval() const { return TimeSpan(_feedIntervalMins * SECS_PER_MIN); }

    inline Astro_FeedingSchedule getFeedingSchedule() const {
        return _feedingsPerDay ? Astro_FeedingSchedule_Daily :
               (_feedingsPerWeek ? Astro_FeedingSchedule_Weekly : Astro_FeedingSchedule_Interval);
    }

protected:
    time_t _lastFeedingTime;                                // Last feeding date (UTC)
    uint8_t _feedTimingMins[2];                             // Feed duration + legacy off-time, in minutes
    uint8_t _feedingsPerDay;                               // Number of feedings per day (0 when unused)
    uint8_t _feedingsPerWeek;                              // Number of feedings per week (0 when unused)
    uint16_t _feedIntervalMins;                            // Elapsed feed interval, in minutes (0 when unused)

    virtual void saveToData(AstroData *dataOut) override;
};


// Astronomy Target Data
// Serializable catalog entry containing identity, display name, and coordinate data.
struct AstroTargetData : public AstroData {
    Astro_TargetId targetId;                                 // Target ID
    Astro_TargetClass targetClass;                           // Target class
    char catalogId[ASTRO_TARGET_ID_MAXSIZE];                 // Catalog/object ID
    char targetName[ASTRO_TARGET_NAME_MAXSIZE];              // Display target name
    int32_t rightAscensionSeconds;                           // J2000 right ascension, in seconds of time
    int32_t declinationArcseconds;                           // J2000 declination, in arcseconds
    int16_t magnitudeCenti;                                  // Visual magnitude multiplied by 100
    bool movingTarget;                                       // Target requires time-dependent ephemeris resolution

    AstroTargetData();

    inline AstroEquatorialCoordinates getJ2000Coordinates() const {
        return AstroEquatorialCoordinates((double)rightAscensionSeconds / 3600.0,
                                           (double)declinationArcseconds / 3600.0);
    }

    AstroEquatorialCoordinates getCoordinates(int64_t unixTime) const;
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroTargets_H
