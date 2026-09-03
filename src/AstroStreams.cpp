/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Streams
*/

#include "Astruino.h"
#include <limits.h>

AstroEEPROMStream::AstroEEPROMStream()
    : Stream(), _eeprom(nullptr), _readAddress(0), _writeAddress(0), _endAddress(0)
{
    if (getController() && (_eeprom = getController()->getEEPROM())) {
        _endAddress = _eeprom->getDeviceSize();
    }
    ASTRO_HARD_ASSERT(_eeprom, SFP(AStr_Err_UnsupportedOperation));
}

AstroEEPROMStream::AstroEEPROMStream(uint16_t dataAddress, size_t dataSize)
      : Stream(), _eeprom(nullptr), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + dataSize)
{
    if (getController()) {
        _eeprom = getController()->getEEPROM();
    }
    ASTRO_HARD_ASSERT(_eeprom, SFP(AStr_Err_UnsupportedOperation));
}

int AstroEEPROMStream::available()
{
    if (!_eeprom || _readAddress >= _endAddress) { return 0; }
    uint32_t available = _endAddress - _readAddress;
    return available > INT_MAX ? INT_MAX : (int)available;
}

int AstroEEPROMStream::read()
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    return (int)_eeprom->readByte(_readAddress++);
}

size_t AstroEEPROMStream::readBytes(char *buffer, size_t length)
{
    if (!_eeprom || !buffer || !length || _readAddress >= _endAddress) { return 0; }
    uint32_t remaining = _endAddress - _readAddress;
    if (length > remaining) { length = remaining; }

    size_t bytesRead = 0;
    while (bytesRead < length) {
        size_t chunkSize = length - bytesRead;
        if (chunkSize > UINT16_MAX) { chunkSize = UINT16_MAX; }
        size_t chunkRead = _eeprom->readBlock((uint16_t)_readAddress, (uint8_t *)buffer + bytesRead, (uint16_t)chunkSize);
        _readAddress += chunkRead;
        bytesRead += chunkRead;
        if (chunkRead != chunkSize) { break; }
    }
    return bytesRead;
}

int AstroEEPROMStream::peek()
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    return (int)_eeprom->readByte(_readAddress);
}

void AstroEEPROMStream::flush()
{
    //_eeprom->commit();
}

size_t AstroEEPROMStream::write(const uint8_t *buffer, size_t size)
{
    if (!_eeprom || !buffer || !size || _writeAddress >= _endAddress) { return 0; }
    uint32_t remaining = _endAddress - _writeAddress;
    if (size > remaining) { size = remaining; }

    size_t bytesWritten = 0;
    while (bytesWritten < size) {
        size_t chunkSize = size - bytesWritten;
        if (chunkSize > UINT16_MAX) { chunkSize = UINT16_MAX; }
        if (!_eeprom->updateBlockVerify((uint16_t)_writeAddress, buffer + bytesWritten, (uint16_t)chunkSize)) {
            ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_OperationFailure));
            break;
        }
        _writeAddress += chunkSize;
        bytesWritten += chunkSize;
    }
    return bytesWritten;
}

size_t AstroEEPROMStream::write(uint8_t data)
{
    if (!_eeprom || _writeAddress >= _endAddress) { return 0; }
    if (_eeprom->updateByteVerify(_writeAddress, data)) {
        _writeAddress += 1;
        return 1;
    } else {
        ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_OperationFailure));
        return 0;
    }
}

int AstroEEPROMStream::availableForWrite()
{
    if (!_eeprom || _writeAddress >= _endAddress) { return 0; }
    uint32_t available = _endAddress - _writeAddress;
    return available > INT_MAX ? INT_MAX : (int)available;
}


AstroPROGMEMStream::AstroPROGMEMStream()
    : Stream(), _readAddress(0), _writeAddress(0), _endAddress(UINTPTR_MAX)
{ ; }

AstroPROGMEMStream::AstroPROGMEMStream(uintptr_t dataAddress)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + strlen_P((const char *)dataAddress))
{ ; }

AstroPROGMEMStream::AstroPROGMEMStream(uintptr_t dataAddress, size_t dataSize)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + dataSize)
{ ; }

int AstroPROGMEMStream::available()
{
    return _endAddress - _readAddress;
}

int AstroPROGMEMStream::read()
{
    if (_readAddress >= _endAddress) { return -1; }
    #ifdef ESP8266
        return pgm_read_byte((const void *)(_readAddress++));
    #else
        return pgm_read_byte(_readAddress++);
    #endif
}

int AstroPROGMEMStream::peek()
{
    if (_readAddress >= _endAddress) { return -1; }
    #ifdef ESP8266
        return pgm_read_byte((const void *)(_readAddress));
    #else
        return pgm_read_byte(_readAddress);
    #endif
}

void AstroPROGMEMStream::flush()
{ ; }

