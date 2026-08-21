/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Controller Modules
*/

#include "Astruino.h"

AstroModule::AstroModule()
    : _initialized(false)
{ ; }

bool AstroModule::begin()
{
    _initialized = true;
    return true;
}

void AstroModule::update()
{ ; }

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
    ASTRO_SOFT_ASSERT(false, "Hashing collision"); // exhaustive search must be performed, publishing may miss values

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
