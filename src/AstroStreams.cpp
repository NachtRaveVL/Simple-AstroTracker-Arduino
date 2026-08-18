/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Streams
*/

#include "AstroStreams.h"
#ifdef __AVR__
#include <avr/pgmspace.h>
#endif

size_t AstroStream::readBytes(void *bufferOut, size_t length)
{
    if (!bufferOut || !length) { return 0; }
    uint8_t *buffer = (uint8_t *)bufferOut;
    size_t count = 0;
    while (count < length && available()) {
        int value = read();
        if (value < 0) { break; }
        buffer[count++] = (uint8_t)value;
    }
    return count;
}

AstroMemoryStream::AstroMemoryStream(const uint8_t *data, size_t size)
    : _data(data), _size(size), _position(0)
{ ; }

int AstroMemoryStream::read()
{
    return _data && _position < _size ? _data[_position++] : -1;
}

size_t AstroMemoryStream::available() const
{
    return _data && _position < _size ? _size - _position : 0;
}

void AstroMemoryStream::reset()
{
    _position = 0;
}

AstroPROGMEMStream::AstroPROGMEMStream(const void *data, size_t size)
    : _data((const uint8_t *)data), _size(size), _position(0)
{ ; }

int AstroPROGMEMStream::read()
{
    if (!_data || _position >= _size) { return -1; }
#ifdef __AVR__
    return pgm_read_byte(_data + _position++);
#else
    return _data[_position++];
#endif
}

size_t AstroPROGMEMStream::available() const
{
    return _data && _position < _size ? _size - _position : 0;
}

void AstroPROGMEMStream::reset()
{
    _position = 0;
}
