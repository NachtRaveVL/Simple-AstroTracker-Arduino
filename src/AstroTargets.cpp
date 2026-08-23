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
        case Astro_TargetType_Sun: return AstroString("Sun");
        case Astro_TargetType_Moon: return AstroString("Moon");
        case Astro_TargetType_Mercury: return AstroString("Mercury");
        case Astro_TargetType_Venus: return AstroString("Venus");
        case Astro_TargetType_Mars: return AstroString("Mars");
        case Astro_TargetType_Jupiter: return AstroString("Jupiter");
        case Astro_TargetType_Saturn: return AstroString("Saturn");
        case Astro_TargetType_Uranus: return AstroString("Uranus");
        case Astro_TargetType_Neptune: return AstroString("Neptune");
        case Astro_TargetType_Sirius: return AstroString("Sirius");
        case Astro_TargetType_Canopus: return AstroString("Canopus");
        case Astro_TargetType_Arcturus: return AstroString("Arcturus");
        case Astro_TargetType_Vega: return AstroString("Vega");
        case Astro_TargetType_Capella: return AstroString("Capella");
        case Astro_TargetType_RigelKentaurus: return AstroString("RigelKentaurus");
        case Astro_TargetType_Procyon: return AstroString("Procyon");
        case Astro_TargetType_Betelgeuse: return AstroString("Betelgeuse");
        case Astro_TargetType_Achernar: return AstroString("Achernar");
        case Astro_TargetType_Hadar: return AstroString("Hadar");
        case Astro_TargetType_Altair: return AstroString("Altair");
        case Astro_TargetType_Acrux: return AstroString("Acrux");
        case Astro_TargetType_Aldebaran: return AstroString("Aldebaran");
        case Astro_TargetType_Spica: return AstroString("Spica");
        case Astro_TargetType_Antares: return AstroString("Antares");
        case Astro_TargetType_Pollux: return AstroString("Pollux");
        case Astro_TargetType_Fomalhaut: return AstroString("Fomalhaut");
        case Astro_TargetType_Deneb: return AstroString("Deneb");
        case Astro_TargetType_Regulus: return AstroString("Regulus");
        case Astro_TargetType_Polaris: return AstroString("Polaris");
        case Astro_TargetType_Castor: return AstroString("Castor");
        case Astro_TargetType_Bellatrix: return AstroString("Bellatrix");
        case Astro_TargetType_Alnilam: return AstroString("Alnilam");
        case Astro_TargetType_Alnitak: return AstroString("Alnitak");
        case Astro_TargetType_Mizar: return AstroString("Mizar");
        case Astro_TargetType_Dubhe: return AstroString("Dubhe");
        default: break;
    }

    if (targetType >= Astro_TargetType_M1 && targetType <= Astro_TargetType_M110) {
        return AstroString("M") + astroPositionIndexToString((aposi_t)((int)targetType - (int)Astro_TargetType_M1 + 1));
    }
    if (targetType >= Astro_TargetType_CustomTarget1 && targetType <= Astro_TargetType_CustomTarget8) {
        return AstroString("CustomTarget") + astroPositionIndexToString((aposi_t)((int)targetType - (int)Astro_TargetType_CustomTarget1 + 1));
    }
    return AstroString();
}

