/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Object
*/

#include "AstroObject.h"
#include "AstroUtils.h"
#include "AstroStrings.h"
#include <stdio.h>
#include <string.h>

AstroIdentity::AstroIdentity(akey_t keyIn)
    : type(Unknown), objTypeAs(), posIndex(ASTRO_POS_SEARCH_FROMBEG), keyString(), key(keyIn)
{ objTypeAs.idType = Unknown; }

AstroIdentity::AstroIdentity(const char *keyStringIn)
    : type(Unknown), objTypeAs(), posIndex(ASTRO_POS_SEARCH_FROMBEG),
      keyString(keyStringIn ? keyStringIn : ""), key(keyStringIn ? astroStringHash(keyStringIn) : akey_none)
{ objTypeAs.idType = Unknown; }

AstroIdentity::AstroIdentity(Astro_ActuatorType actuatorTypeIn, aposi_t positionIndex)
    : type(Actuator), objTypeAs(), posIndex(positionIndex), keyString(), key(akey_none)
{ objTypeAs.actuatorType = actuatorTypeIn; regenKey(); }

AstroIdentity::AstroIdentity(Astro_SensorType sensorTypeIn, aposi_t positionIndex)
    : type(Sensor), objTypeAs(), posIndex(positionIndex), keyString(), key(akey_none)
{ objTypeAs.sensorType = sensorTypeIn; regenKey(); }

AstroIdentity::AstroIdentity(Astro_MountType mountTypeIn, aposi_t positionIndex)
    : type(Mount), objTypeAs(), posIndex(positionIndex), keyString(), key(akey_none)
{ objTypeAs.mountType = mountTypeIn; regenKey(); }

AstroIdentity::AstroIdentity(Astro_RailType railTypeIn, aposi_t positionIndex)
    : type(Rail), objTypeAs(), posIndex(positionIndex), keyString(), key(akey_none)
{ objTypeAs.railType = railTypeIn; regenKey(); }

AstroIdentity::AstroIdentity(int objectType, int16_t subType, aposi_t positionIndex)
    : type((decltype(type))objectType), objTypeAs(), posIndex(positionIndex), keyString(), key(akey_none)
{ objTypeAs.idType = subType; regenKey(); }

akey_t AstroIdentity::regenKey()
{
    switch (type) {
        case Actuator: keyString = actuatorTypeToString(objTypeAs.actuatorType, true); break;
        case Sensor: keyString = sensorTypeToString(objTypeAs.sensorType, true); break;
        case Mount: keyString = mountTypeToString(objTypeAs.mountType, true); break;
        case Rail: keyString = railTypeToString(objTypeAs.railType, true); break;
        case Cover: keyString = SFP(AStr_Cover); break;
        case ObservationDevice: keyString = "ObservationDevice"; break;
        default: return key;
    }

    if (posIndex >= 0) {
        keyString += " #";
        keyString += astroPositionIndexToString(posIndex);
    }
    key = astroStringHash(keyString.c_str());
    return key;
}

AstroString AstroIdentity::getDisplayString() const
{
    AstroString display;
    switch (type) {
        case Actuator: display = "Actuator "; break;
        case Sensor: display = "Sensor "; break;
        case Mount: display = "Mount "; break;
        case Rail: display = "Rail "; break;
        case Cover: display = "Cover "; break;
        case ObservationDevice: display = "Observation "; break;
        default: display = "Unknown "; break;
    }
    display += keyString;
    return display;
}

AstroObjectData::AstroObjectData()
    : idType(AstroIdentity::Unknown), objType(AstroIdentity::Unknown),
      posIndex(ASTRO_POS_SEARCH_FROMBEG), revision(0), name{0}
{ ; }

bool AstroObjectData::toJSON(char *bufferOut, size_t bufferSize) const
{
    if (!bufferOut || !bufferSize) { return false; }
    int written = snprintf(bufferOut, bufferSize,
        "{\"type\":\"AOBJ\",\"idType\":%d,\"objType\":%d,\"posIndex\":%d,\"revision\":%u,\"name\":\"%s\"}",
        (int)idType, (int)objType, (int)posIndex, (unsigned int)revision, name);
    return written >= 0 && (size_t)written < bufferSize;
}

bool AstroObjectData::fromJSON(const char *jsonIn)
{
    if (!jsonIn) { return false; }

    long idTypeIn, objTypeIn, posIndexIn;
    unsigned long revisionIn;
    char typeIn[8] = {0};
    char nameIn[ASTRO_NAME_MAXSIZE] = {0};
    if (!astroJSONGetString(jsonIn, "type", typeIn, sizeof(typeIn)) || strcmp(typeIn, "AOBJ") != 0 ||
        !astroJSONGetLong(jsonIn, "idType", &idTypeIn) ||
        !astroJSONGetLong(jsonIn, "objType", &objTypeIn) ||
        !astroJSONGetLong(jsonIn, "posIndex", &posIndexIn) ||
        !astroJSONGetUnsignedLong(jsonIn, "revision", &revisionIn) ||
        !astroJSONGetString(jsonIn, "name", nameIn, sizeof(nameIn))) {
        return false;
    }

    idType = (int8_t)idTypeIn;
    objType = (int16_t)objTypeIn;
    posIndex = (aposi_t)posIndexIn;
    revision = (uint8_t)revisionIn;
    snprintf(name, sizeof(name), "%s", nameIn);
    return true;
}

