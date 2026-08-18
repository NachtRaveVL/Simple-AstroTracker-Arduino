/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Streams
*/

#ifndef AstroStreams_H
#define AstroStreams_H

#include "AstroCompat.h"

// Stream Base
// Minimal byte stream used by serialization helpers without requiring a specific storage device.
class AstroStream {
public:
    virtual ~AstroStream() { ; }

    virtual int read() = 0;
    virtual size_t available() const = 0;
    virtual void reset() = 0;

    size_t readBytes(void *bufferOut, size_t length);
};

// Memory Stream
// Reads sequential bytes from a normal memory buffer.
class AstroMemoryStream : public AstroStream {
public:
    AstroMemoryStream(const uint8_t *data = nullptr, size_t size = 0);

    virtual int read() override;
    virtual size_t available() const override;
    virtual void reset() override;

protected:
    const uint8_t *_data;                                    // Bound serialization data, not owned
    size_t _size;                                            // Size
    size_t _position;                                        // Current normalized position
};

// PROGMEM Stream
// Reads sequential bytes from Flash/PROGMEM data.
class AstroPROGMEMStream : public AstroStream {
public:
    AstroPROGMEMStream(const void *data = nullptr, size_t size = 0);

    virtual int read() override;
    virtual size_t available() const override;
    virtual void reset() override;

protected:
    const uint8_t *_data;                                    // Bound serialization data, not owned
    size_t _size;                                            // Size
    size_t _position;                                        // Current normalized position
};

#endif // /ifndef AstroStreams_H
