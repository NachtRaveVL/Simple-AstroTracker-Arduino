/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets
*/

#include "Astruino.h"
#include "AstroCoreLogic.h"
#include "AstroEphemeris.h"

AstroTarget *newTargetObjectFromData(const AstroTargetData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(AStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (aid_t)AstroTarget::Static:
                return new AstroStaticTarget((const AstroStaticTargetData *)dataIn);
            case (aid_t)AstroTarget::Dynamic:
                return new AstroDynamicTarget((const AstroDynamicTargetData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


AstroTarget::AstroTarget(Astro_TargetType targetType, aposi_t targetIndex, Astro_SubstrateType substrateType, DateTime sowTime, int classTypeIn)
    : AstroObject(AstroIdentity(targetType, targetIndex)), classType((typeof(classType))classTypeIn)
{
    allocateLinkages(ASTRO_TARGETS_LINKS_BASESIZE);

    //todo call recalc
}

AstroTarget::AstroTarget(const AstroTargetData *dataIn)
    : AstroObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _substrateType(dataIn->substrateType), _sowTime(dataIn->sowTime), _feedReservoir(this),
      _targetsData(nullptr), _growWeek(0), _feedingWeight(dataIn->feedingWeight),
      _targetPhase(Astro_TargetPhase_Undefined), _feedingState(Astro_TriggerState_NotTriggered)
{
    allocateLinkages(ASTRO_TARGETS_LINKS_BASESIZE);

    //todo call recalc
}

AstroTarget::~AstroTarget()
{
    if (_targetsData) { returnTargetsLibData(); }
}

void AstroTarget::update()
{
    AstroObject::update();

    //todo
}

void AstroTarget::handleLowMemory()
{
    AstroObject::handleLowMemory();

    returnTargetsLibData();
}

AstroData *AstroTarget::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void AstroTarget::saveToData(AstroData *dataOut)
{
    AstroObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    // todo
}

void AstroTarget::checkoutTargetsLibData()
{
    if (!_targetsData) {
        _targetsData = astroTargetsLib.checkoutTargetsData(_id.objTypeAs.targetType);
    }
}

void AstroTarget::returnTargetsLibData()
{
    if (_targetsData) {
        astroTargetsLib.returnTargetsData(_targetsData); _targetsData = nullptr;
    }
}

void AstroTarget::handleCustomTargetUpdated(Astro_TargetType targetType)
{
    if (getTargetType() == targetType) {
        returnTargetsLibData(); // forces re-checkout
        //todo call recalc

        if (getScheduler()) {
            getScheduler()->setNeedsScheduling();
        }
    }
}


AstroStaticTarget::AstroStaticTarget(Astro_TargetType targetType, aposi_t targetIndex, int classType)
    : AstroTarget(targetType, targetIndex, classType)
{ ; }

AstroStaticTarget::AstroStaticTarget(const AstroStaticTargetData *dataIn)
    : AstroTarget(dataIn)
{ ; }

void AstroStaticTarget::saveToData(AstroData *dataOut)
{
    AstroTarget::saveToData(dataOut);

    // todo: stuff like ((AstroStaticTargetData *)dataOut)->lastFeedingTime = _lastFeedingTime;
}


AstroTargetData::AstroTargetData()
    : AstroData('A','T','L','D'), targetId(Astro_Target_Undefined), targetClass(Astro_TargetClass_Unknown),
      catalogId{0}, targetName{0}, rightAscensionSeconds(0), declinationArcseconds(0),
      magnitudeCenti(32767), movingTarget(false)
{
    _size = sizeof(*this);
}

AstroEquatorialCoordinates AstroTargetData::getCoordinates(int64_t unixTime) const
{
    if (movingTarget) {
        AstroEquatorialCoordinates coordinates;
        if (astroResolveSolarSystemTarget(targetId, unixTime, &coordinates)) { return coordinates; }
    }
    return astroPrecessJ2000(getJ2000Coordinates(), unixTime);
}

void AstroTargetData::toJSONObject(JsonObject &objectOut) const
{
    AstroData::toJSONObject(objectOut);
    objectOut["catalogId"] = catalogId;
    objectOut["name"] = targetName;
    objectOut["targetId"] = (unsigned int)targetId;
    objectOut["class"] = (int)targetClass;
    objectOut["raSec"] = rightAscensionSeconds;
    objectOut["decArcSec"] = declinationArcseconds;
    objectOut["mag100"] = magnitudeCenti;
    objectOut["moving"] = movingTarget;
}

void AstroTargetData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroData::fromJSONObject(objectIn);

    unsigned int targetIdIn = objectIn["targetId"] | (unsigned int)targetId;
    if (targetIdIn >= Astro_Target_Count) { targetId = Astro_Target_Undefined; return; }
    targetId = (Astro_TargetId)targetIdIn;
    targetClass = (Astro_TargetClass)(objectIn["class"] | (int)targetClass);

    const char *catalogIdIn = objectIn["catalogId"] | nullptr;
    if (catalogIdIn) {
        strncpy(catalogId, catalogIdIn, sizeof(catalogId) - 1);
        catalogId[sizeof(catalogId) - 1] = '\0';
    }
    const char *targetNameIn = objectIn["name"] | nullptr;
    if (targetNameIn) {
        strncpy(targetName, targetNameIn, sizeof(targetName) - 1);
        targetName[sizeof(targetName) - 1] = '\0';
    }

    rightAscensionSeconds = objectIn["raSec"] | rightAscensionSeconds;
    declinationArcseconds = objectIn["decArcSec"] | declinationArcseconds;
    magnitudeCenti = objectIn["mag100"] | magnitudeCenti;
    movingTarget = objectIn["moving"] | movingTarget;
    unsetModified();
}
