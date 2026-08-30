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
        _targetsData = AstroTargetsLib.checkoutTargetsData(getTargetType());
    }
}

void AstroTarget::returnTargetsLibData()
{
    if (_targetsData) {
        AstroTargetsLib.returnTargetsData(_targetsData); _targetsData = nullptr;
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

static AstroString astroTargetTypeToCatalogId(Astro_TargetType targetType)
{
    switch (targetType) {
        case Astro_TargetType_Sun: return SFP(AStr_Sun);
        case Astro_TargetType_Moon: return SFP(AStr_Moon);
        case Astro_TargetType_Mercury: return SFP(AStr_Mercury);
        case Astro_TargetType_Venus: return SFP(AStr_Venus);
        case Astro_TargetType_Mars: return SFP(AStr_Mars);
        case Astro_TargetType_Jupiter: return SFP(AStr_Jupiter);
        case Astro_TargetType_Saturn: return SFP(AStr_Saturn);
        case Astro_TargetType_Uranus: return SFP(AStr_Uranus);
        case Astro_TargetType_Neptune: return SFP(AStr_Neptune);
        case Astro_TargetType_Sirius: return SFP(AStr_Sirius);
        case Astro_TargetType_Canopus: return SFP(AStr_Canopus);
        case Astro_TargetType_Arcturus: return SFP(AStr_Arcturus);
        case Astro_TargetType_Vega: return SFP(AStr_Vega);
        case Astro_TargetType_Capella: return SFP(AStr_Capella);
        case Astro_TargetType_RigelKentaurus: return SFP(AStr_RigelKentaurus);
        case Astro_TargetType_Procyon: return SFP(AStr_Procyon);
        case Astro_TargetType_Betelgeuse: return SFP(AStr_Betelgeuse);
        case Astro_TargetType_Achernar: return SFP(AStr_Achernar);
        case Astro_TargetType_Hadar: return SFP(AStr_Hadar);
        case Astro_TargetType_Altair: return SFP(AStr_Altair);
        case Astro_TargetType_Acrux: return SFP(AStr_Acrux);
        case Astro_TargetType_Aldebaran: return SFP(AStr_Aldebaran);
        case Astro_TargetType_Spica: return SFP(AStr_Spica);
        case Astro_TargetType_Antares: return SFP(AStr_Antares);
        case Astro_TargetType_Pollux: return SFP(AStr_Pollux);
        case Astro_TargetType_Fomalhaut: return SFP(AStr_Fomalhaut);
        case Astro_TargetType_Deneb: return SFP(AStr_Deneb);
        case Astro_TargetType_Regulus: return SFP(AStr_Regulus);
        case Astro_TargetType_Polaris: return SFP(AStr_Polaris);
        case Astro_TargetType_Castor: return SFP(AStr_Castor);
        case Astro_TargetType_Bellatrix: return SFP(AStr_Bellatrix);
        case Astro_TargetType_Alnilam: return SFP(AStr_Alnilam);
        case Astro_TargetType_Alnitak: return SFP(AStr_Alnitak);
        case Astro_TargetType_Mizar: return SFP(AStr_Mizar);
        case Astro_TargetType_Dubhe: return SFP(AStr_Dubhe);
        default: break;
    }

    if (targetType >= Astro_TargetType_M1 && targetType <= Astro_TargetType_M110) {
        return SFP(AStr_MessierPrefix) + astroPositionIndexToString((aposi_t)((int)targetType - (int)Astro_TargetType_M1 + 1));
    }
    if (targetType >= Astro_TargetType_CustomTarget1 && targetType <= Astro_TargetType_CustomTarget8) {
        return SFP(AStr_CustomTargetPrefix) + astroPositionIndexToString((aposi_t)((int)targetType - (int)Astro_TargetType_CustomTarget1 + 1));
    }
    return AstroString();
}

static Astro_TargetType astroTargetTypeFromCatalogId(const AstroString &catalogId)
{
    if (catalogId == SFP(AStr_Sun)) return Astro_TargetType_Sun;
    if (catalogId == SFP(AStr_Moon)) return Astro_TargetType_Moon;
    if (catalogId == SFP(AStr_Mercury)) return Astro_TargetType_Mercury;
    if (catalogId == SFP(AStr_Venus)) return Astro_TargetType_Venus;
    if (catalogId == SFP(AStr_Mars)) return Astro_TargetType_Mars;
    if (catalogId == SFP(AStr_Jupiter)) return Astro_TargetType_Jupiter;
    if (catalogId == SFP(AStr_Saturn)) return Astro_TargetType_Saturn;
    if (catalogId == SFP(AStr_Uranus)) return Astro_TargetType_Uranus;
    if (catalogId == SFP(AStr_Neptune)) return Astro_TargetType_Neptune;
    if (catalogId == SFP(AStr_Sirius)) return Astro_TargetType_Sirius;
    if (catalogId == SFP(AStr_Canopus)) return Astro_TargetType_Canopus;
    if (catalogId == SFP(AStr_Arcturus)) return Astro_TargetType_Arcturus;
    if (catalogId == SFP(AStr_Vega)) return Astro_TargetType_Vega;
    if (catalogId == SFP(AStr_Capella)) return Astro_TargetType_Capella;
    if (catalogId == SFP(AStr_RigelKentaurus)) return Astro_TargetType_RigelKentaurus;
    if (catalogId == SFP(AStr_Procyon)) return Astro_TargetType_Procyon;
    if (catalogId == SFP(AStr_Betelgeuse)) return Astro_TargetType_Betelgeuse;
    if (catalogId == SFP(AStr_Achernar)) return Astro_TargetType_Achernar;
    if (catalogId == SFP(AStr_Hadar)) return Astro_TargetType_Hadar;
    if (catalogId == SFP(AStr_Altair)) return Astro_TargetType_Altair;
    if (catalogId == SFP(AStr_Acrux)) return Astro_TargetType_Acrux;
    if (catalogId == SFP(AStr_Aldebaran)) return Astro_TargetType_Aldebaran;
    if (catalogId == SFP(AStr_Spica)) return Astro_TargetType_Spica;
    if (catalogId == SFP(AStr_Antares)) return Astro_TargetType_Antares;
    if (catalogId == SFP(AStr_Pollux)) return Astro_TargetType_Pollux;
    if (catalogId == SFP(AStr_Fomalhaut)) return Astro_TargetType_Fomalhaut;
    if (catalogId == SFP(AStr_Deneb)) return Astro_TargetType_Deneb;
    if (catalogId == SFP(AStr_Regulus)) return Astro_TargetType_Regulus;
    if (catalogId == SFP(AStr_Polaris)) return Astro_TargetType_Polaris;
    if (catalogId == SFP(AStr_Castor)) return Astro_TargetType_Castor;
    if (catalogId == SFP(AStr_Bellatrix)) return Astro_TargetType_Bellatrix;
    if (catalogId == SFP(AStr_Alnilam)) return Astro_TargetType_Alnilam;
    if (catalogId == SFP(AStr_Alnitak)) return Astro_TargetType_Alnitak;
    if (catalogId == SFP(AStr_Mizar)) return Astro_TargetType_Mizar;
    if (catalogId == SFP(AStr_Dubhe)) return Astro_TargetType_Dubhe;

    const char *text = catalogId.c_str();
    if (text && text[0] == 'M' && text[1]) {
        long number = strtol(text + 1, nullptr, 10);
        if (number >= 1 && number <= 110) {
            return (Astro_TargetType)((int)Astro_TargetType_M1 + number - 1);
        }
    }
    AstroString customPrefix = SFP(AStr_CustomTargetPrefix);
    if (text && !strncmp(text, customPrefix.c_str(), customPrefix.length())) {
        long number = strtol(text + customPrefix.length(), nullptr, 10);
        if (number >= 1 && number <= 8) {
            return (Astro_TargetType)((int)Astro_TargetType_CustomTarget1 + number - 1);
        }
    }
    return Astro_TargetType_Undefined;
}

void AstroTargetsLibData::toJSONObject(JsonObject &objectOut) const
{
    AstroData::toJSONObject(objectOut);
    objectOut[SFP(AStr_Key_Id)] = astroTargetTypeToCatalogId(targetType);
    if (catalogId[0]) { objectOut[SFP(AStr_Key_CatalogId)] = catalogId; }
    if (targetName[0]) { objectOut[SFP(AStr_Key_TargetName)] = targetName; }
    AstroString targetClassStr = targetClassToString(targetClass, true);
    if (targetClassStr.length()) { objectOut[SFP(AStr_Key_TargetClass)] = targetClassStr; }
    if (rightAscensionSeconds) { objectOut[SFP(AStr_Key_RightAscensionSeconds)] = rightAscensionSeconds; }
    if (declinationArcseconds) { objectOut[SFP(AStr_Key_DeclinationArcseconds)] = declinationArcseconds; }
    if (movingTarget) { objectOut[SFP(AStr_Key_MovingTarget)] = movingTarget; }
}

void AstroTargetsLibData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroData::fromJSONObject(objectIn);

    targetType = astroTargetTypeFromCatalogId(AstroString((const char *)(objectIn[SFP(AStr_Key_Id)] | AStr_Blank)));
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
    targetClass = targetClassFromString(AstroString((const char *)(objectIn[SFP(AStr_Key_TargetClass)] | AStr_Blank)));
    rightAscensionSeconds = objectIn[SFP(AStr_Key_RightAscensionSeconds)] | rightAscensionSeconds;
    declinationArcseconds = objectIn[SFP(AStr_Key_DeclinationArcseconds)] | declinationArcseconds;
    movingTarget = objectIn[SFP(AStr_Key_MovingTarget)] | movingTarget;
    unsetModified();
}
