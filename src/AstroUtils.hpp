/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Utility Templates
*/

#ifndef AstroUtils_HPP
#define AstroUtils_HPP

#include <stdio.h>
#include <stdlib.h>

// Converts a numeric array into the compact comma-separated representation used by
// several serialized configuration values in the sibling libraries.
template<class T>
AstroString commaStringFromArray(const T *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return AstroString(); }
    AstroString retVal;
    for (size_t index = 0; index < length; ++index) {
        if (index) { retVal += ','; }
        char valueBuffer[32];
        snprintf(valueBuffer, sizeof(valueBuffer), "%.12g", (double)arrayIn[index]);
        retVal += valueBuffer;
    }
    return retVal;
}

// Parses a compact comma-separated numeric representation into a fixed-size array.
template<class T>
void commaStringToArray(const AstroString &stringIn, T *arrayOut, size_t length)
{
    if (!arrayOut || !length) { return; }
    const char *cursor = stringIn.c_str();
    for (size_t index = 0; index < length; ++index) {
        if (!cursor || !*cursor) { arrayOut[index] = T(); continue; }
        char *end = nullptr;
        const double value = strtod(cursor, &end);
        arrayOut[index] = end != cursor ? (T)value : T();
        if (!end || !*end) { cursor = nullptr; }
        else { cursor = *end == ',' ? end + 1 : end; }
    }
}

#endif // /ifndef AstroUtils_HPP
