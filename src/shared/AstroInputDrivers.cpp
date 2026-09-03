/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Input Drivers
*/

#include "AstruinoUI.h"
#ifdef ASTRO_USE_GUI
#include <DfRobotInputAbstraction.h>

static const char _matrix2x2Keys[] PROGMEM = {ASTRO_UI_2X2MATRIX_KEYS};
static const char _matrix3x4Keys[] PROGMEM = {ASTRO_UI_3X4MATRIX_KEYS};
static const char _matrix4x4Keys[] PROGMEM = {ASTRO_UI_4X4MATRIX_KEYS};


bool AstroInputDriver::areAllPinsInterruptable() const
{
    for (int i = 0; i < _pins.first; ++i) {
        if (isValidPin(_pins.second[i]) && !checkPinCanInterrupt(_pins.second[i])) {
            return false;
        }
    }
    return true;
}

bool AstroInputDriver::areMainPinsInterruptable() const
{
    return _pins.first >= 2 &&
        isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
        isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]);
}

IoAbstractionRef AstroInputDriver::getIoAbstraction() const
{
    return nullptr;
}


AstroInputRotary::AstroInputRotary(Pair<uint8_t, const pintype_t *> controlPins, Astro_EncoderSpeed encoderSpeed)
    : AstroInputDriver(controlPins), _encoderSpeed(encoderSpeed)
{
    ASTRO_SOFT_ASSERT(_pins.first >= 1 && isValidPin(_pins.second[0]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 2 && isValidPin(_pins.second[1]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 3 && isValidPin(_pins.second[2]), AStr_Err_InvalidParameter);
}

void AstroInputRotary::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    auto expander = getController() && _pins.first >= 1 && isValidPin(_pins.second[0]) && _pins.second[0] >= apin_virtual ? getController()->getPinExpander(expanderPosForPinNumber(_pins.second[0])) : nullptr;
    switches.init(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                  getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                  !getBaseUI() || getBaseUI()->isActiveLow());
 
    menuMgr.initForEncoder(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem,
                           pinChannelOrPinNumber(_pins.second[0]), pinChannelOrPinNumber(_pins.second[1]), pinChannelOrPinNumber(_pins.second[2]),
                           _encoderSpeed == Astro_EncoderSpeed_FullCycle ? FULL_CYCLE : _encoderSpeed == Astro_EncoderSpeed_HalfCycle ? HALF_CYCLE : QUARTER_CYCLE);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) { menuMgr.setBackButton(pinChannelOrPinNumber(_pins.second[3])); }
    if (_pins.first > 4 && isValidPin(_pins.second[4])) { menuMgr.setNextButton(pinChannelOrPinNumber(_pins.second[4])); }
}


AstroInputUpDownButtons::AstroInputUpDownButtons(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed)
    : AstroInputDriver(controlPins), _keySpeed(keyRepeatSpeed), _dfRobotIORef(nullptr)
{
    ASTRO_SOFT_ASSERT(_pins.first >= 1 && isValidPin(_pins.second[0]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 2 && isValidPin(_pins.second[1]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 3 && isValidPin(_pins.second[2]), AStr_Err_InvalidParameter);
}

AstroInputUpDownButtons::AstroInputUpDownButtons(bool, uint16_t keyRepeatSpeed)
    : AstroInputDriver(make_pair((uint8_t)5, (const pintype_t *)(new pintype_t[5]))), _keySpeed(keyRepeatSpeed), _dfRobotIORef(inputFromDfRobotShield())
{
    pintype_t *pins = const_cast<pintype_t *>(_pins.second);
    ASTRO_SOFT_ASSERT(pins, SFP(AStr_Err_AllocationFailure));

    if (pins) {
        pins[0] = (pintype_t)DF_KEY_UP;
        pins[1] = (pintype_t)DF_KEY_DOWN;
        pins[2] = (pintype_t)DF_KEY_SELECT;
        pins[3] = (pintype_t)DF_KEY_LEFT;
        pins[4] = (pintype_t)DF_KEY_RIGHT;
    }
    #if NUM_ANALOG_INPUTS > 0
        pinMode(A0, INPUT);
    #endif
}

AstroInputUpDownButtons::~AstroInputUpDownButtons()
{
    if (_dfRobotIORef) {
        pintype_t *pins = const_cast<pintype_t *>(_pins.second);
        if (pins) { delete [] pins; }
    }
}

void AstroInputUpDownButtons::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    if (_dfRobotIORef) {
        switches.initialise(_dfRobotIORef, !getBaseUI() || getBaseUI()->isActiveLow());
    } else {
        auto expander = getController() && _pins.first >= 1 && isValidPin(_pins.second[0]) && _pins.second[0] >= apin_virtual ? getController()->getPinExpander(expanderPosForPinNumber(_pins.second[0])) : nullptr;
        switches.init(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                      getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                      !getBaseUI() || getBaseUI()->isActiveLow());
    }

    menuMgr.initForUpDownOk(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem,
                            pinChannelOrPinNumber(_pins.second[1]), pinChannelOrPinNumber(_pins.second[0]), pinChannelOrPinNumber(_pins.second[2]),
                            _keySpeed);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) { menuMgr.setBackButton(pinChannelOrPinNumber(_pins.second[3])); }
    if (_pins.first > 4 && isValidPin(_pins.second[4])) { menuMgr.setNextButton(pinChannelOrPinNumber(_pins.second[4])); }
}


