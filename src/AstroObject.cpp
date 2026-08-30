/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Object
*/

#include "Astruino.h"

AstroObject *newObjectFromData(const AstroData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(AStr_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.idType) {
            case (aid_t)AstroIdentity::Actuator:
                return newActuatorObjectFromData((AstroActuatorData *)dataIn);
            case (aid_t)AstroIdentity::Sensor:
                return newSensorObjectFromData((AstroSensorData *)dataIn);
            case (aid_t)AstroIdentity::Target:
                return newTargetObjectFromData((AstroTargetData *)dataIn);
            case (aid_t)AstroIdentity::Mount:
                return newMountObjectFromData((AstroMountData *)dataIn);
            case (aid_t)AstroIdentity::Rail:
                return newRailObjectFromData((AstroRailData *)dataIn);
            case (aid_t)AstroIdentity::ObservationDevice:
                return newObservationDeviceObjectFromData((AstroObservationDeviceData *)dataIn);
            default: // Unable
                return nullptr;
        }
    }

    return nullptr;
}


akey_t AstroIdentity::regenKey()
{
    switch (type) {
        case Actuator:
            keyString = actuatorTypeToString(objTypeAs.actuatorType, true);
            break;
        case Sensor:
            keyString = sensorTypeToString(objTypeAs.sensorType, true);
            break;
        case Target:
            keyString = targetTypeToString(objTypeAs.targetType, true);
            break;
        case Mount:
            keyString = mountTypeToString(objTypeAs.mountType, true);
            break;
        case Rail:
            keyString = railTypeToString(objTypeAs.railType, true);
            break;
        case ObservationDevice:
            keyString = SFP(AStr_ObservationDevice);
            break;
        default: // Unable
            return key;
    }
    keyString.concat(' ');
    keyString.concat('#');
    keyString.concat(positionIndexToString(posIndex, true));
    key = stringHash(keyString);
    return key;
}

String AstroIdentity::getDisplayString()
{
    switch (type) {
        case Actuator: return SFP(AStr_Actuator) + ' ' + keyString;
        case Sensor: return SFP(AStr_Sensor) + ' ' + keyString;
        case Target: return SFP(AStr_Target) + ' ' + keyString;
        case Mount: return SFP(AStr_Mount) + ' ' + keyString;
        case Rail: return SFP(AStr_Rail) + ' ' + keyString;
        case ObservationDevice: return SFP(AStr_ObservationDeviceDisplay) + ' ' + keyString;
        default: return SFP(AStr_Unknown) + ' ' + keyString;
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
    if (_links && !_links[_linksSize >> 1].first) { allocateLinkages(_linksSize >> 1); } // shrink /2 if too big
}

AstroData *AstroObject::newSaveData()
{
    auto data = allocateData();
    ASTRO_SOFT_ASSERT(data, SFP(AStr_AllocationFailure));
    if (data) { saveToData(data); }
    return data;
}

void AstroObject::allocateLinkages(size_t size)
{
    if (_linksSize != size) {
        Pair<AstroObject *, int8_t> *newLinks = size ? new Pair<AstroObject *, int8_t>[size] : nullptr;

        if (size) {
            ASTRO_HARD_ASSERT(newLinks, SFP(AStr_AllocationFailure));

            aposi_t linksIndex = 0;
            if (_links) {
                for (; linksIndex < _linksSize && linksIndex < size; ++linksIndex) {
                    newLinks[linksIndex] = _links[linksIndex];
                }
            }
            for (; linksIndex < size; ++linksIndex) {
                newLinks[linksIndex] = make_pair((AstroObject *)nullptr, (int8_t)0);
            }
        }

        if (_links) { delete [] _links; }
        _links = newLinks;
        _linksSize = size;
    }
}

bool AstroObject::addLinkage(AstroObject *obj)
{
    if (!_links) { allocateLinkages(); }
    if (_links) {
        aposi_t linksIndex = 0;
        for (; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                _links[linksIndex].second++;
                return true;
            }
        }
        if (linksIndex >= _linksSize) { allocateLinkages(_linksSize << 1); } // grow *2 if too small
        if (linksIndex < _linksSize) {
            _links[linksIndex] = make_pair(obj, (int8_t)1);
            return true;
        }
    }
    return false;
}

