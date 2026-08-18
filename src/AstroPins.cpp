/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Pins
*/

#include "AstroPins.h"
#include "AstroUtils.h"
#include <stdio.h>
#include <string.h>

#ifndef ARDUINO
static int hostDigitalPins[256] = {0};
static int hostAnalogPins[256] = {0};

void astroSetHostDigitalPin(pintype_t pin, int value) { if (pin >= 0 && pin < 256) { hostDigitalPins[pin] = value ? HIGH : LOW; } }
int astroGetHostDigitalPin(pintype_t pin) { return pin >= 0 && pin < 256 ? hostDigitalPins[pin] : LOW; }
void astroSetHostAnalogPin(pintype_t pin, int value) { if (pin >= 0 && pin < 256) { hostAnalogPins[pin] = value; } }
int astroGetHostAnalogPin(pintype_t pin) { return pin >= 0 && pin < 256 ? hostAnalogPins[pin] : 0; }
#endif

AstroPin *newPinObjectFromSubData(const AstroPinData *dataIn)
{
    if (!dataIn) { return nullptr; }
    switch (dataIn->type) {
        case AstroPin::Digital: return new AstroDigitalPin(dataIn);
        case AstroPin::Analog: return new AstroAnalogPin(dataIn);
        default: return nullptr;
    }
}

AstroPin::AstroPin(int classType, pintype_t pinNumber, Astro_PinMode pinMode, int8_t pinChannel)
    : type((decltype(type))classType), pin(pinNumber), mode(pinMode), channel(pinChannel)
{ ; }

AstroPin::AstroPin(const AstroPinData *dataIn)
    : type(dataIn ? (decltype(type))dataIn->type : Unknown), pin(dataIn ? dataIn->pin : apin_none),
      mode(dataIn ? dataIn->mode : Astro_PinMode_Undefined), channel(dataIn ? dataIn->channel : apinchnl_none)
{ ; }

void AstroPin::saveToData(AstroPinData *dataOut) const
{
    if (!dataOut) { return; }
    dataOut->type = (int8_t)type;
    dataOut->pin = pin;
    dataOut->mode = mode;
    dataOut->channel = channel;
}

void AstroPin::init()
{
#ifdef ARDUINO
    if (!isValid() || isVirtual()) { return; }
    switch (mode) {
        case Astro_PinMode_Digital_Input: pinMode(pin, INPUT); break;
        case Astro_PinMode_Digital_Input_PullUp: pinMode(pin, INPUT_PULLUP); break;
        #ifdef INPUT_PULLDOWN
        case Astro_PinMode_Digital_Input_PullDown: pinMode(pin, INPUT_PULLDOWN); break;
        #else
        case Astro_PinMode_Digital_Input_PullDown: pinMode(pin, INPUT); break;
        #endif
        case Astro_PinMode_Digital_Output:
        case Astro_PinMode_Digital_Output_PushPull:
        case Astro_PinMode_Analog_Output: pinMode(pin, OUTPUT); break;
        case Astro_PinMode_Analog_Input: pinMode(pin, INPUT); break;
        default: break;
    }
#endif
}

void AstroPin::deinit()
{
#ifdef ARDUINO
    if (pin >= 0 && !isVirtual()) { pinMode(pin, INPUT); }
#endif
}

AstroDigitalPin::AstroDigitalPin(pintype_t pinNumber, Astro_PinMode pinMode, bool isActiveLow, int8_t pinChannel)
    : AstroPin(Digital, pinNumber, pinMode, pinChannel), activeLow(isActiveLow)
{ ; }

AstroDigitalPin::AstroDigitalPin(const AstroPinData *dataIn)
    : AstroPin(dataIn), activeLow(dataIn ? dataIn->activeLow : false)
{
    type = Digital;
}

void AstroDigitalPin::saveToData(AstroPinData *dataOut) const
{
    AstroPin::saveToData(dataOut);
    if (dataOut) { dataOut->type = Digital; dataOut->activeLow = activeLow; }
}

int AstroDigitalPin::digitalRead()
{
    if (!canRead() && !isOutput()) { return LOW; }
#ifdef ARDUINO
    return ::digitalRead(pin);
#else
    return astroGetHostDigitalPin(pin);
#endif
}

void AstroDigitalPin::digitalWrite(int status)
{
    if (!canWrite()) { return; }
#ifdef ARDUINO
    ::digitalWrite(pin, status ? HIGH : LOW);
#else
    astroSetHostDigitalPin(pin, status);
#endif
}

AstroAnalogPin::AstroAnalogPin(pintype_t pinNumber, Astro_PinMode pinMode, uint8_t analogBitRes, int8_t pinChannel)
    : AstroPin(Analog, pinNumber, pinMode, pinChannel), bitRes(analogBitRes ? analogBitRes : 10)
{ ; }

AstroAnalogPin::AstroAnalogPin(const AstroPinData *dataIn)
    : AstroPin(dataIn), bitRes(dataIn && dataIn->bitRes ? dataIn->bitRes : 10)
{
    type = Analog;
}

