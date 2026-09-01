/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets
*/

#include "Astruino.h"
#include "AstroCoreLogic.h"

AstroTarget *newTargetObjectFromData(const AstroTargetData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(AStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData() && dataIn->id.object.idType == (aid_t)AstroIdentity::Target) {
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


AstroTarget::AstroTarget(Astro_TargetType targetType, aposi_t targetIndex, int classTypeIn)
    : AstroObject(AstroIdentity(targetType, targetIndex)), classType((typeof(classType))classTypeIn),
      _targetsData(nullptr)
{
    allocateLinkages(ASTRO_TARGETS_LINKS_BASESIZE);
}

AstroTarget::AstroTarget(const AstroTargetData *dataIn)
    : AstroObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _targetsData(nullptr)
{
    allocateLinkages(ASTRO_TARGETS_LINKS_BASESIZE);
}

AstroTarget::~AstroTarget()
{
    if (_targetsData) { returnTargetsLibData(); }
}

void AstroTarget::update()
{
    AstroObject::update();
    checkoutTargetsLibData();
}

void AstroTarget::handleLowMemory()
{
    AstroObject::handleLowMemory();
    returnTargetsLibData();
}

const AstroTargetsLibData *AstroTarget::getTargetsLibData()
{
    checkoutTargetsLibData();
    return _targetsData;
}

AstroEquatorialCoordinates AstroTarget::getCoordinates(int64_t unixTime)
{
    const AstroTargetsLibData *targetData = getTargetsLibData();
    return targetData ? targetData->getCoordinates(unixTime) : AstroEquatorialCoordinates();
}

bool AstroTarget::isMovingTarget()
{
    const AstroTargetsLibData *targetData = getTargetsLibData();
    return targetData && targetData->movingTarget;
}

AstroData *AstroTarget::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void AstroTarget::saveToData(AstroData *dataOut)
{
    if (!dataOut) { return; }
    AstroObject::saveToData(dataOut);
    dataOut->id.object.classType = (int8_t)classType;
}

void AstroTarget::checkoutTargetsLibData()
{
    if (!_targetsData) {
        _targetsData = getTargetsLib()->checkoutTargetsData(getTargetType());
    }
}

void AstroTarget::returnTargetsLibData()
{
    if (_targetsData) {
        getTargetsLib()->returnTargetsData(_targetsData); _targetsData = nullptr;
    }
}


AstroStaticTarget::AstroStaticTarget(Astro_TargetType targetType, aposi_t targetIndex, int classType)
    : AstroTarget(targetType, targetIndex, classType)
{ ; }

AstroStaticTarget::AstroStaticTarget(const AstroStaticTargetData *dataIn)
    : AstroTarget(dataIn)
{ ; }


AstroDynamicTarget::AstroDynamicTarget(Astro_TargetType targetType, aposi_t targetIndex, int classType)
    : AstroTarget(targetType, targetIndex, classType)
{ ; }

AstroDynamicTarget::AstroDynamicTarget(const AstroDynamicTargetData *dataIn)
    : AstroTarget(dataIn)
{ ; }


AstroTargetData::AstroTargetData()
    : AstroObjectData()
{
    _size = sizeof(*this);
    id.object.idType = (aid_t)AstroIdentity::Target;
    id.object.objType = (aid_t)Astro_TargetType_Undefined;
    id.object.posIndex = aposi_none;
    id.object.classType = (aid_t)AstroTarget::Unknown;
}

AstroStaticTargetData::AstroStaticTargetData()
    : AstroTargetData()
{
    _size = sizeof(*this);
    id.object.classType = (aid_t)AstroTarget::Static;
}

AstroDynamicTargetData::AstroDynamicTargetData()
    : AstroTargetData()
{
    _size = sizeof(*this);
    id.object.classType = (aid_t)AstroTarget::Dynamic;
}


AstroTargetsLibData::AstroTargetsLibData()
    : AstroData('A','T','L','D'), targetType(Astro_TargetType_Undefined), targetClass(Astro_TargetClass_Unknown),
      catalogId{0}, targetName{0}, rightAscensionSeconds(0), declinationArcseconds(0),
      movingTarget(false)
{
    _size = sizeof(*this);
}

AstroEquatorialCoordinates AstroTargetsLibData::getCoordinates(int64_t unixTime) const
{
    if (movingTarget) {
        AstroEquatorialCoordinates coordinates;
        if (astroResolveSolarSystemTarget(targetType, unixTime, &coordinates)) { return coordinates; }
    }
    return astroPrecessJ2000(getJ2000Coordinates(), unixTime);
}

void AstroTargetsLibData::toJSONObject(JsonObject &objectOut) const
{
    AstroData::toJSONObject(objectOut);
    objectOut[SFP(AStr_Key_Id)] = targetTypeToString(targetType);
    if (catalogId[0]) { objectOut[SFP(AStr_Key_CatalogId)] = catalogId; }
    if (targetName[0]) { objectOut[SFP(AStr_Key_TargetName)] = targetName; }
    String targetClassStr = targetClassToString(targetClass, true);
    if (targetClassStr.length()) { objectOut[SFP(AStr_Key_TargetClass)] = targetClassStr; }
    if (rightAscensionSeconds) { objectOut[SFP(AStr_Key_RightAscensionSeconds)] = rightAscensionSeconds; }
    if (declinationArcseconds) { objectOut[SFP(AStr_Key_DeclinationArcseconds)] = declinationArcseconds; }
    if (movingTarget) { objectOut[SFP(AStr_Key_MovingTarget)] = movingTarget; }
}

void AstroTargetsLibData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroData::fromJSONObject(objectIn);

    targetType = targetTypeFromString((const char *)(objectIn[SFP(AStr_Key_Id)] | AStr_Blank));
    const char *catalogIdIn = objectIn[SFP(AStr_Key_CatalogId)] | nullptr;
    if (catalogIdIn) {
        strncpy(catalogId, catalogIdIn, sizeof(catalogId) - 1);
        catalogId[sizeof(catalogId) - 1] = '\0';
    }
    const char *targetNameIn = objectIn[SFP(AStr_Key_TargetName)] | nullptr;
    if (targetNameIn) {
        strncpy(targetName, targetNameIn, sizeof(targetName) - 1);
        targetName[sizeof(targetName) - 1] = '\0';
    }
    targetClass = targetClassFromString(String((const char *)(objectIn[SFP(AStr_Key_TargetClass)] | AStr_Blank)));
    rightAscensionSeconds = objectIn[SFP(AStr_Key_RightAscensionSeconds)] | rightAscensionSeconds;
    declinationArcseconds = objectIn[SFP(AStr_Key_DeclinationArcseconds)] | declinationArcseconds;
    movingTarget = objectIn[SFP(AStr_Key_MovingTarget)] | movingTarget;
    unsetModified();
}
