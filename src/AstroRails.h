/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Power Rails
*/

#ifndef AstroRails_H
#define AstroRails_H

class AstroRail;
class AstroSimpleRail;
class AstroRegulatedRail;

struct AstroRailData;
struct AstroSimpleRailData;
struct AstroRegulatedRailData;

#include "Astroduino.h"

// Creates rail object from passed rail data (return ownership transfer - user code *must* delete returned object)
extern AstroRail *newRailObjectFromData(const AstroRailData *dataIn);


// Power Rail Base
// This is the base class for all power rails, which defines how the rail is identified,
// where it lives, what's attached to it, and who can activate under it.
class AstroRail : public AstroObject,
                  public AstroRailObjectInterface,
                  public AstroPowerUnitsInterfaceStorage {
public:
    const enum : signed char { Simple, Regulated, Unknown = -1 } classType; // Power rail class (custom RTTI)
    inline bool isSimpleClass() const { return classType == Simple; }
    inline bool isRegulatedClass() const { return classType == Regulated; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    AstroRail(Astro_RailType railType,
              aposi_t railIndex,
              int classType = Unknown);
    AstroRail(const AstroRailData *dataIn);
    virtual ~AstroRail();

    virtual void update() override;

    virtual bool addLinkage(AstroObject *obj) override;
    virtual bool removeLinkage(AstroObject *obj) override;

    inline Astro_RailType getRailType() const { return _id.objTypeAs.railType; }
    inline aposi_t getRailIndex() const { return _id.posIndex; }
    inline float getRailVoltage() const { return getRailVoltageFromType(getRailType()); }

    Signal<AstroRail *, ASTRO_RAIL_SIGNAL_SLOTS> &getCapacitySignal();

protected:
    Astro_TriggerState _limitState;                         // Limit state (last handled)

    Signal<AstroRail *, ASTRO_RAIL_SIGNAL_SLOTS> _capacitySignal; // Capacity changed signal

    virtual AstroData *allocateData() const override;
    virtual void saveToData(AstroData *dataOut) override;

    void handleLimit(Astro_TriggerState limitState);
    friend class AstroRegulatedRail;
};

// Simple Power Rail
// Basic power rail that tracks # of devices turned on, with a limit to how many
// can be on at the same time. Crude, but effective, especially when all devices
// along the rail will use about the same amount of power anyways.
class AstroSimpleRail : public AstroRail {
public:
    AstroSimpleRail(Astro_RailType railType,
                    aposi_t railIndex,
                    int maxActiveAtOnce = 2,
                    int classType = Simple);
    AstroSimpleRail(const AstroSimpleRailData *dataIn);

    virtual bool canActivate(AstroActuator *actuator) override;
    virtual float getCapacity(bool poll = false) override;

    virtual void setPowerUnits(Astro_UnitsType powerUnits) override;

    inline int getActiveCount() { return _activeCount; }

protected:
    int _activeCount;                                       // Current active count
    int _maxActiveAtOnce;                                   // Max active count

    virtual void saveToData(AstroData *dataOut) override;

    void handleActivation(AstroActuator *actuator);
    friend class AstroRail;
};

// Regulated Power Rail
// Power rail that has a max power rating and power sensor that can track power
// usage, with limit trigger for over-power state limiting actuator activation.
class AstroRegulatedRail : public AstroRail,
                           public AstroPowerUsageSensorAttachmentInterface,
                           public AstroLimitTriggerAttachmentInterface {
public:
    AstroRegulatedRail(Astro_RailType railType,
                       aposi_t railIndex,
                       float maxPower,
                       int classType = Regulated);
    AstroRegulatedRail(const AstroRegulatedRailData *dataIn);

    virtual void update() override;
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *obj) const override;

    virtual bool canActivate(AstroActuator *actuator) override;
    virtual float getCapacity(bool poll = false) override;

    virtual void setPowerUnits(Astro_UnitsType powerUnits) override;

    virtual AstroSensorAttachment &getPowerUsageSensorAttachment() override;

    virtual AstroTriggerAttachment &getLimitTriggerAttachment() override;

    inline float getMaxPower() const { return _maxPower; }

protected:
    float _maxPower;                                        // Maximum power
    AstroSensorAttachment _powerUsage;                      // Power usage sensor attachment
    AstroTriggerAttachment _limitTrigger;                   // Power limit trigger attachment

    virtual void saveToData(AstroData *dataOut) override;

    void handleActivation(AstroActuator *actuator);
    friend class AstroRail;

    void handlePower(const AstroMeasurement *measurement);
};


// Rail Serialization Data
struct AstroRailData : public AstroObjectData
{
    Astro_UnitsType powerUnits;                             // Power units

    AstroRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Simple Rail Serialization Data
struct AstroSimpleRailData : public AstroRailData
{
    int maxActiveAtOnce;                                    // Max active count

    AstroSimpleRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Regulated Rail Serialization Data
struct AstroRegulatedRailData : public AstroRailData
{
    float maxPower;                                         // Maximum power
    char powerUsageSensor[ASTRO_NAME_MAXSIZE];              // Power usage sensor
    AstroTriggerSubData limitTrigger;                       // Power limit trigger

    AstroRegulatedRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroRails_H