AstroInputESP32TouchKeys::AstroInputESP32TouchKeys(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed, uint16_t switchThreshold, Astro_ESP32Touch_HighRef highVoltage, Astro_ESP32Touch_LowRef lowVoltage, Astro_ESP32Touch_HighRefAtten attenuation)
    : AstroInputDriver(controlPins), _keySpeed(keyRepeatSpeed)
#ifdef ESP32
      , _esp32Touch(switchThreshold,
                    highVoltage == Astro_ESP32Touch_HighRef_V_2V4 ? TOUCH_HVOLT_2V4 : highVoltage == Astro_ESP32Touch_HighRef_V_2V5 ? TOUCH_HVOLT_2V5 :
                    highVoltage == Astro_ESP32Touch_HighRef_V_2V6 ? TOUCH_HVOLT_2V6 : highVoltage == Astro_ESP32Touch_HighRef_V_2V7 ? TOUCH_HVOLT_2V7 : 
                    highVoltage == Astro_ESP32Touch_HighRef_Max ? TOUCH_HVOLT_MAX : TOUCH_HVOLT_KEEP,
                    lowVoltage == Astro_ESP32Touch_LowRef_V_0V5 ? TOUCH_LVOLT_0V5 : lowVoltage == Astro_ESP32Touch_LowRef_V_0V6 ? TOUCH_LVOLT_0V6 :
                    lowVoltage == Astro_ESP32Touch_LowRef_V_0V7 ? TOUCH_LVOLT_0V7 : lowVoltage == Astro_ESP32Touch_LowRef_V_0V8 ? TOUCH_LVOLT_0V8 :
                    lowVoltage == Astro_ESP32Touch_LowRef_Max ? TOUCH_LVOLT_MAX : TOUCH_LVOLT_KEEP,
                    attenuation == Astro_ESP32Touch_HighRefAtten_V_0V ? TOUCH_HVOLT_ATTEN_0V : attenuation == Astro_ESP32Touch_HighRefAtten_V_0V5 ? TOUCH_HVOLT_ATTEN_0V5 :
                    attenuation == Astro_ESP32Touch_HighRefAtten_V_1V ? TOUCH_HVOLT_ATTEN_1V : attenuation == Astro_ESP32Touch_HighRefAtten_V_1V5 ? TOUCH_HVOLT_ATTEN_1V5 :
                    attenuation == Astro_ESP32Touch_HighRefAtten_Max ? TOUCH_HVOLT_ATTEN_MAX : TOUCH_HVOLT_ATTEN_KEEP)