bool AstroObject::removeLinkage(AstroObject *obj)
{
    if (_links) {
        for (aposi_t linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                if (--_links[linksIndex].second <= 0) {
                    for (int linksSubIndex = linksIndex; linksSubIndex < _linksSize - 1; ++linksSubIndex) {
                        _links[linksSubIndex] = _links[linksSubIndex + 1];
                    }
                    _links[_linksSize - 1] = make_pair((AstroObject *)nullptr, (int8_t)0);
                }
                return true;
            }
        }
    }
    return false;
}

bool AstroObject::hasLinkage(AstroObject *obj) const
{
    if (_links) {
        for (aposi_t linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                return true;
            }
        }
    }
    return false;
}

void AstroObject::unresolveAny(AstroObject *obj)
{
    if (this == obj && _links) {
        AstroObject *lastObject = nullptr;
        for (aposi_t linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            AstroObject *object = _links[linksIndex].first;
            if (object != obj) {
                object->unresolveAny(obj); // may clobber indexing

                if (linksIndex && _links[linksIndex].first != object) {
                    while (linksIndex && _links[linksIndex].first != lastObject) { --linksIndex; }
                    object = lastObject;
                }
            }
            lastObject = object;
        }
    }
}

AstroIdentity AstroObject::getId() const
{
    return _id;
}

akey_t AstroObject::getKey() const
{
    return _id.key;
}

String AstroObject::getKeyString() const
{
    return _id.keyString;
}

SharedPtr<AstroObjInterface> AstroObject::getSharedPtr() const
{
    return getController() ? static_pointer_cast<AstroObjInterface>(getController()->objectById(_id)) : nullptr;
}

SharedPtr<AstroObjInterface> AstroObject::getSharedPtrFor(const AstroObjInterface *obj) const
{
    return obj->isObject() ? obj->getSharedPtr() : nullptr;
}

bool AstroObject::isObject() const
{
    return true;
}

AstroData *AstroObject::allocateData() const
{
    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_UnsupportedOperation));
    return new AstroData();
}

void AstroObject::saveToData(AstroData *dataOut)
{
    dataOut->id.object.idType = (aid_t)_id.type;
    dataOut->id.object.objType = _id.objTypeAs.idType;
    dataOut->id.object.posIndex = _id.posIndex;
    dataOut->_revision = getRevision();
    if (_id.keyString.length()) {
        strncpy(((AstroObjectData *)dataOut)->name, _id.keyString.c_str(), ASTRO_NAME_MAXSIZE);
    }
}


void AstroSubObject::unresolveAny(AstroObject *obj)
{ ; }

AstroIdentity AstroSubObject::getId() const
{
    return AstroIdentity(getKey());
}

akey_t AstroSubObject::getKey() const
{
    return (akey_t)(intptr_t)this;
}

String AstroSubObject::getKeyString() const
{
    return addressToString((uintptr_t)this);
}

SharedPtr<AstroObjInterface> AstroSubObject::getSharedPtr() const
{
    return _parent ? _parent->getSharedPtrFor((const AstroObjInterface *)this) : nullptr;
}

SharedPtr<AstroObjInterface> AstroSubObject::getSharedPtrFor(const AstroObjInterface *obj) const
{
    return obj->isObject() ? obj->getSharedPtr() : _parent ? _parent->getSharedPtrFor(obj) : nullptr;
}

bool AstroSubObject::isObject() const
{
    return false;
}

void AstroSubObject::setParent(AstroObjInterface *parent)
{
    _parent = parent;
}


AstroObjectData::AstroObjectData()
    : AstroData(), name{0}
{
    _size = sizeof(*this);
}

void AstroObjectData::toJSONObject(JsonObject &objectOut) const
{
    AstroData::toJSONObject(objectOut);

    if (name[0]) { objectOut[SFP(AStr_Key_Id)] = charsToString(name, ASTRO_NAME_MAXSIZE); }
}

void AstroObjectData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroData::fromJSONObject(objectIn);

    const char *nameStr = objectIn[SFP(AStr_Key_Id)];
    if (nameStr && nameStr[0]) { strncpy(name, nameStr, ASTRO_NAME_MAXSIZE); }
}
