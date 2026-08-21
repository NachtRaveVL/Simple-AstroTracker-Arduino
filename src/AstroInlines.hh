/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Common Inlines
*/

#ifndef AstroInlines_HH
#define AstroInlines_HH

struct AstroI2CDeviceSetup;
struct AstroSPIDeviceSetup;
struct AstroUARTDeviceSetup;
struct AstroDeviceSetup;
struct AstroBitResolution;
struct AstroTwilight;

#include "AstroDefines.h"
#include "AstroCoordinates.h"
#include <math.h>

// Returns if pin is valid.
inline bool isValidPin(pintype_t pin) { return pin != apin_none; }
// Returns if pin channel is valid.
inline bool isValidChannel(int8_t channel) { return channel != apinchnl_none; }
// Returns if measurement row is valid.
inline bool isValidRow(uint8_t row) { return row != (uint8_t)-1; }
// Returns if time millis is valid.
inline bool isValidTime(millis_t time) { return time != 0; }
// Returns if position index is valid.
inline bool isValidIndex(aposi_t index) { return index >= 0; }
// Returns if id key is valid.
inline bool isValidKey(akey_t key) { return key != akey_none; }
// Returns if frame is valid.
inline bool isValidFrame(aframe_t frame) { return frame != aframe_none; }

// Returns if two single-precision floating point values are equal within the library epsilon.
inline bool isFPEqual(float lhs, float rhs) { return fabsf(rhs - lhs) <= FLT_EPSILON; }
// Returns if two double-precision floating point values are equal within the library epsilon.
inline bool isFPEqual(double lhs, double rhs) { return fabs(rhs - lhs) <= DBL_EPSILON; }

// Returns the first units value that is not undefined.
inline Astro_UnitsType definedUnitsElse(Astro_UnitsType units1, Astro_UnitsType units2)
{
    return units1 != Astro_UnitsType_Undefined ? units1 : units2;
}

// Returns the first units value that is not undefined.
inline Astro_UnitsType definedUnitsElse(Astro_UnitsType units1, Astro_UnitsType units2, Astro_UnitsType units3)
{
    return units1 != Astro_UnitsType_Undefined ? units1 :
           units2 != Astro_UnitsType_Undefined ? units2 : units3;
}

// Returns a signed -1 when a pin is not valid, useful during serialization and display.
inline int intForPin(pintype_t pin) { return isValidPin(pin) ? (int)pin : -1; }

// Rounds a floating point value to a requested number of decimal places.
inline double roundToDecimalPlaces(double value, int decimalPlaces)
{
    if (decimalPlaces >= 0) {
        const double shiftScaler = pow(10.0, decimalPlaces);
        return round(value * shiftScaler) / shiftScaler;
    }
    return value;
}

#ifdef ARDUINO
// I2C Device Setup
// Quick storage for I2C device connection settings.
struct AstroI2CDeviceSetup {
    TwoWire *wire;                                          // I2C wire instance
    uint32_t speed;                                         // I2C max data speed, in Hz
    uint8_t address;                                        // I2C device address

    inline AstroI2CDeviceSetup(TwoWire *i2cWire = ASTRO_USE_WIRE, uint32_t i2cSpeed = 100000U, uint8_t i2cAddress = 0)
        : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
};

// SPI Device Setup
// Quick storage for SPI device connection settings.
struct AstroSPIDeviceSetup {
    SPIClass *spi;                                          // SPI class instance
    uint32_t speed;                                         // SPI max data speed, in Hz
    pintype_t cs;                                           // SPI chip-select pin

    inline AstroSPIDeviceSetup(SPIClass *spiClass = ASTRO_USE_SPI, uint32_t spiSpeed = 1000000U, pintype_t spiCS = apin_none)
        : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
};

// UART Device Setup
// Quick storage for serial device connection settings.
struct AstroUARTDeviceSetup {
    SerialClass *serial;                                    // UART class instance
    uint32_t baud;                                          // UART baud rate, in bits per second