static Astro_TargetType astroTargetTypeFromCatalogId(const AstroString &catalogId)
{
    if (catalogId == AstroString("Sun")) return Astro_TargetType_Sun;
    if (catalogId == AstroString("Moon")) return Astro_TargetType_Moon;
    if (catalogId == AstroString("Mercury")) return Astro_TargetType_Mercury;
    if (catalogId == AstroString("Venus")) return Astro_TargetType_Venus;
    if (catalogId == AstroString("Mars")) return Astro_TargetType_Mars;
    if (catalogId == AstroString("Jupiter")) return Astro_TargetType_Jupiter;
    if (catalogId == AstroString("Saturn")) return Astro_TargetType_Saturn;
    if (catalogId == AstroString("Uranus")) return Astro_TargetType_Uranus;
    if (catalogId == AstroString("Neptune")) return Astro_TargetType_Neptune;
    if (catalogId == AstroString("Sirius")) return Astro_TargetType_Sirius;
    if (catalogId == AstroString("Canopus")) return Astro_TargetType_Canopus;
    if (catalogId == AstroString("Arcturus")) return Astro_TargetType_Arcturus;
    if (catalogId == AstroString("Vega")) return Astro_TargetType_Vega;
    if (catalogId == AstroString("Capella")) return Astro_TargetType_Capella;
    if (catalogId == AstroString("RigelKentaurus")) return Astro_TargetType_RigelKentaurus;
    if (catalogId == AstroString("Procyon")) return Astro_TargetType_Procyon;
    if (catalogId == AstroString("Betelgeuse")) return Astro_TargetType_Betelgeuse;
    if (catalogId == AstroString("Achernar")) return Astro_TargetType_Achernar;
    if (catalogId == AstroString("Hadar")) return Astro_TargetType_Hadar;
    if (catalogId == AstroString("Altair")) return Astro_TargetType_Altair;
    if (catalogId == AstroString("Acrux")) return Astro_TargetType_Acrux;
    if (catalogId == AstroString("Aldebaran")) return Astro_TargetType_Aldebaran;
    if (catalogId == AstroString("Spica")) return Astro_TargetType_Spica;
    if (catalogId == AstroString("Antares")) return Astro_TargetType_Antares;
    if (catalogId == AstroString("Pollux")) return Astro_TargetType_Pollux;
    if (catalogId == AstroString("Fomalhaut")) return Astro_TargetType_Fomalhaut;
    if (catalogId == AstroString("Deneb")) return Astro_TargetType_Deneb;
    if (catalogId == AstroString("Regulus")) return Astro_TargetType_Regulus;
    if (catalogId == AstroString("Polaris")) return Astro_TargetType_Polaris;
    if (catalogId == AstroString("Castor")) return Astro_TargetType_Castor;
    if (catalogId == AstroString("Bellatrix")) return Astro_TargetType_Bellatrix;
    if (catalogId == AstroString("Alnilam")) return Astro_TargetType_Alnilam;
    if (catalogId == AstroString("Alnitak")) return Astro_TargetType_Alnitak;
    if (catalogId == AstroString("Mizar")) return Astro_TargetType_Mizar;
    if (catalogId == AstroString("Dubhe")) return Astro_TargetType_Dubhe;

    const char *text = catalogId.c_str();
    if (text && text[0] == 'M' && text[1]) {
        long number = strtol(text + 1, nullptr, 10);
        if (number >= 1 && number <= 110) {
            return (Astro_TargetType)((int)Astro_TargetType_M1 + number - 1);
        }
    }
    const char customPrefix[] = "CustomTarget";
    if (text && !strncmp(text, customPrefix, sizeof(customPrefix) - 1)) {
        long number = strtol(text + sizeof(customPrefix) - 1, nullptr, 10);
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
    if (catalogId[0]) { objectOut["catalogId"] = catalogId; }
    if (targetName[0]) { objectOut["targetName"] = targetName; }
    AstroString targetClassStr = targetClassToString(targetClass, true);
    if (targetClassStr.length()) { objectOut["targetClass"] = targetClassStr; }
    if (rightAscensionSeconds) { objectOut["rightAscensionSeconds"] = rightAscensionSeconds; }
    if (declinationArcseconds) { objectOut["declinationArcseconds"] = declinationArcseconds; }
    if (movingTarget) { objectOut["movingTarget"] = movingTarget; }
}

void AstroTargetsLibData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroData::fromJSONObject(objectIn);

    targetType = astroTargetTypeFromCatalogId(AstroString((const char *)(objectIn[SFP(AStr_Key_Id)] | "")));
    const char *catalogIdIn = objectIn["catalogId"] | nullptr;
    if (catalogIdIn) {
        strncpy(catalogId, catalogIdIn, sizeof(catalogId) - 1);
        catalogId[sizeof(catalogId) - 1] = '\0';
    }
    const char *targetNameIn = objectIn["targetName"] | nullptr;
    if (targetNameIn) {
        strncpy(targetName, targetNameIn, sizeof(targetName) - 1);
        targetName[sizeof(targetName) - 1] = '\0';
    }
    targetClass = targetClassFromString(AstroString((const char *)(objectIn["targetClass"] | "")));
    rightAscensionSeconds = objectIn["rightAscensionSeconds"] | rightAscensionSeconds;
    declinationArcseconds = objectIn["declinationArcseconds"] | declinationArcseconds;
    movingTarget = objectIn["movingTarget"] | movingTarget;
    unsetModified();
}
