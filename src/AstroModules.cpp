/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Controller Modules
*/

#include "Astruino.h"

AstroCalibrations::~AstroCalibrations()
{
    clearUserCalibrations();
}

void AstroCalibrations::clearUserCalibrations()
{
    while (_calibrationData.size()) {
        auto iter = _calibrationData.begin();
        if (iter->second) { delete iter->second; }
        _calibrationData.erase(iter);
    }
}

const AstroCalibrationData *AstroCalibrations::getUserCalibrationData(akey_t key) const
{
    auto iter = _calibrationData.find(key);
    if (iter != _calibrationData.end()) {
        return iter->second;
    }
    return nullptr;
}

bool AstroCalibrations::setUserCalibrationData(const AstroCalibrationData *calibrationData)
{
    ASTRO_SOFT_ASSERT(calibrationData, SFP(AStr_Err_InvalidParameter));

    if (calibrationData) {
        akey_t key = stringHash(calibrationData->ownerName);
        auto iter = _calibrationData.find(key);
        bool retVal = false;

        if (iter == _calibrationData.end()) {
            auto calibData = new AstroCalibrationData();

            ASTRO_SOFT_ASSERT(calibData, SFP(AStr_Err_AllocationFailure));
            if (calibData) {
                *calibData = *calibrationData;
                _calibrationData[key] = calibData;
                retVal = (_calibrationData.find(key) != _calibrationData.end());
            }
        } else {
            *(iter->second) = *calibrationData;
            retVal = true;
        }

        return retVal;
    }
    return false;
}

bool AstroCalibrations::dropUserCalibrationData(const AstroCalibrationData *calibrationData)
{
    ASTRO_HARD_ASSERT(calibrationData, SFP(AStr_Err_InvalidParameter));
    if (!calibrationData) { return false; }

    akey_t key = stringHash(calibrationData->ownerName);
    auto iter = _calibrationData.find(key);

    if (iter != _calibrationData.end()) {
        if (iter->second) { delete iter->second; }
        _calibrationData.erase(iter);

        return true;
    }

    return false;
}


bool AstroObjectRegistration::registerObject(SharedPtr<AstroObject> obj)
{
    ASTRO_SOFT_ASSERT(obj && obj->getId().posIndex >= 0 && obj->getId().posIndex < ASTRO_POS_MAXSIZE, SFP(AStr_Err_InvalidParameter));
    if (obj && _objects.find(obj->getKey()) == _objects.end()) {
        _objects[obj->getKey()] = obj;

        if (obj->isActuatorType() || obj->isMountType() || obj->isTargetType()) {
            if (getScheduler()) {
                getScheduler()->setNeedsScheduling();
            }
        }
        if (obj->isSensorType()) {
            if (getPublisher()) {
                getPublisher()->setNeedsTabulation();
            }
        }

        return true;
    }
    return false;
}

bool AstroObjectRegistration::unregisterObject(SharedPtr<AstroObject> obj)
{
    ASTRO_SOFT_ASSERT(obj, SFP(AStr_Err_InvalidParameter));
    if (!obj) { return false; }

    auto iter = _objects.find(obj->getKey());
    if (iter != _objects.end()) {
        _objects.erase(iter);

        if (obj->isActuatorType() || obj->isMountType() || obj->isTargetType()) {
            if (getScheduler()) {
                getScheduler()->setNeedsScheduling();
            }
        }
        if (obj->isSensorType()) {
            if (getPublisher()) {
                getPublisher()->setNeedsTabulation();
            }
        }

        return true;
    }
    return false;
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
    ASTRO_SOFT_ASSERT(false, F("Hashing collision")); // exhaustive search must be performed, publishing may miss values

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (id.keyString == iter->second->getKeyString()) {
            return iter->second;
        }
    }

    return nullptr;
}

aposi_t AstroObjectRegistration::firstPosition(AstroIdentity id, bool taken)
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


bool AstroPinHandlers::tryGetPinLock(pintype_t pin, millis_t wait)
{
    millis_t start = millis();
    while (1) {
        auto iter = _pinLocks.find(pin);
        if (iter == _pinLocks.end()) {
            _pinLocks[pin] = true;
            return (_pinLocks.find(pin) != _pinLocks.end());
        }
        else if (millis() - start >= wait) { return false; }
        else { yield(); }
    }
}

void AstroPinHandlers::deactivatePinMuxers()
{
    for (auto iter = _pinMuxers.begin(); iter != _pinMuxers.end(); ++iter) {
        iter->second->deactivate();
    }
}

OneWire *AstroPinHandlers::getOneWireForPin(pintype_t pin)
{
    auto wireIter = _pinOneWire.find(pin);
    if (wireIter != _pinOneWire.end()) {
        return wireIter->second;
    } else {
        OneWire *oneWire = new OneWire(pin);
        if (oneWire) {
            _pinOneWire[pin] = oneWire;
            if (_pinOneWire.find(pin) != _pinOneWire.end()) { return oneWire; }
            else if (oneWire) { delete oneWire; }
        } else if (oneWire) { delete oneWire; }
    }
    return nullptr;
}

void AstroPinHandlers::dropOneWireForPin(pintype_t pin)
{
    auto wireIter = _pinOneWire.find(pin);
    if (wireIter != _pinOneWire.end()) {
        if (wireIter->second) {
            wireIter->second->depower();
            delete wireIter->second;
        }
        _pinOneWire.erase(wireIter);
    }
}
