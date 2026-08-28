/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Controller Modules
*/

#include "Astruino.h"

AstroCalibrations::~AstroCalibrations()
{
    clearUserCalibrationData();
}

void AstroCalibrations::clearUserCalibrationData()
{
    for (auto iter = _calibrationData.begin(); iter != _calibrationData.end(); ++iter) {
        if (iter->second) { delete iter->second; }
    }
    _calibrationData.clear();
}

const AstroCalibrationData *AstroCalibrations::getUserCalibrationData(akey_t key) const
{
    auto iter = _calibrationData.find(key);
    return iter != _calibrationData.end() ? iter->second : nullptr;
}

bool AstroCalibrations::setUserCalibrationData(const AstroCalibrationData *calibrationData)
{
    ASTRO_SOFT_ASSERT(calibrationData && calibrationData->ownerName[0], SFP(AStr_InvalidParameter));
    if (!calibrationData || !calibrationData->ownerName[0]) { return false; }

    akey_t key = astroStringHash(calibrationData->ownerName);
    auto iter = _calibrationData.find(key);
    if (iter == _calibrationData.end()) {
        AstroCalibrationData *copy = new AstroCalibrationData(*calibrationData);
        ASTRO_SOFT_ASSERT(copy, SFP(AStr_AllocationFailure));
        if (!copy) { return false; }
        _calibrationData[key] = copy;
    } else {
        *(iter->second) = *calibrationData;
    }
    return true;
}

bool AstroCalibrations::dropUserCalibrationData(const AstroCalibrationData *calibrationData)
{
    ASTRO_HARD_ASSERT(calibrationData, SFP(AStr_InvalidParameter));
    if (!calibrationData) { return false; }
    akey_t key = astroStringHash(calibrationData->ownerName);
    auto iter = _calibrationData.find(key);
    if (iter == _calibrationData.end()) { return false; }
    if (iter->second) { delete iter->second; }
    _calibrationData.erase(iter);
    return true;
}

bool AstroObjectRegistration::registerObject(SharedPtr<AstroObject> object)
{
    if (!object || object->getKey() == akey_none || _objects.find(object->getKey()) != _objects.end()) { return false; }
    _objects[object->getKey()] = object;
    return true;
}

bool AstroObjectRegistration::unregisterObject(SharedPtr<AstroObject> object)
{
    if (!object) { return false; }
    auto iter = _objects.find(object->getKey());
    if (iter == _objects.end() || iter->second.get() != object.get()) { return false; }

    object->unresolve();
    _objects.erase(iter);
    return true;
}

SharedPtr<AstroObject> AstroObjectRegistration::objectById(AstroIdentity id) const
{
    if (id.posIndex == ASTRO_POS_SEARCH_FROMBEG) {
        while (++id.posIndex < ASTRO_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (iter != _objects.end()) {
                if (id.keyString == iter->second->getKeyString()) {
                    return iter->second;
                } else {
                    return objectById_Col(id);
                }
            }
        }
    } else if (id.posIndex == ASTRO_POS_SEARCH_FROMEND) {
        while (--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (iter != _objects.end()) {
                if (id.keyString == iter->second->getKeyString()) {
                    return iter->second;
                } else {
                    return objectById_Col(id);
                }
            }
        }
    } else {
        auto iter = _objects.find(id.key);
        if (iter != _objects.end()) {
            if (id.keyString == iter->second->getKeyString()) {
                return iter->second;
            } else {
                return objectById_Col(id);
            }
        }
    }

    return nullptr;
}

SharedPtr<AstroObject> AstroObjectRegistration::objectById_Col(const AstroIdentity &id) const
{
    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_HashingCollision)); // exhaustive search must be performed, publishing may miss values

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (id.keyString == iter->second->getKeyString()) {
            return iter->second;
        }
    }

    return nullptr;
}

aposi_t AstroObjectRegistration::firstPosition(AstroIdentity id, bool taken) const
{
    if (id.posIndex != ASTRO_POS_SEARCH_FROMEND) {
        id.posIndex = ASTRO_POS_SEARCH_FROMBEG;
        while (++id.posIndex < ASTRO_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    } else {
        id.posIndex = ASTRO_POS_SEARCH_FROMEND;
        while (--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    }

    return -1;
}

void AstroObjectRegistration::updateObjects()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second) { iter->second->update(); }
    }
}

AstroFixedLocationProvider::AstroFixedLocationProvider(const AstroObserver &observer)
    : _observer(observer)
{ ; }

bool AstroFixedLocationProvider::getObserver(AstroObserver *observerOut)
{
    if (!observerOut) { return false; }
    *observerOut = _observer;
    return true;
}

void AstroFixedLocationProvider::setObserver(const AstroObserver &observer)
{
    _observer = observer;
}

AstroCallbackLocationProvider::AstroCallbackLocationProvider(LocationCallback callback, void *context)
    : _callback(callback), _context(context)
{ ; }

bool AstroCallbackLocationProvider::getObserver(AstroObserver *observerOut)
{
    return _callback && observerOut ? _callback(_context, observerOut) : false;
}
