/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Attachment Inlines
*/

#ifndef AstroAttachments_HPP
#define AstroAttachments_HPP

#include "Astruino.h"

inline AstroDLinkObject &AstroDLinkObject::operator=(AstroIdentity rhs)
{
    _key = rhs.key;
    _obj = nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    auto len = rhs.keyString.length();
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs.keyString.c_str(), len + 1);
    }
    return *this;
}

inline AstroDLinkObject &AstroDLinkObject::operator=(const char *rhs)
{
    _key = stringHash(rhs);
    _obj = nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    auto len = strnlen(rhs, ASTRO_NAME_MAXSIZE);
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs, len + 1);
    }
    return *this;
}

inline AstroDLinkObject &AstroDLinkObject::operator=(const AstroObjInterface *rhs)
{
    _key = rhs ? rhs->getKey() : akey_none;
    _obj = rhs ? rhs->getSharedPtr() : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    return *this;
}

inline AstroDLinkObject &AstroDLinkObject::operator=(const AstroAttachment *rhs)
{
    _key = rhs ? rhs->getKey() : akey_none;
    _obj = rhs && rhs->isResolved() ? rhs->getSharedPtr() : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    if (rhs && !rhs->isResolved()) {
        String keyString = rhs->getKeyString();
        auto len = keyString.length();
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, keyString.c_str(), len + 1);
        }
    }
    return *this;
}

template<class U>
inline AstroDLinkObject &AstroDLinkObject::operator=(SharedPtr<U> &rhs)
{
    _key = rhs ? rhs->getKey() : akey_none;
    _obj = rhs ? static_pointer_cast<AstroObjInterface>(rhs) : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    return *this;
}


template<class U>
void AstroAttachment::setObject(U obj, bool modify)
{
    if (!(_obj == obj)) {
        if (_obj.isResolved()) { detachObject(); }

        _obj = obj; // will be replaced by templated operator= inline

        if (_obj.isResolved()) { attachObject(); }

        if (modify && _parent) {
            if (_parent->isObject()) {
                ((AstroObject *)_parent)->bumpRevisionIfNeeded();
            } else {
                ((AstroSubObject *)_parent)->bumpRevisionIfNeeded();
            }
        }
    }
}

template<class U>
SharedPtr<U> AstroAttachment::getObject()
{
    if (_obj) { return _obj.getObject<U>(); }
    else if (!_obj.isSet()) { return nullptr; }
    else if (_obj.needsResolved() && _obj.resolveObject()) {
        attachObject();
    }
    return _obj.getObject<U>();
}


template<class ParameterType, int Slots> template<class U>
AstroSignalAttachment<ParameterType,Slots>::AstroSignalAttachment(AstroObjInterface *parent, aposi_t subIndex, Signal<ParameterType,Slots> &(U::*signalGetter)(void))
    : AstroAttachment(parent, subIndex), _signalGetter((SignalGetterPtr)signalGetter), _handleSlot(nullptr)
{ ; }

template<class ParameterType, int Slots>
AstroSignalAttachment<ParameterType,Slots>::AstroSignalAttachment(const AstroSignalAttachment<ParameterType,Slots> &attachment)
    : AstroAttachment(attachment), _signalGetter((SignalGetterPtr)attachment._signalGetter),
      _handleSlot(attachment._handleSlot ? attachment._handleSlot->clone() : nullptr)
{ ; }

template<class ParameterType, int Slots>
AstroSignalAttachment<ParameterType,Slots> &AstroSignalAttachment<ParameterType,Slots>::operator=(const AstroSignalAttachment<ParameterType,Slots> &attachment)
{
    if (this != &attachment) {
        if (isResolved() && _handleSlot && _signalGetter) {
            (get()->*_signalGetter)().detach(*_handleSlot);
        }

        AstroAttachment::operator=(attachment);
        _signalGetter = attachment._signalGetter;
        if (_handleSlot) { delete _handleSlot; _handleSlot = nullptr; }
        _handleSlot = attachment._handleSlot ? attachment._handleSlot->clone() : nullptr;

        if (isResolved() && _handleSlot && _signalGetter) {
            (get()->*_signalGetter)().attach(*_handleSlot);
        }
    }
    return *this;
}

template<class ParameterType, int Slots>
AstroSignalAttachment<ParameterType,Slots>::~AstroSignalAttachment()
{
    if (isResolved() && _handleSlot && _signalGetter) {
        (get()->*_signalGetter)().detach(*_handleSlot);
    }
    if (_handleSlot) {
        delete _handleSlot; _handleSlot = nullptr;
    }
}

template<class ParameterType, int Slots>
void AstroSignalAttachment<ParameterType,Slots>::attachObject()
{
    AstroAttachment::attachObject();

    if (isResolved() && _handleSlot && _signalGetter) {
        (get()->*_signalGetter)().attach(*_handleSlot);
    }
}

template<class ParameterType, int Slots>
void AstroSignalAttachment<ParameterType,Slots>::detachObject()
{
    if (isResolved() && _handleSlot && _signalGetter) {
        (get()->*_signalGetter)().detach(*_handleSlot);
    }

    AstroAttachment::detachObject();
}

template<class ParameterType, int Slots> template<class U>
void AstroSignalAttachment<ParameterType,Slots>::setSignalGetter(Signal<ParameterType,Slots> &(U::*signalGetter)(void))
{
    if (_signalGetter != signalGetter) {
        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().detach(*_handleSlot); }

        _signalGetter = signalGetter;

        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().attach(*_handleSlot); }
    }
}

template<class ParameterType, int Slots>
void AstroSignalAttachment<ParameterType,Slots>::setHandleSlot(const Slot<ParameterType> &handleSlot)
{
    if (!_handleSlot || !_handleSlot->operator==(&handleSlot)) {
        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().detach(*_handleSlot); }

        if (_handleSlot) { delete _handleSlot; _handleSlot = nullptr; }
        _handleSlot = handleSlot.clone();

        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().attach(*_handleSlot); }
    }
}


inline float AstroActuatorAttachment::getActiveDriveIntensity()
{
    return resolve() ? get()->getDriveIntensity() : 0.0f;
}

inline float AstroActuatorAttachment::getSetupDriveIntensity() const
{
    return _actSetup.getDriveIntensity();
}


inline Astro_TriggerState AstroTriggerAttachment::getTriggerState(bool poll)
{
    return resolve() ? get()->getTriggerState(poll) : Astro_TriggerState_Undefined;
}

#endif // /ifndef AstroAttachments_HPP
