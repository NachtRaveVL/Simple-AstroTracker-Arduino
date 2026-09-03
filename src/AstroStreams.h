/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Streams
*/

#ifndef AstroStreams_H
#define AstroStreams_H

class AstroEEPROMStream;
class AstroPROGMEMStream;

#include "Astruino.h"

#ifdef ARDUINO_ARCH_SAM // Stream doesn't have availableForWrite
#define ASTRO_STREAM_AVAIL4WRT_OVERRIDE
#else
#define ASTRO_STREAM_AVAIL4WRT_OVERRIDE override
#endif

// EEPROM Stream
// Stream class for working with I2C_EEPROM data.
class AstroEEPROMStream : public Stream {
public:
    AstroEEPROMStream();
    AstroEEPROMStream(uint16_t dataAddress, size_t dataSize);

    virtual int available() override;
    virtual int read() override;
    size_t readBytes(char *buffer, size_t length);
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;
    virtual int availableForWrite() ASTRO_STREAM_AVAIL4WRT_OVERRIDE;

protected:
    I2C_eeprom *_eeprom;
    uint32_t _readAddress, _writeAddress, _endAddress;
};


// PROGMEM Stream
// Stream class for working with PROGMEM data.
class AstroPROGMEMStream : public Stream {
public:
    AstroPROGMEMStream();
    AstroPROGMEMStream(uintptr_t dataAddress);
    AstroPROGMEMStream(uintptr_t dataAddress, size_t dataSize);

    virtual int available() override;
    virtual int read() override;
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;

protected:
    uintptr_t _readAddress, _writeAddress, _endAddress;
};

#ifdef ASTRO_USE_WIFI_STORAGE

class AstroWiFiStorageFileStream : public Stream {
public:
    AstroWiFiStorageFileStream(WiFiStorageFile file, uint32_t seekPos = 0);
    virtual ~AstroWiFiStorageFileStream();

    virtual int available() override;
    virtual int read() override;
    size_t readBytes(char *buffer, size_t length);
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;
    virtual int availableForWrite() ASTRO_STREAM_AVAIL4WRT_OVERRIDE;

protected:
    enum WiFiStorageFileDirection : signed char { ReadBuffer, WriteBuffer, None = -1 };

    WiFiStorageFile _file;
    uint8_t _buffer[ASTRO_WIFISTREAM_BUFFER_SIZE];
    size_t _bufferOffset;
    uint32_t _bufferFileOffset;
    WiFiStorageFileDirection _bufferDirection;
    uint32_t _readOffset, _writeOffset, _endOffset;

    void prepareReadBuffer();
    void prepareWriteBuffer();
};

#endif

#endif // /ifndef AstroStreams_H