size_t AstroPROGMEMStream::write(const uint8_t *buffer, size_t size)
{
    (void)buffer;
    (void)size;
    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_OperationFailure));
    return 0;
}

size_t AstroPROGMEMStream::write(uint8_t data)
{
    (void)data;
    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_OperationFailure));
    return 0;
}


#ifdef ASTRO_USE_WIFI_STORAGE

AstroWiFiStorageFileStream::AstroWiFiStorageFileStream(WiFiStorageFile file, uint32_t seekPos)
    : Stream(), _file(file), _buffer{0}, _bufferOffset(0), _bufferFileOffset(-1), _bufferDirection(None), _readOffset(0), _writeOffset(0), _endOffset(0)
{
    _endOffset = _file.size();
    _readOffset = _writeOffset = seekPos;
}

AstroWiFiStorageFileStream::~AstroWiFiStorageFileStream()
{
    flush();
}

int AstroWiFiStorageFileStream::available()
{
    if (!_file || _readOffset >= _endOffset) { return 0; }
    uint32_t available = _endOffset - _readOffset;
    return available > INT_MAX ? INT_MAX : (int)available;
}

int AstroWiFiStorageFileStream::read()
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    prepareReadBuffer();
    _readOffset++;
    return _buffer[_bufferOffset++];
}

size_t AstroWiFiStorageFileStream::readBytes(char *buffer, size_t length)
{
    if (!_file || !buffer || !length || _readOffset >= _endOffset) { return 0; }
    size_t bytesRead = 0;
    while (length && _readOffset < _endOffset) {
        prepareReadBuffer();
        size_t howMany = min(length, _endOffset - _readOffset);
        howMany = min(howMany, ASTRO_WIFISTREAM_BUFFER_SIZE - _bufferOffset);
        memcpy(buffer, &_buffer[_bufferOffset], howMany);
        _readOffset += howMany;
        _bufferOffset += howMany;
        buffer += howMany;
        length -= howMany;
        bytesRead += howMany;
    }
    return bytesRead;
}

int AstroWiFiStorageFileStream::peek()
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    prepareReadBuffer();
    return _buffer[_bufferOffset];
}

void AstroWiFiStorageFileStream::flush()
{
    if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
        _file.seek(_bufferFileOffset);
        _file.write((const void*)_buffer, _bufferOffset); _bufferOffset = 0;
        _bufferFileOffset = _writeOffset;
    }
}

size_t AstroWiFiStorageFileStream::write(const uint8_t *buffer, size_t size)
{
    if (!buffer || !size) { return 0; }
    size_t bytesWritten = 0;
    while (size) {
        prepareWriteBuffer();
        size_t howMany = min(size, ASTRO_WIFISTREAM_BUFFER_SIZE - _bufferOffset);
        memcpy(&_buffer[_bufferOffset], buffer, howMany);
        _writeOffset += howMany;
        _bufferOffset += howMany;
        buffer += howMany;
        size -= howMany;
        bytesWritten += howMany;
    }
    if (_writeOffset > _endOffset) { _endOffset = _writeOffset; }
    return bytesWritten;
}

size_t AstroWiFiStorageFileStream::write(uint8_t data)
{
    prepareWriteBuffer();
    _buffer[_bufferOffset++] = data;
    _writeOffset++;
    if (_writeOffset > _endOffset) { _endOffset = _writeOffset; }
    return 1;
}

int AstroWiFiStorageFileStream::availableForWrite() 
{
    return _bufferDirection == WriteBuffer ? ASTRO_WIFISTREAM_BUFFER_SIZE - _bufferOffset : ASTRO_WIFISTREAM_BUFFER_SIZE;
}

void AstroWiFiStorageFileStream::prepareReadBuffer()
{
    if (_bufferDirection != ReadBuffer || _bufferFileOffset == UINT32_MAX || _readOffset < _bufferFileOffset || _readOffset >= _bufferFileOffset + ASTRO_WIFISTREAM_BUFFER_SIZE) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); //_bufferOffset = 0;
        }
        _bufferDirection = ReadBuffer;
        _bufferFileOffset = _readOffset;
        _bufferOffset = 0;

        _file.seek(_bufferFileOffset);
        _file.read((void *)_buffer, ASTRO_WIFISTREAM_BUFFER_SIZE);
    }
}

void AstroWiFiStorageFileStream::prepareWriteBuffer()
{
    if (_bufferDirection != WriteBuffer || _bufferFileOffset == UINT32_MAX || _writeOffset < _bufferFileOffset || _writeOffset >= _bufferFileOffset + ASTRO_WIFISTREAM_BUFFER_SIZE) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); //_bufferOffset = 0;
        }
        _bufferDirection = WriteBuffer;
        _bufferFileOffset = _writeOffset;
        _bufferOffset = 0;
    }
}

#endif
