/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Pins
*/

#include "Astruino.h"
#include "AnalogDeviceAbstraction.h"

AstroPin *newPinObjectFromSubData(const AstroPinData *dataIn)
{
    if (!dataIn || !isValidType(dataIn->type)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && isValidType(dataIn->type), SFP(AStr_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case AstroPin::Digital:
                return new AstroDigitalPin(dataIn);
            case AstroPin::Analog:
                return new AstroAnalogPin(dataIn);
            default: break;
        }
    }

    return nullptr;
}

AstroPin::AstroPin()
    : type(Unknown), pin(apin_none), mode(Astro_PinMode_Undefined), channel(apinchnl_none)
{ ; }

AstroPin::AstroPin(int classType, pintype_t pinNumber, Astro_PinMode pinMode, int8_t pinChannel)
    : type((typeof(type))classType), pin(pinNumber), mode(pinMode),
      channel(isValidChannel(pinChannel) ? pinChannel : (isValidPin(pinNumber) && pinNumber >= apin_virtual ? pinChannelForExpanderChannel(pinNumber - apin_virtual) : apinchnl_none))
{ ; }

AstroPin::AstroPin(const AstroPinData *dataIn)
    : type((typeof(type))(dataIn->type)), pin(dataIn->pin), mode(dataIn->mode), channel(dataIn->channel)
{ ; }

AstroPin::operator AstroDigitalPin() const
{
    return (isDigitalType() || isDigital() || (!isUnknownType() && !isAnalog())) ? AstroDigitalPin(pin, mode, channel) : AstroDigitalPin();
}

AstroPin::operator AstroAnalogPin() const
{
    return (isAnalogType() || isAnalog() || (!isUnknownType() && !isDigital())) ? AstroAnalogPin(pin, mode, isOutput() ? DAC_RESOLUTION : ADC_RESOLUTION, channel) : AstroAnalogPin();
}

void AstroPin::saveToData(AstroPinData *dataOut) const
{
    dataOut->type = (int8_t)type;
    dataOut->pin = pin;
    dataOut->mode = mode;
    dataOut->channel = channel;
}

void AstroPin::init()
{
    #if !ASTRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                ASTRO_SOFT_ASSERT(!isMuxed() || channel == pinChannelForMuxerChannel(muxerChannelForPinChannel(channel)), SFP(AStr_Err_NotConfiguredProperly));

                switch (mode) {
                    case Astro_PinMode_Digital_Input:
                    case Astro_PinMode_Analog_Input:
                        pinMode(pin, INPUT);
                        break;

                    case Astro_PinMode_Digital_Input_PullUp:
                        pinMode(pin, INPUT_PULLUP);
                        break;

                    case Astro_PinMode_Digital_Input_PullDown:
                        #if HAS_INPUT_PULLDOWN
                            pinMode(pin, INPUT_PULLDOWN);
                        #else
                            pinMode(pin, INPUT);
                        #endif
                        break;

                    case Astro_PinMode_Digital_Output:
                    case Astro_PinMode_Digital_Output_PushPull:
                    case Astro_PinMode_Analog_Output:
                        pinMode(pin, OUTPUT);
                        break;

                    default:
                        break;
                }
            } else {
                #ifdef ASTRO_USE_MULTITASKING
                    ASTRO_SOFT_ASSERT(isVirtual() && pin == pinNumberForPinChannel(channel), SFP(AStr_Err_NotConfiguredProperly));
                    ASTRO_SOFT_ASSERT(channel == pinChannelForExpanderChannel(channel), SFP(AStr_Err_NotConfiguredProperly));

                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        #if HAS_INPUT_PULLDOWN
                            expander->getIoAbstraction()->pinDirection(channel % 16, isOutput() ? OUTPUT : mode == Astro_PinMode_Digital_Input_PullUp ? INPUT_PULLUP : mode == Astro_PinMode_Digital_Input_PullDown ? INPUT_PULLDOWN : INPUT);
                        #else
                            expander->getIoAbstraction()->pinDirection(channel % 16, isOutput() ? OUTPUT : mode == Astro_PinMode_Digital_Input_PullUp ? INPUT_PULLUP : INPUT);
                        #endif
                    }
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
}

void AstroPin::deinit()
{
    #if !ASTRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                pinMode(pin, INPUT);
            } else {
                #ifdef ASTRO_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        expander->getIoAbstraction()->pinDirection(channel % 16, INPUT);
                    }
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
}