#endif
{
    #ifndef ESP32
        ASTRO_HARD_ASSERT(false, AStr_Err_UnsupportedOperation);
    #endif
    ASTRO_SOFT_ASSERT(_pins.first >= 1 && isValidPin(_pins.second[0]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 2 && isValidPin(_pins.second[1]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 3 && isValidPin(_pins.second[2]), AStr_Err_InvalidParameter);
}

void AstroInputESP32TouchKeys::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    switches.init(getIoAbstraction() ?: internalDigitalIo(),
                  getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                  !getBaseUI() || getBaseUI()->isActiveLow());

    menuMgr.initForUpDownOk(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem,
                            pinChannelOrPinNumber(_pins.second[1]), pinChannelOrPinNumber(_pins.second[0]), pinChannelOrPinNumber(_pins.second[2]),
                            _keySpeed);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) { menuMgr.setBackButton(pinChannelOrPinNumber(_pins.second[3])); }
    if (_pins.first > 4 && isValidPin(_pins.second[4])) { menuMgr.setNextButton(pinChannelOrPinNumber(_pins.second[4])); }
    #ifdef ESP32
        _esp32Touch.ensureInterruptRegistered(); // not to be put inside allow-ISR check
    #endif
}


AstroInputJoystick::AstroInputJoystick(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, float decreaseDivisor, float jsCenterX, float jsCenterY, float jsZeroTol)
    : AstroInputDriver(controlPins), _repeatDelay(repeatDelay), _decreaseDivisor(decreaseDivisor),
      _joystickCalib{jsCenterX, jsCenterY, jsZeroTol},
      _joystickMultiIo(nullptr), _joystickIoXAxis(nullptr)
{
    ASTRO_SOFT_ASSERT(_pins.first >= 2 && isValidPin(_pins.second[1]), AStr_Err_InvalidParameter);

    if (_pins.first >= 1 && isValidPin(_pins.second[0])) {
        _joystickMultiIo = new MultiIoAbstraction(200);
        ASTRO_SOFT_ASSERT(_joystickMultiIo, SFP(AStr_Err_AllocationFailure));
        _joystickIoXAxis = new AnalogJoystickToButtons(internalAnalogIo(), _pins.second[0], _joystickCalib[0]);
        ASTRO_SOFT_ASSERT(_joystickIoXAxis, SFP(AStr_Err_AllocationFailure));

        if (_joystickMultiIo && _joystickIoXAxis) {
            multiIoAddExpander(_joystickMultiIo, _joystickIoXAxis, 5);
        }
    }
}

AstroInputJoystick::~AstroInputJoystick()
{
    if (_joystickIoXAxis) { delete _joystickIoXAxis; }
    if (_joystickMultiIo) { delete _joystickMultiIo; }
}

static void menuMgrOnMenuSelect(pinid_t /*key*/, bool held)
{
    menuMgr.onMenuSelect(held);
}

static void menuMgrPerformDirectionMoveTrue(pinid_t /*key*/, bool held)
{
    menuMgr.performDirectionMove(true);
}

static void menuMgrPerformDirectionMoveFalse(pinid_t /*key*/, bool held)
{
    menuMgr.performDirectionMove(false);
}

static void menuMgrValueChanged(int val)
{
    menuMgr.valueChanged(val);
}

void AstroInputJoystick::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    switches.init(getIoAbstraction() ?: internalDigitalIo(),
                  getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                  !getBaseUI() || getBaseUI()->isActiveLow());

    if (_pins.first >= 3 && isValidPin(_pins.second[2])) {
        switches.addSwitch(_pins.second[2], NULL);
        switches.onRelease(_pins.second[2], &menuMgrOnMenuSelect);
    }
    if (_joystickMultiIo && _joystickIoXAxis) {
        switches.addSwitch(200, &menuMgrPerformDirectionMoveTrue);
        switches.addSwitch(201, &menuMgrPerformDirectionMoveFalse);
    }
    setupAnalogJoystickEncoder(internalAnalogIo(), _pins.second[1], &menuMgrValueChanged);
    ASTRO_SOFT_ASSERT(switches.getEncoder(), SFP(AStr_Err_OperationFailure));
    if (switches.getEncoder()) {
        reinterpret_cast<JoystickSwitchInput*>(switches.getEncoder())->setTolerance(_joystickCalib[1], _joystickCalib[2]);
        reinterpret_cast<JoystickSwitchInput*>(switches.getEncoder())->setAccelerationParameters(_repeatDelay, _decreaseDivisor);
    }

    menuMgr.initWithoutInput(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem);
}

