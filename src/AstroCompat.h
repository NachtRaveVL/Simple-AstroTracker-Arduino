/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Compatibility
*/

#ifndef AstroCompat_H
#define AstroCompat_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef ARDUINO
#include <Arduino.h>
typedef String AstroString;
#else
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <string>

typedef std::string AstroString;

#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW 0x0
#endif
#ifndef INPUT
#define INPUT 0x0
#endif
#ifndef OUTPUT
#define OUTPUT 0x1
#endif
#ifndef INPUT_PULLUP
#define INPUT_PULLUP 0x2
#endif
#ifndef INPUT_PULLDOWN
#define INPUT_PULLDOWN 0x3
#endif
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

typedef uint32_t millis_t;
typedef int16_t pintype_t;
typedef int8_t aposi_t;
typedef uint16_t akey_t;
typedef uint32_t aframe_t;

static const pintype_t apin_none = -1;
static const pintype_t apin_virtual = 100;
static const int8_t apinchnl_none = -127;
static const akey_t akey_none = 0;
static const aframe_t aframe_none = 0;

inline millis_t astroMillis()
{
#ifdef ARDUINO
    return millis();
#else
    using namespace std::chrono;
    return (millis_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
#endif
}

inline millis_t astroNZMillis()
{
    millis_t now = astroMillis();
    return now ? now : 1;
}

template<class T>
inline T astroConstrain(T value, T low, T high)
{
    return value < low ? low : (value > high ? high : value);
}

#endif // /ifndef AstroCompat_H