bool AstroPin::enablePin(int step)
{
    #if !ASTRO_SYS_DRY_RUN_ENABLE
        if (isValid() && isValidChannel(channel)) {
            if (isMuxed()) {
                SharedPtr<AstroPinMuxer> muxer = getController() ? getController()->getPinMuxer(pin) : nullptr;
                if (muxer) {
                    switch (step) {
                        case 0: muxer->selectChannel(muxerChannelForPinChannel(channel)); muxer->activate(); return true;
                        case 1: muxer->selectChannel(muxerChannelForPinChannel(channel)); return true;
                        case 2: muxer->activate(); return true;
                        default: return false;
                    }
                }
            } else if (isExpanded() || isVirtual()) {
                #ifdef ASTRO_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    return expander && expander->trySyncChannel();
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
            }
        }
        return false;
    #else
        return isValid() && isValidChannel(channel);
    #endif
}


AstroDigitalPin::AstroDigitalPin()
    : AstroPin(Digital), activeLow(false)
{ ; }

AstroDigitalPin::AstroDigitalPin(pintype_t pinNumber, ard_pinmode_t pinMode, int8_t pinChannel)
    : AstroPin(Digital, pinNumber, pinMode != OUTPUT ? (pinMode != INPUT ? (pinMode == INPUT_PULLUP ? Astro_PinMode_Digital_Input_PullUp : Astro_PinMode_Digital_Input_PullDown)
                                                                         : Astro_PinMode_Digital_Input)
                                                     : (pinMode == OUTPUT ? Astro_PinMode_Digital_Output : Astro_PinMode_Digital_Output_PushPull), pinChannel),
      activeLow(pinMode == INPUT || pinMode == INPUT_PULLUP || pinMode == OUTPUT)
{ ; }

AstroDigitalPin::AstroDigitalPin(pintype_t pinNumber, Astro_PinMode pinMode, int8_t pinChannel)
    : AstroPin(Digital, pinNumber, pinMode, pinChannel),
      activeLow(pinMode == Astro_PinMode_Digital_Input ||
                pinMode == Astro_PinMode_Digital_Input_PullUp ||
                pinMode == Astro_PinMode_Digital_Output)
{ ; }

AstroDigitalPin::AstroDigitalPin(pintype_t pinNumber, ard_pinmode_t pinMode, bool isActiveLow, int8_t pinChannel)
    : AstroPin(Digital, pinNumber, pinMode != OUTPUT ? (isActiveLow ? Astro_PinMode_Digital_Input_PullUp : Astro_PinMode_Digital_Input_PullDown)
                                                     : (isActiveLow ? Astro_PinMode_Digital_Output : Astro_PinMode_Digital_Output_PushPull), pinChannel),
      activeLow(isActiveLow)
{ ; }

AstroDigitalPin::AstroDigitalPin(pintype_t pinNumber, Astro_PinMode pinMode, bool isActiveLow, int8_t pinChannel)
    : AstroPin(Digital, pinNumber, pinMode, pinChannel),
      activeLow(isActiveLow)
{ ; }

AstroDigitalPin::AstroDigitalPin(const AstroPinData *dataIn)
    : AstroPin(dataIn), activeLow(dataIn->dataAs.digitalPin.activeLow)
{ ; }

void AstroDigitalPin::saveToData(AstroPinData *dataOut) const
{
    AstroPin::saveToData(dataOut);

    dataOut->dataAs.digitalPin.activeLow = activeLow;
}

ard_pinstatus_t AstroDigitalPin::digitalRead()
{
    #if !ASTRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (isValidChannel(channel)) { selectAndActivatePin(); }
            if (!(isExpanded() || isVirtual())) {
                return ::digitalRead(pin);
            } else {
                #ifdef ASTRO_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        return (ard_pinstatus_t)(expander->getIoAbstraction()->readValue(channel % 16));
                    }
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
    return (ard_pinstatus_t)-1;
}

void AstroDigitalPin::digitalWrite(ard_pinstatus_t status)
{
    #if !ASTRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                if (isMuxed()) { selectPin(); }
                ::digitalWrite(pin, status);
            } else {
                #ifdef ASTRO_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        expander->getIoAbstraction()->writeValue(channel % 16, (uint8_t)status);
                    }
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
            }
            if (isValidChannel(channel)) { activatePin(); }
        }
    #endif
}