    inline AstroUARTDeviceSetup(SerialClass *serialClass = ASTRO_USE_SERIAL1, uint32_t serialBaud = 9600U)
        : serial(serialClass), baud(serialBaud) { ; }
};
#else
// Host versions retain the same configuration shape without requiring Arduino classes.
struct AstroI2CDeviceSetup {
    void *wire;                                             // Host I2C instance placeholder
    uint32_t speed;                                         // I2C max data speed, in Hz
    uint8_t address;                                        // I2C device address
    inline AstroI2CDeviceSetup(void *i2cWire = nullptr, uint32_t i2cSpeed = 100000U, uint8_t i2cAddress = 0)
        : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
};
// SPI Device Setup
// Host-side equivalent of the SPI connection settings structure.
struct AstroSPIDeviceSetup {
    void *spi;                                              // Host SPI instance placeholder
    uint32_t speed;                                         // SPI max data speed, in Hz
    pintype_t cs;                                           // SPI chip-select pin
    inline AstroSPIDeviceSetup(void *spiClass = nullptr, uint32_t spiSpeed = 1000000U, pintype_t spiCS = apin_none)
        : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
};
// UART Device Setup
// Host-side equivalent of the serial connection settings structure.
struct AstroUARTDeviceSetup {
    void *serial;                                           // Host UART instance placeholder
    uint32_t baud;                                          // UART baud rate, in bits per second
    inline AstroUARTDeviceSetup(void *serialClass = nullptr, uint32_t serialBaud = 9600U)
        : serial(serialClass), baud(serialBaud) { ; }
};
#endif

// Combined Device Setup
// Tagged union for the common I2C, SPI, and UART connection settings.
struct AstroDeviceSetup {
    enum : signed char { None, I2CSetup, SPISetup, UARTSetup } cfgType; // Configuration type
    AstroI2CDeviceSetup i2c;                                // I2C configuration
    AstroSPIDeviceSetup spi;                                // SPI configuration
    AstroUARTDeviceSetup uart;                              // UART configuration

    inline AstroDeviceSetup() : cfgType(None), i2c(), spi(), uart() { ; }
    inline AstroDeviceSetup(const AstroI2CDeviceSetup &setup) : cfgType(I2CSetup), i2c(setup), spi(), uart() { ; }
    inline AstroDeviceSetup(const AstroSPIDeviceSetup &setup) : cfgType(SPISetup), i2c(), spi(setup), uart() { ; }
    inline AstroDeviceSetup(const AstroUARTDeviceSetup &setup) : cfgType(UARTSetup), i2c(), spi(), uart(setup) { ; }
};

// Analog Bit Resolution
// Converts between raw ADC/DAC values and normalized [0,1] values.
struct AstroBitResolution {
    uint8_t bits;                                           // Bit resolution
    int32_t maxVal;                                         // Maximum raw value

    inline AstroBitResolution(uint8_t numBits = 8)
        : bits(numBits), maxVal((1L << (numBits > 30 ? 30 : numBits)) - 1) { ; }

    inline float transform(int value) const
    {
        return maxVal > 0 ? constrain(value / (float)maxVal, 0.0f, 1.0f) : 0.0f;
    }

    inline int inverseTransform(float value) const
    {
        return constrain((int)roundf(maxVal * constrain(value, 0.0f, 1.0f)), 0, maxVal);
    }
};

// Twilight Timing Data
// Stores daily sunrise/sunset style thresholds for simple day/night scheduling.
struct AstroTwilight {
    double dawnHour;                                        // Dawn hour, fractional local or UTC
    double duskHour;                                        // Dusk hour, fractional local or UTC
    bool isUTC;                                             // Hours are in UTC when true

    inline AstroTwilight(double dawnHourIn = 6.0, double duskHourIn = 18.0, bool isUTCIn = false)
        : dawnHour(dawnHourIn), duskHour(duskHourIn), isUTC(isUTCIn) { ; }

    inline bool isDaytimeHour(double hour) const
    {
        return dawnHour <= duskHour ? hour >= dawnHour && hour <= duskHour
                                    : hour >= dawnHour || hour <= duskHour;
    }
    inline bool isNighttimeHour(double hour) const { return !isDaytimeHour(hour); }
};

#endif // /ifndef AstroInlines_HH
