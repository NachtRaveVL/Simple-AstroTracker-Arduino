/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Attachment Inlines
*/

#ifndef AstroAttachments_HPP
#define AstroAttachments_HPP

inline AstroDLinkObject &AstroDLinkObject::operator=(AstroIdentity rhs)
{
    _key = rhs.key;
    _obj = nullptr;
    _keyString = rhs.keyString;
    return *this;
}

inline AstroDLinkObject &AstroDLinkObject::operator=(const char *rhs)
{
    _key = rhs ? astroStringHash(rhs) : akey_none;
    _obj = nullptr;
    _keyString = rhs ? AstroString(rhs) : AstroString();
    return *this;
}

inline AstroDLinkObject &AstroDLinkObject::operator=(const AstroObjInterface *rhs)
{
    _key = rhs ? rhs->getKey() : akey_none;
    _obj = rhs ? rhs->getSharedPtr() : nullptr;
    _keyString = rhs && !_obj ? rhs->getKeyString() : AstroString();
    return *this;
}

inline AstroDLinkObject &AstroDLinkObject::operator=(const AstroAttachment *rhs)
{
    _key = rhs ? rhs->getKey() : akey_none;
    _obj = rhs && rhs->isResolved() ? const_cast<AstroAttachment *>(rhs)->getObject<AstroObjInterface>() : nullptr;
    _keyString = rhs && !rhs->isResolved() ? rhs->getKeyString() : AstroString();
    return *this;
}

template<class U>
inline AstroDLinkObject &AstroDLinkObject::operator=(SharedPtr<U> rhs)
{
    _key = rhs ? rhs->getKey() : akey_none;
    _obj = rhs ? static_pointer_cast<AstroObjInterface>(rhs) : nullptr;
    _keyString = AstroString();
    return *this;
}

template<class U>
void AstroAttachment::setObject(U object, bool modify)
{
    if (!(_obj == object)) {
        if (_obj.isResolved()) { detachObject(); }
        _obj = object;
        if (_obj.isResolved()) { attachObject(); }
        if (modify) { bumpRevisionIfNeeded(); }
    }
}

template<class U>
SharedPtr<U> AstroAttachment::getObject()
{
    if (_obj) { return _obj.getObject<U>(); }
    if (!_obj.isSet()) { return nullptr; }
    if (_obj.needsResolved() && _obj.resolveObject()) { attachObject(); }
    return _obj.getObject<U>();
}

#endif // /ifndef AstroAttachments_HPP