AstroAnalogPin::AstroAnalogPin()
    : AstroPin(Analog), bitRes(0)
#ifdef ESP32
      , pwmChannel(-1)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(0)
#endif
{ ; }

AstroAnalogPin::AstroAnalogPin(pintype_t pinNumber, ard_pinmode_t pinMode, uint8_t analogBitRes,
#ifdef ESP32
                               uint8_t pinPWMChannel,
#endif
#ifdef ESP_PLATFORM
                               float pinPWMFrequency,
#endif
                               int8_t pinChannel)
    : AstroPin(Analog, pinNumber, pinMode != OUTPUT ? Astro_PinMode_Analog_Input : Astro_PinMode_Analog_Output, pinChannel),
      bitRes(analogBitRes ? analogBitRes : (pinMode == OUTPUT ? DAC_RESOLUTION : ADC_RESOLUTION))
#ifdef ESP32
      , pwmChannel(pinPWMChannel)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(pinPWMFrequency)
#endif
{ ; }

AstroAnalogPin::AstroAnalogPin(pintype_t pinNumber, Astro_PinMode pinMode, uint8_t analogBitRes,
#ifdef ESP32
                               uint8_t pinPWMChannel,
#endif
#ifdef ESP_PLATFORM
                               float pinPWMFrequency,
#endif
                               int8_t pinChannel)
    : AstroPin(Analog, pinNumber, pinMode, pinChannel),
      bitRes(analogBitRes ? analogBitRes : (pinMode == Astro_PinMode_Analog_Output ? DAC_RESOLUTION : ADC_RESOLUTION))
#ifdef ESP32
      , pwmChannel(pinPWMChannel)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(pinPWMFrequency)
#endif
{ ; }

AstroAnalogPin::AstroAnalogPin(const AstroPinData *dataIn)
    : AstroPin(dataIn), bitRes(dataIn->dataAs.analogPin.bitRes)
#ifdef ESP32
      , pwmChannel(dataIn->dataAs.analogPin.pwmChannel)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(dataIn->dataAs.analogPin.pwmFrequency)
#endif
{ ; }

void AstroAnalogPin::init()
{
    #if !ASTRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                AstroPin::init();

                #ifdef ESP32
                    ledcAttachPin(pin, pwmChannel);
                    ledcSetup(pwmChannel, pwmFrequency, bitRes.bits);
                #endif
            } else {
                #ifdef ASTRO_USE_MULTITASKING
                    ASTRO_SOFT_ASSERT(isVirtual() && pin == pinNumberForPinChannel(channel), SFP(AStr_Err_NotConfiguredProperly));
                    ASTRO_SOFT_ASSERT(channel == pinChannelForExpanderChannel(channel), SFP(AStr_Err_NotConfiguredProperly));

                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        auto ioDir = isOutput() ? AnalogDirection::DIR_OUT : AnalogDirection::DIR_IN;
                        auto analogIORef = (AnalogDevice *)(expander->getIoAbstraction());
                        analogIORef->initPin(channel % 16, ioDir);

                        auto ioRefBits = analogIORef->getBitDepth(ioDir, channel % 16);
                        if (bitRes.bits != ioRefBits) {
                            bitRes = BitResolution(ioRefBits);
                        }
                    }
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
}

void AstroAnalogPin::saveToData(AstroPinData *dataOut) const
{
    AstroPin::saveToData(dataOut);

    dataOut->dataAs.analogPin.bitRes = bitRes.bits;
    #ifdef ESP32
        dataOut->dataAs.analogPin.pwmChannel = pwmChannel;
    #endif
    #ifdef ESP_PLATFORM
        dataOut->dataAs.analogPin.pwmFrequency = pwmFrequency;
    #endif
}

float AstroAnalogPin::analogRead()
{
    return bitRes.transform(analogRead_raw());
}

int AstroAnalogPin::analogRead_raw()
{
    #if !ASTRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (isValidChannel(channel)) { selectAndActivatePin(); }
            if (!(isExpanded() || isVirtual())) {
                #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                    analogReadResolution(bitRes.bits);
                #endif
                return ::analogRead(pin);
            } else {
                #ifdef ASTRO_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        auto analogIORef = (AnalogDevice *)(expander->getIoAbstraction());
                        analogIORef->getCurrentValue(channel % 16);
                    }
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
    return 0;
}

