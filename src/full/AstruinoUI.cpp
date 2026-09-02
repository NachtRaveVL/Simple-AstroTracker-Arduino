/*  Astruino: Full/RW UI.
    Copyright (C) 2026 NachtRaveVL
*/

#include "AstruinoUI.h"
#ifdef ASTRO_USE_GUI

AstruinoFullUI::AstruinoFullUI(String deviceUUID, UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts, bool enableBufferedVRAM)
    : AstruinoBaseUI(deviceUUID, uiControlSetup, uiDisplaySetup, isActiveLowIO, allowInterruptableIO, enableTcUnicodeFonts, enableBufferedVRAM)
{
    auto controller = getController();
    ASTRO_HARD_ASSERT(controller, SFP(AStr_Err_InitializationFailure));

    if (controller) {
        // Input driver setup
        auto ctrlInMode = controller->getControlInputMode();
        auto ctrlInPins = controller->getControlInputPins();
        switch (ctrlInMode) {
            case Astro_ControlInputMode_RotaryEncoderOk:
            case Astro_ControlInputMode_RotaryEncoderOkLR: {
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Encoder, SFP(AStr_Err_InvalidParameter));
                _input = new AstroInputRotary(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.encoder.encoderSpeed);
            } break;

            case Astro_ControlInputMode_UpDownButtonsOk:
            case Astro_ControlInputMode_UpDownButtonsOkLR: {
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Buttons, SFP(AStr_Err_InvalidParameter));
                if (!_uiCtrlSetup.ctrlCfgAs.buttons.isDFRobotShield) {
                    _input = new AstroInputUpDownButtons(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.buttons.repeatSpeed);
                } else {
                    _input = new AstroInputUpDownButtons(true, _uiCtrlSetup.ctrlCfgAs.buttons.repeatSpeed);
                }
            } break;

            case Astro_ControlInputMode_UpDownESP32TouchOk:
            case Astro_ControlInputMode_UpDownESP32TouchOkLR: {
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::ESP32Touch, SFP(AStr_Err_InvalidParameter));
                _input = new AstroInputESP32TouchKeys(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.espTouch.repeatSpeed, _uiCtrlSetup.ctrlCfgAs.espTouch.switchThreshold,
                                                      _uiCtrlSetup.ctrlCfgAs.espTouch.highVoltage, _uiCtrlSetup.ctrlCfgAs.espTouch.lowVoltage, _uiCtrlSetup.ctrlCfgAs.espTouch.attenuation);
            } break;

            case Astro_ControlInputMode_AnalogJoystickOk: {
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Joystick, SFP(AStr_Err_InvalidParameter));
                if (_uiData) {
                    _input = new AstroInputJoystick(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.joystick.repeatDelay, _uiCtrlSetup.ctrlCfgAs.joystick.decreaseDivisor,
                                                    _uiData->joystickCalib[0], _uiData->joystickCalib[1], _uiData->joystickCalib[2]);
                } else {
                    _input = new AstroInputJoystick(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.joystick.repeatDelay, _uiCtrlSetup.ctrlCfgAs.joystick.decreaseDivisor);
                }
            } break;

            case Astro_ControlInputMode_Matrix2x2UpDownButtonsOkL: {
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(AStr_Err_InvalidParameter));
                _input = new AstroInputMatrix2x2(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.matrix.repeatDelay, _uiCtrlSetup.ctrlCfgAs.matrix.repeatInterval);
            } break;

            case Astro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk:
            case Astro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR: {
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(AStr_Err_InvalidParameter));
                _input = new AstroInputMatrix3x4(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.matrix.repeatDelay, _uiCtrlSetup.ctrlCfgAs.matrix.repeatInterval,
                                                 _uiCtrlSetup.ctrlCfgAs.matrix.encoderSpeed);
            } break;

            case Astro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk:
            case Astro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR: {
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(AStr_Err_InvalidParameter));
                _input = new AstroInputMatrix4x4(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.matrix.repeatDelay, _uiCtrlSetup.ctrlCfgAs.matrix.repeatInterval,
                                                 _uiCtrlSetup.ctrlCfgAs.matrix.encoderSpeed);
            } break;

            default: break;
        }
        ASTRO_SOFT_ASSERT(!(ctrlInMode >= Astro_ControlInputMode_RotaryEncoderOk && ctrlInMode != Astro_ControlInputMode_ResistiveTouch && ctrlInMode <= Astro_ControlInputMode_TouchScreen) || _input, SFP(AStr_Err_AllocationFailure));

        // Display driver setup
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        // LiquidCrystalIO supports only i2c, /w LCD setup
        ASTRO_SOFT_ASSERT(!(dispOutMode >= Astro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Astro_DisplayOutputMode_LCD20x4_RS) || displaySetup.cfgType == DeviceSetup::I2CSetup, SFP(AStr_Err_InvalidParameter));
        ASTRO_SOFT_ASSERT(!(dispOutMode >= Astro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Astro_DisplayOutputMode_LCD20x4_RS) || _uiDispSetup.dispCfgType == UIDisplaySetup::LCD, SFP(AStr_Err_InvalidParameter));
        // U8g2 supports either i2c or SPI, /w Pixel setup
        ASTRO_SOFT_ASSERT(!(dispOutMode >= Astro_DisplayOutputMode_SSD1305 && dispOutMode <= Astro_DisplayOutputMode_CustomOLED) || (displaySetup.cfgType == DeviceSetup::I2CSetup || displaySetup.cfgType == DeviceSetup::SPISetup), SFP(AStr_Err_InvalidParameter));
        ASTRO_SOFT_ASSERT(!(dispOutMode >= Astro_DisplayOutputMode_SSD1607 && dispOutMode <= Astro_DisplayOutputMode_IL3820_V2) || displaySetup.cfgType == DeviceSetup::SPISetup, SFP(AStr_Err_InvalidParameter));
        ASTRO_SOFT_ASSERT(!(dispOutMode >= Astro_DisplayOutputMode_SSD1305 && dispOutMode <= Astro_DisplayOutputMode_IL3820_V2) || _uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(AStr_Err_InvalidParameter));
        // AdafruitGFX supports only SPI, /w Pixel setup
        ASTRO_SOFT_ASSERT(!(dispOutMode >= Astro_DisplayOutputMode_ST7735 && dispOutMode <= Astro_DisplayOutputMode_ILI9341) || displaySetup.cfgType == DeviceSetup::SPISetup, SFP(AStr_Err_InvalidParameter));
        ASTRO_SOFT_ASSERT(!(dispOutMode >= Astro_DisplayOutputMode_ST7735 && dispOutMode <= Astro_DisplayOutputMode_ILI9341) || _uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(AStr_Err_InvalidParameter));
        // TFT_eSPI supports only SPI, /w TFT setup
        ASTRO_SOFT_ASSERT(dispOutMode != Astro_DisplayOutputMode_TFT || displaySetup.cfgType == DeviceSetup::SPISetup, SFP(AStr_Err_InvalidParameter));
        ASTRO_SOFT_ASSERT(dispOutMode != Astro_DisplayOutputMode_TFT || _uiDispSetup.dispCfgType == UIDisplaySetup::TFT, SFP(AStr_Err_InvalidParameter));

        switch (dispOutMode) {
            // LiquidCrystalIO
            case Astro_DisplayOutputMode_LCD16x2_EN:
            case Astro_DisplayOutputMode_LCD16x2_RS:
            case Astro_DisplayOutputMode_LCD20x4_EN:
            case Astro_DisplayOutputMode_LCD20x4_RS: {
                if (!_uiDispSetup.dispCfgAs.lcd.isDFRobotShield) {
                    _display = new AstroDisplayLiquidCrystal(dispOutMode, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.ledMode);
                } else {
                    _display = new AstroDisplayLiquidCrystal(true, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.ledMode);
                }
            } break;

            // U8g2OLED
            case Astro_DisplayOutputMode_SSD1305: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305Wire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE1) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305Wire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI1) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_InvalidParameter));
                }
            } break;
            case Astro_DisplayOutputMode_SSD1305_x32Ada: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305x32AdaWire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE1) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305x32AdaWire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305x32AdaSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI1) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305x32AdaSPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_InvalidParameter));
                }
            } break;
            case Astro_DisplayOutputMode_SSD1305_x64Ada: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305x64AdaWire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE1) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305x64AdaWire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305x64AdaSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI1) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1305x64AdaSPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_InvalidParameter));
                }
            } break;
            case Astro_DisplayOutputMode_SSD1306: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1306Wire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE1) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1306Wire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1306SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI1) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1306SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_InvalidParameter));
                }
            } break;
            case Astro_DisplayOutputMode_SH1106: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE) {
                    _display = AstroDisplayU8g2OLED::allocateSH1106Wire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == ASTRO_USE_WIRE1) {
                    _display = AstroDisplayU8g2OLED::allocateSH1106Wire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI) {
                    _display = AstroDisplayU8g2OLED::allocateSH1106SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI1) {
                    _display = AstroDisplayU8g2OLED::allocateSH1106SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_InvalidParameter));
                }
            } break;
            case Astro_DisplayOutputMode_CustomOLED: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                    _display = AstroDisplayU8g2OLED::allocateCustomOLEDI2C(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                    _display = AstroDisplayU8g2OLED::allocateCustomOLEDSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_InvalidParameter));
                }
            } break;
            case Astro_DisplayOutputMode_SSD1607: {
                if (displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1607SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI1) {
                    _display = AstroDisplayU8g2OLED::allocateSSD1607SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_InvalidParameter));
                }
            } break;
            case Astro_DisplayOutputMode_IL3820: {
                if (displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI) {
                    _display = AstroDisplayU8g2OLED::allocateIL3820SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI1) {
                    _display = AstroDisplayU8g2OLED::allocateIL3820SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_InvalidParameter));
                }
            } break;
            case Astro_DisplayOutputMode_IL3820_V2: {
                if (displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI) {
                    _display = AstroDisplayU8g2OLED::allocateIL3820V2SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI1) {
                    _display = AstroDisplayU8g2OLED::allocateIL3820V2SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_InvalidParameter));
                }
            } break;

            // AdafruitGFX
            case Astro_DisplayOutputMode_ST7735: {
                _display = new AstroDisplayAdafruitGFX<Adafruit_ST7735>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.st77Kind, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Astro_DisplayOutputMode_ST7789: {
                _display = new AstroDisplayAdafruitGFX<Adafruit_ST7789>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.st77Kind, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Astro_DisplayOutputMode_ILI9341: {
                _display = new AstroDisplayAdafruitGFX<Adafruit_ILI9341>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;

            // TFT_eSPI
            case Astro_DisplayOutputMode_TFT: {
                ASTRO_SOFT_ASSERT(!(bool)ASTRO_USE_SPI || displaySetup.cfgAs.spi.spi == ASTRO_USE_SPI, SFP(AStr_Err_InvalidParameter));
                #ifdef TFT_CS
                    ASTRO_SOFT_ASSERT(displaySetup.cfgAs.spi.cs == TFT_CS, SFP(AStr_Err_NotConfiguredProperly));
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
                _display = new AstroDisplayTFTeSPI(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.tft.rotation, _uiDispSetup.dispCfgAs.tft.st77Kind);
            } break;

            default: break;
        }
        ASTRO_SOFT_ASSERT(!(dispOutMode >= Astro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Astro_DisplayOutputMode_TFT) || _display, SFP(AStr_Err_AllocationFailure));

        // Late input driver setup
        switch (ctrlInMode) {
            case Astro_ControlInputMode_ResistiveTouch: {
                ASTRO_SOFT_ASSERT(_display, SFP(AStr_Err_NotYetInitialized));
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Touchscreen, SFP(AStr_Err_InvalidParameter));
                _input = new AstroInputResistiveTouch(ctrlInPins, _display, _uiDispSetup.getDisplayRotation(), _uiCtrlSetup.ctrlCfgAs.touchscreen.orient);
            } break;

            case Astro_ControlInputMode_TouchScreen: {
                ASTRO_SOFT_ASSERT(_display, SFP(AStr_Err_NotYetInitialized));
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Touchscreen, SFP(AStr_Err_InvalidParameter));
                #ifdef ASTRO_UI_ENABLE_XPT2046TS
                    ASTRO_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && isValidPin(ctrlInPins.second[0]), SFP(AStr_Err_InvalidPinOrType));
                #endif
                _input = new AstroInputTouchscreen(ctrlInPins, _display, _uiDispSetup.getDisplayRotation(), _uiCtrlSetup.ctrlCfgAs.touchscreen.orient);
            } break;

            // TFT_eSPI
            case Astro_ControlInputMode_TFTTouch: {
                ASTRO_SOFT_ASSERT(_display, SFP(AStr_Err_NotYetInitialized));
                ASTRO_SOFT_ASSERT(dispOutMode == Astro_DisplayOutputMode_TFT, SFP(AStr_Err_InvalidParameter));
                ASTRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Touchscreen, SFP(AStr_Err_InvalidParameter));
                ASTRO_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && isValidPin(ctrlInPins.second[0]), SFP(AStr_Err_InvalidPinOrType));
                #ifdef TOUCH_CS
                    ASTRO_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && ctrlInPins.second[0] == TOUCH_CS, SFP(AStr_Err_NotConfiguredProperly));
                #else
                    ASTRO_HARD_ASSERT(false, SFP(AStr_Err_NotConfiguredProperly));
                #endif
                _input = new AstroInputTFTTouch(ctrlInPins, (AstroDisplayTFTeSPI *)_display, _uiDispSetup.getDisplayRotation(), _uiCtrlSetup.ctrlCfgAs.touchscreen.orient, ASTRO_UI_TFTTOUCH_USES_RAW);
            } break;

            default: break;
        }
        ASTRO_SOFT_ASSERT(!(ctrlInMode == Astro_ControlInputMode_ResistiveTouch || ctrlInMode == Astro_ControlInputMode_TFTTouch) || _input, SFP(AStr_Err_AllocationFailure));
    }
}