bool AstroInputJoystick::areAllPinsInterruptable() const
{
    return _pins.first >= 3 && isValidPin(_pins.second[2]) && checkPinCanInterrupt(_pins.second[2]);
}


AstroInputMatrix2x2::AstroInputMatrix2x2(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval)
    : AstroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(2, 2, _matrix2x2Keys),
      _tcMenuKeyListener(SFP(AUIStr_Keys_MatrixActions)[0], SFP(AUIStr_Keys_MatrixActions)[1], SFP(AUIStr_Keys_MatrixActions)[2], SFP(AUIStr_Keys_MatrixActions)[3])
{
    ASTRO_SOFT_ASSERT(_pins.first >= 1 && isValidPin(_pins.second[0]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 2 && isValidPin(_pins.second[1]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 3 && isValidPin(_pins.second[2]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 4 && isValidPin(_pins.second[3]), AStr_Err_InvalidParameter);

    _keyboardLayout.setRowPin(0, controlPins.second[0]);
    _keyboardLayout.setRowPin(1, controlPins.second[1]);
    _keyboardLayout.setColPin(0, controlPins.second[2]);
    _keyboardLayout.setColPin(1, controlPins.second[3]);
    _keyboard.setRepeatKeyMillis(repeatDelay, repeatInterval);
}

void AstroInputMatrix2x2::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    auto expander = getController() && _pins.first >= 1 && isValidPin(_pins.second[0]) && _pins.second[0] >= apin_virtual ? getController()->getPinExpander(expanderPosForPinNumber(_pins.second[0])) : nullptr;
    _keyboard.initialise(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                         &_keyboardLayout, &_tcMenuKeyListener,
                         (!getBaseUI() || getBaseUI()->allowingISR()) && areRowPinsInterruptable());
 
    menuMgr.initWithoutInput(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem);
}

bool AstroInputMatrix2x2::areRowPinsInterruptable() const
{
    return AstroInputDriver::areMainPinsInterruptable();
}


AstroInputMatrix3x4::AstroInputMatrix3x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Astro_EncoderSpeed encoderSpeed)
    : AstroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(4, 3, _matrix3x4Keys),
      _tcMenuKeyListener(SFP(AUIStr_Keys_MatrixActions)[0], SFP(AUIStr_Keys_MatrixActions)[1], SFP(AUIStr_Keys_MatrixActions)[2], SFP(AUIStr_Keys_MatrixActions)[3]),
      _rotaryEncoder(nullptr)
{
    ASTRO_SOFT_ASSERT(_pins.first >= 1 && isValidPin(_pins.second[0]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 2 && isValidPin(_pins.second[1]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 3 && isValidPin(_pins.second[2]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 4 && isValidPin(_pins.second[3]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 5 && isValidPin(_pins.second[4]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 6 && isValidPin(_pins.second[5]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 7 && isValidPin(_pins.second[6]), AStr_Err_InvalidParameter);

    _keyboardLayout.setRowPin(0, controlPins.second[0]);
    _keyboardLayout.setRowPin(1, controlPins.second[1]);
    _keyboardLayout.setRowPin(2, controlPins.second[2]);
    _keyboardLayout.setRowPin(3, controlPins.second[3]);
    _keyboardLayout.setColPin(0, controlPins.second[4]);
    _keyboardLayout.setColPin(1, controlPins.second[5]);
    _keyboardLayout.setColPin(2, controlPins.second[6]);
    _keyboard.setRepeatKeyMillis(repeatDelay, repeatInterval);

    if (isValidPin(controlPins.second[7])) {
        _rotaryEncoder = new AstroInputRotary(make_pair((uint8_t)(controlPins.first - 7), &controlPins.second[7]), encoderSpeed);
    }
}

AstroInputMatrix3x4::~AstroInputMatrix3x4()
{
    if (_rotaryEncoder) { delete _rotaryEncoder; }
}

void AstroInputMatrix3x4::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    auto expander = getController() && _pins.first >= 1 && isValidPin(_pins.second[0]) && _pins.second[0] >= apin_virtual ? getController()->getPinExpander(expanderPosForPinNumber(_pins.second[0])) : nullptr;
    _keyboard.initialise(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                         &_keyboardLayout, &_tcMenuKeyListener,
                         (!getBaseUI() || getBaseUI()->allowingISR()) && areRowPinsInterruptable());

    if (_rotaryEncoder) { _rotaryEncoder->begin(displayDriver, initialItem); }
    else { menuMgr.initWithoutInput(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem); }
}

bool AstroInputMatrix3x4::areRowPinsInterruptable() const
{
    return _pins.first >= 4 &&
           isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
           isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]) &&
           isValidPin(_pins.second[2]) && checkPinCanInterrupt(_pins.second[2]) &&
           isValidPin(_pins.second[3]) && checkPinCanInterrupt(_pins.second[3]);
}

bool AstroInputMatrix3x4::areMainPinsInterruptable() const
{
    return areRowPinsInterruptable() && (!_rotaryEncoder || _rotaryEncoder->areMainPinsInterruptable());
}


AstroInputMatrix4x4::AstroInputMatrix4x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Astro_EncoderSpeed encoderSpeed)
    : AstroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(4, 4, _matrix4x4Keys),
      _tcMenuKeyListener(SFP(AUIStr_Keys_MatrixActions)[0], SFP(AUIStr_Keys_MatrixActions)[1], SFP(AUIStr_Keys_MatrixActions)[2], SFP(AUIStr_Keys_MatrixActions)[3]),
      _rotaryEncoder(nullptr)
{
    ASTRO_SOFT_ASSERT(_pins.first >= 1 && isValidPin(_pins.second[0]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 2 && isValidPin(_pins.second[1]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 3 && isValidPin(_pins.second[2]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 4 && isValidPin(_pins.second[3]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 5 && isValidPin(_pins.second[4]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 6 && isValidPin(_pins.second[5]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 7 && isValidPin(_pins.second[6]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 8 && isValidPin(_pins.second[7]), AStr_Err_InvalidParameter);

    _keyboardLayout.setRowPin(0, controlPins.second[0]);
    _keyboardLayout.setRowPin(1, controlPins.second[1]);
    _keyboardLayout.setRowPin(2, controlPins.second[2]);
    _keyboardLayout.setRowPin(3, controlPins.second[3]);
    _keyboardLayout.setColPin(0, controlPins.second[4]);
    _keyboardLayout.setColPin(1, controlPins.second[5]);
    _keyboardLayout.setColPin(2, controlPins.second[6]);
    _keyboardLayout.setColPin(3, controlPins.second[7]);
    _keyboard.setRepeatKeyMillis(repeatDelay, repeatInterval);

    if (isValidPin(controlPins.second[8])) {
        _rotaryEncoder = new AstroInputRotary(make_pair((uint8_t)(controlPins.first - 8), &controlPins.second[8]), encoderSpeed);
    }
}

AstroInputMatrix4x4::~AstroInputMatrix4x4()
{
    if (_rotaryEncoder) { delete _rotaryEncoder; }
}

void AstroInputMatrix4x4::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    auto expander = getController() && _pins.first >= 1 && isValidPin(_pins.second[0]) && _pins.second[0] >= apin_virtual ? getController()->getPinExpander(expanderPosForPinNumber(_pins.second[0])) : nullptr;
    _keyboard.initialise(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                         &_keyboardLayout, &_tcMenuKeyListener,
                         (!getBaseUI() || getBaseUI()->allowingISR()) && areRowPinsInterruptable());
 
    if (_rotaryEncoder) { _rotaryEncoder->begin(displayDriver, initialItem); }
    else { menuMgr.initWithoutInput(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem); }
}

bool AstroInputMatrix4x4::areRowPinsInterruptable() const
{
    return _pins.first >= 4 &&
           isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
           isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]) &&
           isValidPin(_pins.second[2]) && checkPinCanInterrupt(_pins.second[2]) &&
           isValidPin(_pins.second[3]) && checkPinCanInterrupt(_pins.second[3]);
}

bool AstroInputMatrix4x4::areMainPinsInterruptable() const
{
    return areRowPinsInterruptable() && (!_rotaryEncoder || _rotaryEncoder->areMainPinsInterruptable());
}


AstroInputResistiveTouch::AstroInputResistiveTouch(Pair<uint8_t, const pintype_t *> controlPins, AstroDisplayDriver *displayDriver, Astro_DisplayRotation displayRotation, Astro_TouchscreenOrientation touchOrient)
    : AstroInputDriver(controlPins),
      _touchInterrogator(controlPins.second[0], controlPins.second[1], controlPins.second[2], controlPins.second[3]),
      _touchOrientation(
         /*swap*/ (touchOrient == Astro_TouchscreenOrientation_Same && (displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_R3)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus1 && (displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_R0)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus2 && (displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_R1)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus3 && (displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_R2))
                  || (touchOrient == Astro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertXY_SwapXY || touchOrient == Astro_TouchscreenOrientation_SwapXY),
         /*invX*/ (touchOrient == Astro_TouchscreenOrientation_Same && (displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_HorzMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus1 && (displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_HorzMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus2 && (displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_HorzMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus3 && (displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_HorzMirror))
                  || (touchOrient == Astro_TouchscreenOrientation_InvertX || touchOrient == Astro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertXY || touchOrient == Astro_TouchscreenOrientation_InvertXY_SwapXY),
         /*invY*/ (touchOrient == Astro_TouchscreenOrientation_Same && (displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_VertMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus1 && (displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_VertMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus2 && (displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_VertMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus3 && (displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_VertMirror))
                  || (touchOrient == Astro_TouchscreenOrientation_InvertY || touchOrient == Astro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertXY || touchOrient == Astro_TouchscreenOrientation_InvertXY_SwapXY)
      ),
      _touchScreen(&_touchInterrogator, displayDriver->getGraphicsRenderer(), _touchOrientation)
{
    ASTRO_SOFT_ASSERT(_pins.first >= 1 && isValidPin(_pins.second[0]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 2 && isValidPin(_pins.second[1]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 3 && isValidPin(_pins.second[2]), AStr_Err_InvalidParameter);
    ASTRO_SOFT_ASSERT(_pins.first >= 4 && isValidPin(_pins.second[3]), AStr_Err_InvalidParameter);
}

void AstroInputResistiveTouch::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    _touchScreen.start();
    menuMgr.initWithoutInput(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem);
}


AstroInputTouchscreen::AstroInputTouchscreen(Pair<uint8_t, const pintype_t *> controlPins, AstroDisplayDriver *displayDriver, Astro_DisplayRotation displayRotation, Astro_TouchscreenOrientation touchOrient)
    : AstroInputDriver(controlPins),
      #ifdef ASTRO_UI_ENABLE_XPT2046TS
          _touchScreen(controlPins.second[0], (!getBaseUI() || getBaseUI()->allowingISR()) && isValidPin(controlPins.second[1]) ? controlPins.second[1] : 0xff),
      #else
          _touchScreen(),
      #endif
      #ifdef ASTRO_UI_ENABLE_BSP_TOUCH
          _touchInterrogator(),
      #else
          _touchInterrogator(_touchScreen),
      #endif
      _touchOrientation(
         /*swap*/ (touchOrient == Astro_TouchscreenOrientation_Same && (displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_R3)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus1 && (displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_R0)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus2 && (displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_R1)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus3 && (displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_R2))
                  || (touchOrient == Astro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertXY_SwapXY || touchOrient == Astro_TouchscreenOrientation_SwapXY),
         /*invX*/ (touchOrient == Astro_TouchscreenOrientation_Same && (displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_HorzMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus1 && (displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_HorzMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus2 && (displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_HorzMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus3 && (displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_HorzMirror))
                  || (touchOrient == Astro_TouchscreenOrientation_InvertX || touchOrient == Astro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertXY || touchOrient == Astro_TouchscreenOrientation_InvertXY_SwapXY),
         /*invY*/ (touchOrient == Astro_TouchscreenOrientation_Same && (displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_VertMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus1 && (displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_VertMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus2 && (displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_VertMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus3 && (displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_VertMirror))
                  || (touchOrient == Astro_TouchscreenOrientation_InvertY || touchOrient == Astro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertXY || touchOrient == Astro_TouchscreenOrientation_InvertXY_SwapXY)
      )
{
    #ifdef ASTRO_UI_ENABLE_XPT2046TS
        ASTRO_SOFT_ASSERT(_pins.first >= 1 && isValidPin(_pins.second[0]), AStr_Err_InvalidParameter);
    #endif
}

bool AstroInputTouchscreen::areMainPinsInterruptable() const
{
    #ifdef ASTRO_UI_ENABLE_XPT2046TS
        return _pins.first >= 2 && isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]);
    #else
        return false;
    #endif
}

void AstroInputTouchscreen::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    #ifndef ASTRO_UI_ENABLE_XPT2046TS
        _touchInterrogator.init(displayDriver ? displayDriver->getScreenSize(false).first : TFT_GFX_WIDTH,
                                displayDriver ? displayDriver->getScreenSize(false).second : TFT_GFX_HEIGHT);
    #else
        _touchInterrogator.init(getBaseUI() ? getBaseUI()->getControlSetup().ctrlCfgAs.touchscreen.spiClass : nullptr);
    #endif
    menuMgr.initWithoutInput(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem);
    #ifdef ASTRO_UI_ENABLE_XPT2046TS
        _touchScreen.setRotation(getBaseUI()  ? (uint8_t)getBaseUI()->getDisplaySetup().getDisplayRotation() : 0);
    #endif
}


AstroInputTFTTouch::AstroInputTFTTouch(Pair<uint8_t, const pintype_t *> controlPins, AstroDisplayTFTeSPI *displayDriver, Astro_DisplayRotation displayRotation, Astro_TouchscreenOrientation touchOrient, bool useRawTouch)
    : AstroInputDriver(controlPins),
      _touchInterrogator(&displayDriver->getGfx(), useRawTouch),
      _touchOrientation(
         /*swap*/ (touchOrient == Astro_TouchscreenOrientation_Same && (displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_R3)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus1 && (displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_R0)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus2 && (displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_R1)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus3 && (displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_R2))
                  || (touchOrient == Astro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertXY_SwapXY || touchOrient == Astro_TouchscreenOrientation_SwapXY),
         /*invX*/ (touchOrient == Astro_TouchscreenOrientation_Same && (displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_HorzMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus1 && (displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_HorzMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus2 && (displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_HorzMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus3 && (displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_HorzMirror))
                  || (touchOrient == Astro_TouchscreenOrientation_InvertX || touchOrient == Astro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertXY || touchOrient == Astro_TouchscreenOrientation_InvertXY_SwapXY),
         /*invY*/ (touchOrient == Astro_TouchscreenOrientation_Same && (displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_VertMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus1 && (displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_R3 || displayRotation == Astro_DisplayRotation_VertMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus2 && (displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_R0 || displayRotation == Astro_DisplayRotation_VertMirror)) ||
                  (touchOrient == Astro_TouchscreenOrientation_Plus3 && (displayRotation == Astro_DisplayRotation_R2 || displayRotation == Astro_DisplayRotation_R1 || displayRotation == Astro_DisplayRotation_VertMirror))
                  || (touchOrient == Astro_TouchscreenOrientation_InvertY || touchOrient == Astro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Astro_TouchscreenOrientation_InvertXY || touchOrient == Astro_TouchscreenOrientation_InvertXY_SwapXY)
      ),
      _touchScreen(&_touchInterrogator, displayDriver->getGraphicsRenderer(), _touchOrientation)
{
    ASTRO_SOFT_ASSERT(_pins.first >= 1 && isValidPin(_pins.second[0]), AStr_Err_InvalidParameter);
}

bool AstroInputTFTTouch::areMainPinsInterruptable() const
{
    return _pins.first >= 2 && isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]);
}

void AstroInputTFTTouch::begin(AstroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    _touchInterrogator.init(displayDriver ? displayDriver->getScreenSize(false).first : TFT_GFX_WIDTH,
                            displayDriver ? displayDriver->getScreenSize(false).second : TFT_GFX_HEIGHT);
    _touchScreen.start();
    menuMgr.initWithoutInput(displayDriver ? displayDriver->getBaseRenderer() : nullptr, initialItem);
}

#endif