void AstroAnalogPin::analogWrite(float amount)
{
    analogWrite_raw(bitRes.inverseTransform(amount));
}

void AstroAnalogPin::analogWrite_raw(int amount)
{
    #if !ASTRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                if (isMuxed()) { selectPin(); }
                #ifdef ESP32
                    ledcWrite(pwmChannel, amount);
                #else
                    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                        analogWriteResolution(bitRes.bits);
                    #elif defined(ESP8266)
                        analogWriteRange(bitRes.maxVal);
                        analogWriteFreq(pwmFrequency);
                    #endif
                    ::analogWrite(pin, amount);
                #endif
            } else {
                #ifdef ASTRO_USE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderPosForPinChannel(channel) : expanderPosForPinNumber(pin)) : nullptr;
                    if (expander) {
                        auto analogIORef = (AnalogDevice *)(expander->getIoAbstraction());
                        analogIORef->setCurrentValue(channel % 16, amount);
                    }
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
            }
            if (isValidChannel(channel)) { activatePin(); }
        }
    #endif
}


AstroPinData::AstroPinData()
    : AstroSubData((int8_t)AstroPin::Unknown), pin(apin_none), mode(Astro_PinMode_Undefined), channel(apinchnl_none), dataAs{0}
{ ; }

void AstroPinData::toJSONObject(JsonObject &objectOut) const
{
    AstroSubData::toJSONObject(objectOut);

    if (isValidPin(pin)) { objectOut[SFP(AStr_Key_Pin)] = pin; }
    if (mode != Astro_PinMode_Undefined) { objectOut[SFP(AStr_Key_Mode)] = pinModeToString(mode); }
    if (isValidChannel(channel)) { objectOut[SFP(AStr_Key_Channel)] = channel; }

    if (mode != Astro_PinMode_Undefined) {
        if (!(mode == Astro_PinMode_Analog_Input || mode == Astro_PinMode_Analog_Output)) {
            objectOut[SFP(AStr_Key_ActiveLow)] = dataAs.digitalPin.activeLow;
        } else {
            objectOut[SFP(AStr_Key_BitRes)] = dataAs.analogPin.bitRes;
            #ifdef ESP32
                objectOut[SFP(AStr_Key_PWMChannel)] = dataAs.analogPin.pwmChannel;
            #endif
            #ifdef ESP_PLATFORM
                objectOut[SFP(AStr_Key_PWMFrequency)] = dataAs.analogPin.pwmFrequency;
            #endif
        }
    }
}

void AstroPinData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroSubData::fromJSONObject(objectIn);

    pin = objectIn[SFP(AStr_Key_Pin)] | pin;
    mode = pinModeFromString(objectIn[SFP(AStr_Key_Mode)]);
    channel = objectIn[SFP(AStr_Key_Channel)] | channel;

    if (mode != Astro_PinMode_Undefined) {
        if (!(mode == Astro_PinMode_Analog_Input || mode == Astro_PinMode_Analog_Output)) {
            type = (int8_t)AstroPin::Digital;
            dataAs.digitalPin.activeLow = objectIn[SFP(AStr_Key_ActiveLow)] | dataAs.digitalPin.activeLow;
        } else {
            type = (int8_t)AstroPin::Analog;
            dataAs.analogPin.bitRes = objectIn[SFP(AStr_Key_BitRes)] | dataAs.analogPin.bitRes;
            #ifdef ESP32
                dataAs.analogPin.pwmChannel = objectIn[SFP(AStr_Key_PWMChannel)] | dataAs.analogPin.pwmChannel;
            #endif
            #ifdef ESP_PLATFORM
                dataAs.analogPin.pwmFrequency = objectIn[SFP(AStr_Key_PWMFrequency)] | dataAs.analogPin.pwmFrequency;
            #endif
        }
    } else {
        type = (int8_t)AstroPin::Unknown;
    }
}


AstroPinMuxer::AstroPinMuxer()
    : _signal(), _chipEnable(), _channelPins{apin_none},
      _channelBits(0), _channelSelect(-1), _usingISR(false)
{
    _signal.channel = apinchnl_none; // unused
}