AstruinoFullUI::~AstruinoFullUI()
{ ; }

void AstruinoFullUI::addRemote(Astro_RemoteControl rcType, UARTDeviceSetup rcSetup, uint16_t rcServerPort)
{
    AstroRemoteControl *remoteControl = nullptr;

    switch (rcType) {
        case Astro_RemoteControl_Serial: {
            remoteControl = new AstroRemoteSerialControl(rcSetup);
            ASTRO_SOFT_ASSERT(remoteControl, SFP(AStr_Err_AllocationFailure));
        } break;

        case Astro_RemoteControl_Simhub: {
            remoteControl = new AstroRemoteSimhubControl(rcSetup, ASTRO_UI_SIMHUB_STATUS_MENU_ID);
            ASTRO_SOFT_ASSERT(remoteControl, SFP(AStr_Err_AllocationFailure));
        } break;

        case Astro_RemoteControl_WiFi: {
            #ifdef ASTRO_USE_WIFI
                remoteControl = new AstroRemoteWiFiControl(rcServerPort);
                ASTRO_SOFT_ASSERT(remoteControl, SFP(AStr_Err_AllocationFailure));
            #endif
        } break;

        case Astro_RemoteControl_Ethernet: {
            #ifdef ASTRO_USE_ETHERNET
                remoteControl = new AstroRemoteEthernetControl(rcServerPort);
                ASTRO_SOFT_ASSERT(remoteControl, SFP(AStr_Err_AllocationFailure));
            #endif
        } break;

        default: break;
    }

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else {
        if (remoteControl) { delete remoteControl; }
    }
}

bool AstruinoFullUI::isFullUI()
{
    return true;
}

#endif
