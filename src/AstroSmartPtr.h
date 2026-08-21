/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Shared Pointer Compatibility
*/

#ifndef AstroSmartPtr_H
#define AstroSmartPtr_H

#if defined(ARDUINO)
#include <ArxSmartPtr.h>

template<typename T>
using SharedPtr = arx::stdx::shared_ptr<T>;

template<class T, class U>
inline SharedPtr<T> astroStaticPointerCast(const SharedPtr<U> &ptr)
{
    return arx::stdx::static_pointer_cast<T>(ptr);
}
#else
#include <memory>

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<class T, class U>
inline SharedPtr<T> astroStaticPointerCast(const SharedPtr<U> &ptr)
{
    return std::static_pointer_cast<T>(ptr);
}
#endif

#endif // /ifndef AstroSmartPtr_H
