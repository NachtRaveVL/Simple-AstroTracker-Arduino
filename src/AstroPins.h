/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Pins
*/

#ifndef AstroPins_H
#define AstroPins_H

#include "AstroDefines.h"
#include "AstroInterfaces.h"

struct AstroPin;
struct AstroPinData;

// Creates Pin from passed Pin data (return ownership transfer - user code *must* delete returned Pin)
extern AstroPin *newPinObjectFromSubData(const AstroPinData *dataIn);

inline pintype_t pinNumberForPinChannel(int8_t pinChannel) {
    return pinChannel >= 0 ? (pintype_t)(apin_virtual + pinChannel) : apin_none;
}
inline uint8_t pinChannelOrPinNumber(pintype_t pinNumber) {
    return pinNumber >= 0 ? (uint8_t)(pinNumber >= apin_virtual ? (pinNumber - apin_virtual) % 16 : pinNumber) : (uint8_t)-1;
}
inline aposi_t expanderPosForPinNumber(pintype_t pinNumber) {
    return pinNumber >= apin_virtual ? (aposi_t)((pinNumber - apin_virtual) / 16) : (aposi_t)-1;
}
inline aposi_t expanderPosForPinChannel(int8_t pinChannel) {
    return pinChannel >= 0 ? (aposi_t)(pinChannel / 16) : (aposi_t)-1;
}
inline uint8_t muxerChannelForPinChannel(int8_t pinChannel) {
    return pinChannel < 0 && pinChannel != apinchnl_none ? (uint8_t)(-pinChannel - 1) : (uint8_t)-1;
}
inline int8_t pinChannelForMuxerChannel(uint8_t muxChannel) {
    return muxChannel < 126 ? -(int8_t)(muxChannel + 1) : apinchnl_none;
}
inline int8_t pinChannelForExpanderChannel(uint8_t expChannel) {
    return expChannel < 127 ? (int8_t)expChannel : apinchnl_none;
}

// Pin Base
// Common pin identity, mode, channel, and initialization state.
struct AstroPin {
    enum : signed char { Digital, Analog, Unknown = -1 } type;

    pintype_t pin;                                           // Physical or virtual pin number
    Astro_PinMode mode;                                      // Pin operating mode
    int8_t channel;                                          // Mux/expander channel

    AstroPin(int classType = Unknown, pintype_t pinNumber = apin_none,
             Astro_PinMode pinMode = Astro_PinMode_Undefined, int8_t pinChannel = apinchnl_none); // Pin mode
    virtual ~AstroPin() { ; }
    AstroPin(const AstroPinData *dataIn);

    void saveToData(AstroPinData *dataOut) const;

    void init();
    void deinit();

    inline bool isValid() const { return pin >= 0 && mode != Astro_PinMode_Undefined; }
    inline bool isVirtual() const { return pin >= apin_virtual; }
    inline bool isMuxed() const { return channel < 0 && channel != apinchnl_none; }
    inline bool isExpanded() const { return channel >= 0; }
    inline bool isInput() const {
        return mode == Astro_PinMode_Digital_Input ||
               mode == Astro_PinMode_Digital_Input_PullUp ||
               mode == Astro_PinMode_Digital_Input_PullDown ||
               mode == Astro_PinMode_Analog_Input;           // Pin operating mode
    }
    inline bool isOutput() const {
        return mode == Astro_PinMode_Digital_Output ||
               mode == Astro_PinMode_Digital_Output_PushPull ||
               mode == Astro_PinMode_Analog_Output;          // Pin operating mode
    }
    inline bool canRead() const { return isValid() && isInput(); }
    inline bool canWrite() const { return isValid() && isOutput(); }
};

// Digital Pin
// Digital input/output pin with optional active-low handling.
struct AstroDigitalPin : public AstroPin, public AstroDigitalInputPinInterface, public AstroDigitalOutputPinInterface {
    bool activeLow;                                          // Active-low logic flag

    AstroDigitalPin(pintype_t pinNumber = apin_none,
                    Astro_PinMode pinMode = Astro_PinMode_Undefined,
                    bool isActiveLow = false,
                    int8_t pinChannel = apinchnl_none);      // Pin channel
    AstroDigitalPin(const AstroPinData *dataIn);

    void saveToData(AstroPinData *dataOut) const;

    virtual int digitalRead() override;
    virtual void digitalWrite(int status) override;

    inline bool isActive() { return digitalRead() == (activeLow ? LOW : HIGH); }
    inline void activate() { digitalWrite(activeLow ? LOW : HIGH); }
    inline void deactivate() { digitalWrite(activeLow ? HIGH : LOW); }
};

// Analog Pin
// Analog input/output pin with normalized bit-resolution conversion.
struct AstroAnalogPin : public AstroPin, public AstroAnalogInputPinInterface, public AstroAnalogOutputPinInterface {
    uint8_t bitRes;                                          // Analog bit resolution

    AstroAnalogPin(pintype_t pinNumber = apin_none,
                   Astro_PinMode pinMode = Astro_PinMode_Undefined,
                   uint8_t analogBitRes = 10,
                   int8_t pinChannel = apinchnl_none);       // Pin channel
    AstroAnalogPin(const AstroPinData *dataIn);

    void saveToData(AstroPinData *dataOut) const;

    virtual float analogRead() override;
    virtual int analogRead_raw() override;
    virtual void analogWrite(float amount) override;
    virtual void analogWrite_raw(int amount) override;
};

// Pin Serialization Data
// Serializable pin number, mode, channel, active state, and bit resolution.
struct AstroPinData {
    int8_t type;                                             // Type
    pintype_t pin;                                           // Physical or virtual pin number
    Astro_PinMode mode;                                      // Pin operating mode
    int8_t channel;                                          // Mux/expander channel
    bool activeLow;                                          // Active-low logic flag
    uint8_t bitRes;                                          // Analog bit resolution

    AstroPinData();
    bool toJSON(char *bufferOut, size_t bufferSize) const;
    bool fromJSON(const char *jsonIn);
};

// Pin Muxer
// Selects channels on simple address-line multiplexers used to expand available I/O.
class AstroPinMuxer {
public:
    AstroPinMuxer();
    AstroPinMuxer(AstroPin signalPin, const pintype_t *channelPins, uint8_t channelBits,
                  AstroDigitalPin chipEnablePin = AstroDigitalPin());

    void init();
    void selectChannel(uint8_t channelNumber);
    void setIsActive(bool isActive);

    inline void activate() { setIsActive(true); }
    inline void deactivate() { setIsActive(false); }
    inline const AstroPin &getSignalPin() const { return _signal; }
    inline const AstroDigitalPin &getChipEnablePin() const { return _chipEnable; }
    inline uint8_t getChannelBits() const { return _channelBits; }
    inline uint8_t getSelectedChannel() const { return _channelSelect; }

protected:
    AstroPin _signal;                                        // Signal
    AstroDigitalPin _chipEnable;                             // Chip enable
    pintype_t _channelPins[4];                               // Channel pins
    uint8_t _channelBits;                                    // Channel bits
    uint8_t _channelSelect;                                  // Channel select
};

#ifndef ARDUINO
extern void astroSetHostDigitalPin(pintype_t pin, int value);
extern int astroGetHostDigitalPin(pintype_t pin);
extern void astroSetHostAnalogPin(pintype_t pin, int value);
extern int astroGetHostAnalogPin(pintype_t pin);
#endif

#endif // /ifndef AstroPins_H