AstroObject::AstroObject(AstroIdentity id)
    : _id(id), _revision(-1), _linksSize(0), _links(nullptr)
{ ; }

AstroObject::AstroObject(const AstroObjectData *dataIn)
    : _id(dataIn ? AstroIdentity(dataIn->idType, dataIn->objType, dataIn->posIndex) : AstroIdentity()),
      _revision(dataIn ? (int8_t)dataIn->revision : -1), _linksSize(0), _links(nullptr)
{
    if (dataIn && dataIn->name[0]) {
        _id.keyString = dataIn->name;
        _id.key = astroStringHash(_id.keyString.c_str());
    }
}

AstroObject::~AstroObject()
{
    if (_links) { delete [] _links; _links = nullptr; }
}

void AstroObject::update()
{ ; }

void AstroObject::handleLowMemory()
{
    if (_links && _linksSize > 1 && !_links[_linksSize >> 1].object) { allocateLinkages(_linksSize >> 1); }
}

AstroObjectData *AstroObject::newSaveData()
{
    AstroObjectData *dataOut = allocateData();
    if (dataOut) { saveToData(dataOut); }
    return dataOut;
}

void AstroObject::allocateLinkages(size_t size)
{
    if (_linksSize == size) { return; }
    AstroObjectLink *newLinks = size ? new AstroObjectLink[size] : nullptr;
    if (size && !newLinks) { return; }

    size_t copyCount = _linksSize < size ? _linksSize : size;
    for (size_t i = 0; i < copyCount; ++i) { newLinks[i] = _links[i]; }
    if (_links) { delete [] _links; }
    _links = newLinks;
    _linksSize = size;
}

bool AstroObject::addLinkage(AstroObject *object)
{
    if (!object) { return false; }
    if (!_links) { allocateLinkages(); }
    if (!_links) { return false; }

    size_t index = 0;
    for (; index < _linksSize && _links[index].object; ++index) {
        if (_links[index].object == object) {
            ++_links[index].count;
            return true;
        }
    }
    if (index >= _linksSize) {
        size_t oldSize = _linksSize;
        allocateLinkages(_linksSize ? _linksSize << 1 : 1);
        index = oldSize;
    }
    if (index < _linksSize) {
        _links[index] = AstroObjectLink(object, 1);
        return true;
    }
    return false;
}

bool AstroObject::removeLinkage(AstroObject *object)
{
    if (!_links || !object) { return false; }
    for (size_t i = 0; i < _linksSize && _links[i].object; ++i) {
        if (_links[i].object == object) {
            if (--_links[i].count <= 0) {
                for (size_t j = i; j + 1 < _linksSize; ++j) { _links[j] = _links[j + 1]; }
                _links[_linksSize - 1] = AstroObjectLink();
            }
            return true;
        }
    }
    return false;
}

bool AstroObject::hasLinkage(AstroObject *object) const
{
    if (!_links || !object) { return false; }
    for (size_t i = 0; i < _linksSize && _links[i].object; ++i) {
        if (_links[i].object == object) { return true; }
    }
    return false;
}

void AstroObject::unresolveAny(AstroObject *object)
{
    if (!_links || !object) { return; }
    while (removeLinkage(object)) { ; }
}

akey_t AstroObject::getKey() const
{
    return _id.key;
}

AstroString AstroObject::getKeyString() const
{
    return _id.keyString;
}

bool AstroObject::isObject() const
{
    return true;
}

AstroObjectData *AstroObject::allocateData() const
{
    return new AstroObjectData();
}

void AstroObject::saveToData(AstroObjectData *dataOut) const
{
    if (!dataOut) { return; }
    dataOut->idType = (int8_t)_id.type;
    dataOut->objType = _id.objTypeAs.idType;
    dataOut->posIndex = _id.posIndex;
    dataOut->revision = getRevision();
    snprintf(dataOut->name, sizeof(dataOut->name), "%s", _id.keyString.c_str());
}

akey_t AstroSubObject::getKey() const
{
    uintptr_t address = (uintptr_t)this;
    return (akey_t)((address ^ (address >> 16)) & 0xffff);
}

AstroString AstroSubObject::getKeyString() const
{
    char buffer[2 * sizeof(void *) + 3];
    snprintf(buffer, sizeof(buffer), "%p", (const void *)this);
    return AstroString(buffer);
}