void AstroAnalogPin::saveToData(AstroPinData *dataOut) const
{
    AstroPin::saveToData(dataOut);
    if (dataOut) { dataOut->type = Analog; dataOut->bitRes = bitRes; }
}

float AstroAnalogPin::analogRead()
{
    int raw = analogRead_raw();
    int maxValue = (1 << (bitRes > 15 ? 15 : bitRes)) - 1;
    return maxValue ? (float)raw / (float)maxValue : 0.0f;
}

int AstroAnalogPin::analogRead_raw()
{
    if (!canRead()) { return 0; }
#ifdef ARDUINO
    return ::analogRead(pin);
#else
    return astroGetHostAnalogPin(pin);
#endif
}

void AstroAnalogPin::analogWrite(float amount)
{
    amount = astroConstrain(amount, 0.0f, 1.0f);
    int maxValue = (1 << (bitRes > 15 ? 15 : bitRes)) - 1;
    analogWrite_raw((int)(amount * maxValue + 0.5f));
}

void AstroAnalogPin::analogWrite_raw(int amount)
{
    if (!canWrite()) { return; }
    int maxValue = (1 << (bitRes > 15 ? 15 : bitRes)) - 1;
    amount = astroConstrain(amount, 0, maxValue);
#ifdef ARDUINO
    ::analogWrite(pin, amount);
#else
    astroSetHostAnalogPin(pin, amount);
#endif
}

AstroPinData::AstroPinData()
    : type(AstroPin::Unknown), pin(apin_none), mode(Astro_PinMode_Undefined),
      channel(apinchnl_none), activeLow(false), bitRes(10)
{ ; }

bool AstroPinData::toJSON(char *bufferOut, size_t bufferSize) const
{
    if (!bufferOut || !bufferSize) { return false; }
    int written = snprintf(bufferOut, bufferSize,
        "{\"type\":%d,\"pin\":%d,\"mode\":%d,\"channel\":%d,\"activeLow\":%d,\"bitRes\":%u}",
        type, (int)pin, (int)mode, (int)channel, activeLow ? 1 : 0, (unsigned int)bitRes);
    return written >= 0 && (size_t)written < bufferSize;
}

bool AstroPinData::fromJSON(const char *jsonIn)
{
    if (!jsonIn) { return false; }

    long typeIn, pinIn, modeIn, channelIn, activeLowIn;
    unsigned long bitResIn;
    if (!astroJSONGetLong(jsonIn, "type", &typeIn) ||
        !astroJSONGetLong(jsonIn, "pin", &pinIn) ||
        !astroJSONGetLong(jsonIn, "mode", &modeIn) ||
        !astroJSONGetLong(jsonIn, "channel", &channelIn) ||
        !astroJSONGetLong(jsonIn, "activeLow", &activeLowIn) ||
        !astroJSONGetUnsignedLong(jsonIn, "bitRes", &bitResIn)) {
        return false;
    }

    type = (int8_t)typeIn;
    pin = (pintype_t)pinIn;
    mode = (Astro_PinMode)modeIn;
    channel = (int8_t)channelIn;
    activeLow = activeLowIn != 0;
    bitRes = (uint8_t)bitResIn;
    return true;
}

AstroPinMuxer::AstroPinMuxer()
    : _signal(), _chipEnable(), _channelPins{apin_none, apin_none, apin_none, apin_none},
      _channelBits(0), _channelSelect(0)
{ ; }

AstroPinMuxer::AstroPinMuxer(AstroPin signalPin, const pintype_t *channelPins, uint8_t channelBits,
                             AstroDigitalPin chipEnablePin)
    : _signal(signalPin), _chipEnable(chipEnablePin),
      _channelPins{apin_none, apin_none, apin_none, apin_none},
      _channelBits(channelBits > 4 ? 4 : channelBits), _channelSelect(0)
{
    for (uint8_t i = 0; i < _channelBits; ++i) { _channelPins[i] = channelPins ? channelPins[i] : apin_none; }
}

void AstroPinMuxer::init()
{
#ifdef ARDUINO
    for (uint8_t i = 0; i < _channelBits; ++i) {
        if (_channelPins[i] >= 0) { pinMode(_channelPins[i], OUTPUT); }
    }
#endif
    _chipEnable.init();
    deactivate();
    selectChannel(0);
}

void AstroPinMuxer::selectChannel(uint8_t channelNumber)
{
    uint8_t maxChannel = _channelBits ? (uint8_t)((1u << _channelBits) - 1u) : 0;
    _channelSelect = channelNumber > maxChannel ? maxChannel : channelNumber;
    for (uint8_t i = 0; i < _channelBits; ++i) {
        int value = (_channelSelect & (1u << i)) ? HIGH : LOW;
#ifdef ARDUINO
        if (_channelPins[i] >= 0) { ::digitalWrite(_channelPins[i], value); }
#else
        astroSetHostDigitalPin(_channelPins[i], value);
#endif
    }
}

void AstroPinMuxer::setIsActive(bool isActive)
{
    if (!_chipEnable.isValid()) { return; }
    if (isActive) { _chipEnable.activate(); }
    else { _chipEnable.deactivate(); }
}