AstroPinMuxer::AstroPinMuxer(AstroPin signalPin,
                             pintype_t *muxChannelPins, int8_t muxChannelBits,
                             AstroDigitalPin chipEnablePin, AstroDigitalPin interruptPin)
    : _signal(signalPin), _chipEnable(chipEnablePin), _interrupt(interruptPin),
      _channelPins{ muxChannelBits > 0 ? muxChannelPins[0] : apin_none,
                    muxChannelBits > 1 ? muxChannelPins[1] : apin_none,
                    muxChannelBits > 2 ? muxChannelPins[2] : apin_none,
                    muxChannelBits > 3 ? muxChannelPins[3] : apin_none },
      _channelBits(muxChannelBits), _channelSelect(-1), _usingISR(false)
{
    _signal.channel = apinchnl_none; // unused
}

void AstroPinMuxer::init()
{
    _signal.deinit();
    _chipEnable.init();
    _chipEnable.deactivate();

    if (isValidPin(_channelPins[0])) {
        pinMode(_channelPins[0], OUTPUT);
        ::digitalWrite(_channelPins[0], LOW);

        if (isValidPin(_channelPins[1])) {
            pinMode(_channelPins[1], OUTPUT);
            ::digitalWrite(_channelPins[1], LOW);

            if (isValidPin(_channelPins[2])) {
                pinMode(_channelPins[2], OUTPUT);
                ::digitalWrite(_channelPins[2], LOW);

                if (isValidPin(_channelPins[3])) {
                    pinMode(_channelPins[3], OUTPUT);
                    ::digitalWrite(_channelPins[3], LOW);
                }
            }
        }
    }
    _channelSelect = 0;
}

bool AstroPinMuxer::tryRegisterISR(bool anyChange)
{
    #ifdef ASTRO_USE_MULTITASKING
        if (!_usingISR && _interrupt.isValid() && checkPinCanInterrupt(_interrupt.pin)) {
            taskManager.addInterrupt(&interruptImpl, _interrupt.pin, !anyChange ? (_interrupt.activeLow ? FALLING : RISING) : CHANGE);
            _usingISR = true;
        }
    #endif
    return _usingISR;
}

void AstroPinMuxer::selectChannel(uint8_t channelNumber)
{
    if (_channelSelect != channelNumber) {
        #if ASTRO_MUXERS_SHARED_ADDR_BUS
            if (getController()) { getController()->deactivatePinMuxers(); }
        #endif

        if (isValidPin(_channelPins[0])) {
            ::digitalWrite(_channelPins[0], (channelNumber >> 0) & 1 ? HIGH : LOW);

            if (isValidPin(_channelPins[1])) {
                ::digitalWrite(_channelPins[1], (channelNumber >> 1) & 1 ? HIGH : LOW);

                if (isValidPin(_channelPins[2])) {
                    ::digitalWrite(_channelPins[2], (channelNumber >> 2) & 1 ? HIGH : LOW);

                    if (isValidPin(_channelPins[3])) {
                        ::digitalWrite(_channelPins[3], (channelNumber >> 3) & 1 ? HIGH : LOW);
                    }
                }
            }
        }
        _channelSelect = channelNumber;
    }
}

void AstroPinMuxer::setIsActive(bool isActive)
{
    if (isActive) {
        _signal.init();
        _chipEnable.activate();
    } else {
        _chipEnable.deactivate();
        _signal.deinit();
    }
}

#ifdef ASTRO_USE_MULTITASKING

AstroPinExpander::AstroPinExpander()
    : _expander(0), _channelBits(0), _ioRef(nullptr), _interrupt(), _usingISR(false)
{ ; }

AstroPinExpander::AstroPinExpander(aposi_t expanderPos, uint8_t channelBits, IoAbstractionRef ioRef, AstroDigitalPin interruptPin)
    : _expander(expanderPos), _channelBits(channelBits), _ioRef(ioRef), _interrupt(interruptPin), _usingISR(false)
{ ; }

bool AstroPinExpander::tryRegisterISR(bool anyChange)
{
    #ifdef ASTRO_USE_MULTITASKING
        if (!_usingISR && _interrupt.isValid() && checkPinCanInterrupt(_interrupt.pin)) {
            taskManager.addInterrupt(&interruptImpl, _interrupt.pin, !anyChange ? (_interrupt.activeLow ? FALLING : RISING) : CHANGE);
            _usingISR = true;
        }
    #endif
    return _usingISR;
}

bool AstroPinExpander::trySyncChannel()
{
    return _ioRef->sync();
}

#endif // /ifdef ASTRO_USE_MULTITASKING
