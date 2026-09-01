/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Display Drivers
*/

#include "AstruinoUI.h"
#ifdef ASTRO_USE_GUI
#include "BaseRenderers.h"
#include "graphics/BaseGraphicalRenderer.h"
#include "IoAbstractionWire.h"
#include "DfRobotInputAbstraction.h"

void AstroDisplayDriver::setupRendering(Astro_DisplayTheme displayTheme, Astro_TitleMode titleMode, const void *itemFont, const void *titleFont, bool analogSlider, bool editingIcons, bool tcUnicodeFonts)
{
    auto graphicsRenderer = getGraphicsRenderer();
    if (graphicsRenderer) {
        if (getController()->getControlInputMode() >= Astro_ControlInputMode_ResistiveTouch &&
            getController()->getControlInputMode() < Astro_ControlInputMode_RemoteControl) {
            graphicsRenderer->setHasTouchInterface(true);
        }
        graphicsRenderer->setTitleMode((BaseGraphicalRenderer::TitleMode)titleMode);
        graphicsRenderer->setUseSliderForAnalog(analogSlider);
        if (tcUnicodeFonts) { graphicsRenderer->enableTcUnicode(); }

        if (_displayTheme != displayTheme) {
            switch ((_displayTheme = displayTheme)) {
                case Astro_DisplayTheme_CoolBlue_ML:
                    installCoolBlueModernTheme(*graphicsRenderer, MenuFontDef(itemFont, ASTRO_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, ASTRO_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Astro_DisplayTheme_CoolBlue_SM:
                    installCoolBlueTraditionalTheme(*graphicsRenderer, MenuFontDef(itemFont, ASTRO_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, ASTRO_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Astro_DisplayTheme_DarkMode_ML:
                    installDarkModeModernTheme(*graphicsRenderer, MenuFontDef(itemFont, ASTRO_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, ASTRO_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Astro_DisplayTheme_DarkMode_SM:
                    installDarkModeTraditionalTheme(*graphicsRenderer, MenuFontDef(itemFont, ASTRO_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, ASTRO_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Astro_DisplayTheme_MonoOLED:
                    installMonoBorderedTheme(*graphicsRenderer, MenuFontDef(itemFont, ASTRO_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, ASTRO_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
                case Astro_DisplayTheme_MonoOLED_Inv:
                    installMonoInverseTitleTheme(*graphicsRenderer, MenuFontDef(itemFont, ASTRO_UI_MENU_ITEM_MAG_LEVEL), MenuFontDef(titleFont, ASTRO_UI_MENU_TITLE_MAG_LEVEL), editingIcons);
                    break;
            }
        }
    }
}


AstroDisplayLiquidCrystal::AstroDisplayLiquidCrystal(Astro_DisplayOutputMode displayMode, I2CDeviceSetup displaySetup, Astro_BacklightMode ledMode)
    : AstroDisplayDriver(Astro_DisplayRotation_Undefined, displayMode < Astro_DisplayOutputMode_LCD20x4_EN ? 16 : 20, displayMode < Astro_DisplayOutputMode_LCD20x4_EN ? 2 : 4),
      _lcd(displayMode == Astro_DisplayOutputMode_LCD16x2_EN || displayMode == Astro_DisplayOutputMode_LCD20x4_EN ? 2 : 0, 1,
           displayMode == Astro_DisplayOutputMode_LCD16x2_EN || displayMode == Astro_DisplayOutputMode_LCD20x4_EN ? 0 : 2, 4, 5, 6, 7,
           ledMode == Astro_BacklightMode_Normal ? LiquidCrystal::BACKLIGHT_NORMAL : ledMode == Astro_BacklightMode_Inverted ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_PWM,
           ioFrom8574(ASTRO_UI_I2C_LCD_BASEADDR | displaySetup.address, 0xff, displaySetup.wire, false)),
      _renderer(_lcd, _screenSize[0], _screenSize[1], AstroDisplayDriver::getSystemName())
{
    _lcd.configureBacklightPin(3);
    _renderer.setTitleRequired(false);
}

AstroDisplayLiquidCrystal::AstroDisplayLiquidCrystal(bool, I2CDeviceSetup displaySetup, Astro_BacklightMode ledMode)
    : AstroDisplayDriver(Astro_DisplayRotation_Undefined, 16, 2),
      _lcd(8, 9, 4, 5, 6, 7,
           ledMode == Astro_BacklightMode_Normal ? LiquidCrystal::BACKLIGHT_NORMAL : ledMode == Astro_BacklightMode_Inverted ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_PWM,
           ioFrom8574(ASTRO_UI_I2C_LCD_BASEADDR | displaySetup.address, 0xff, displaySetup.wire, false)),
      _renderer(_lcd, _screenSize[0], _screenSize[1], AstroDisplayDriver::getSystemName())
{
    _lcd.configureBacklightPin(10);
    _renderer.setTitleRequired(false);
}

void AstroDisplayLiquidCrystal::initBaseUIFromDefaults()
{
    getBaseUI()->init(ASTRO_UI_UPDATE_SPEED, Astro_DisplayTheme_Undefined, _screenSize[1] >= 4 ? Astro_TitleMode_Always : Astro_TitleMode_None);
}

void AstroDisplayLiquidCrystal::begin()
{
    _lcd.begin(_screenSize[0], _screenSize[1]);
}

void AstroDisplayLiquidCrystal::setupRendering(Astro_DisplayTheme displayTheme, Astro_TitleMode titleMode, const void *itemFont, const void *titleFont, bool analogSlider, bool editingIcons, bool tcUnicodeFonts)
{
    // AstroDisplayDriver::setupRendering(displayTheme, titleMode, itemFont, titleFont, analogSlider, editingIcons, tcUnicodeFonts); // simply returns
    _renderer.setTitleRequired(titleMode == Astro_TitleMode_Always);
}

AstroOverview *AstroDisplayLiquidCrystal::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new AstroOverviewLCD(this);
}


AstroDisplayU8g2OLED::AstroDisplayU8g2OLED(DeviceSetup displaySetup, Astro_DisplayRotation displayRotation, U8G2 *gfx)
    : AstroDisplayDriver(displayRotation, gfx->getDisplayWidth(), gfx->getDisplayHeight()), // already rotated due to constructor, possibly incorrect until after begin
      _gfx(gfx), _drawable(nullptr), _renderer(nullptr)
{
    ASTRO_SOFT_ASSERT(_gfx, SFP(AStr_Err_AllocationFailure));
    if (_gfx) {
        if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
            _gfx->setI2CAddress(ASTRO_UI_I2C_OLED_BASEADDR | displaySetup.cfgAs.i2c.address);
        }
        #ifdef ASTRO_UI_ENABLE_STCHROMA_LDTC
            _drawable = new StChromaArtDrawable();
        #else
            if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                _drawable = new U8g2Drawable(_gfx, displaySetup.cfgAs.i2c.wire, getBaseUI() && getBaseUI()->isTcUnicodeFonts());
            } else {
                _drawable = new U8g2Drawable(_gfx, nullptr, getBaseUI() && getBaseUI()->isTcUnicodeFonts());
            }
        #endif
        ASTRO_SOFT_ASSERT(_drawable, SFP(AStr_Err_AllocationFailure));

        if (_drawable) {
            _renderer = new GraphicsDeviceRenderer(ASTRO_UI_RENDERER_BUFFERSIZE, AstroDisplayDriver::getSystemName(), _drawable);
            ASTRO_SOFT_ASSERT(_renderer, SFP(AStr_Err_AllocationFailure));
        }
    }
}

AstroDisplayU8g2OLED::~AstroDisplayU8g2OLED()
{
    if (_renderer) { delete _renderer; }
    if (_drawable) { delete _drawable; }
    if (_gfx) { delete _gfx; }
}

void AstroDisplayU8g2OLED::initBaseUIFromDefaults()
{
    getBaseUI()->init(ASTRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), Astro_DisplayTheme_MonoOLED), Astro_TitleMode_Always);
}

void AstroDisplayU8g2OLED::begin()
{
    if (_gfx) {
        _gfx->begin();
        _screenSize[0] = _gfx->getDisplayWidth();
        _screenSize[1] = _gfx->getDisplayHeight();
    }
}

AstroOverview *AstroDisplayU8g2OLED::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new AstroOverviewOLED(this, clockFont, detailFont);
}


AstroDisplayAdafruitGFX<Adafruit_ST7735>::AstroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Astro_DisplayRotation displayRotation, Astro_ST77XXKind st77Kind, pintype_t dcPin, pintype_t resetPin)
    : AstroDisplayDriver(displayRotation, _gfx.width(), _gfx.height()), _kind(st77Kind),
      #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, getBaseUI() ? getBaseUI()->getVRAMBufferRows() : 0),
      _renderer(ASTRO_UI_RENDERER_BUFFERSIZE, AstroDisplayDriver::getSystemName(), &_drawable)
{
    ASTRO_SOFT_ASSERT(_kind != Astro_ST77XXKind_Undefined, SFP(AStr_Err_InvalidParameter));
    #ifdef ESP8266
        ASTRO_SOFT_ASSERT(!(bool)ASTRO_USE_SPI || displaySetup.spi == ASTRO_USE_SPI, SFP(AStr_Err_InvalidParameter));
    #endif

    switch (_kind) {
        case Astro_ST7735Tag_Green144:
        case Astro_ST7735Tag_HalloWing:
            _screenSize[0] = 128; _screenSize[1] = 128;
            break;
        case Astro_ST7735Tag_Mini:
        case Astro_ST7735Tag_MiniPlugin:
            _screenSize[0] = 80; _screenSize[1] = 160;
            break;
        default:
            _screenSize[0] = 128; _screenSize[1] = 160;
            break;
    }
}

void AstroDisplayAdafruitGFX<Adafruit_ST7735>::initBaseUIFromDefaults()
{
    getBaseUI()->init(ASTRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Astro_DisplayTheme, ASTRO_UI_GFX_DISP_THEME_BASE, ASTRO_UI_GFX_DISP_THEME_SMLMED)), Astro_TitleMode_Always, ASTRO_UI_GFX_USE_ANALOG_SLIDER, ASTRO_UI_GFX_USE_EDITING_ICONS);
}

void AstroDisplayAdafruitGFX<Adafruit_ST7735>::begin()
{
    if (_kind == Astro_ST7735Tag_B) {
        _gfx.initB();
    } else {
        _gfx.initR((uint8_t)_kind);
    }
    _screenSize[0] = _gfx.width();
    _screenSize[1] = _gfx.height();
    _gfx.setRotation((uint8_t)_rotation);
}

AstroOverview *AstroDisplayAdafruitGFX<Adafruit_ST7735>::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new AstroOverviewGFX<Adafruit_ST7735>(this, clockFont, detailFont);
}


AstroDisplayAdafruitGFX<Adafruit_ST7789>::AstroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Astro_DisplayRotation displayRotation, Astro_ST77XXKind st77Kind, pintype_t dcPin, pintype_t resetPin)
    : AstroDisplayDriver(displayRotation, _gfx.width(), _gfx.height()), _kind(st77Kind),
      #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, getBaseUI() ? getBaseUI()->getVRAMBufferRows() : 0),
      _renderer(ASTRO_UI_RENDERER_BUFFERSIZE, AstroDisplayDriver::getSystemName(), &_drawable)
{
    ASTRO_SOFT_ASSERT(_kind != Astro_ST77XXKind_Undefined, SFP(AStr_Err_InvalidParameter));
    #ifdef ESP8266
        ASTRO_SOFT_ASSERT(!(bool)ASTRO_USE_SPI || displaySetup.spi == ASTRO_USE_SPI, SFP(AStr_Err_InvalidParameter));
    #endif

    switch (_kind) {
        case Astro_ST7789Res_128x128:
            _screenSize[0] = 128; _screenSize[1] = 128;
            break;
        case Astro_ST7789Res_135x240:
            _screenSize[0] = 135; _screenSize[1] = 240;
            break;
        case Astro_ST7789Res_170x320:
            _screenSize[0] = 170; _screenSize[1] = 320;
            break;
        case Astro_ST7789Res_172x320:
            _screenSize[0] = 172; _screenSize[1] = 320;
            break;
        case Astro_ST7789Res_240x240:
            _screenSize[0] = 240; _screenSize[1] = 240;
            break;
        case Astro_ST7789Res_240x280:
            _screenSize[0] = 240; _screenSize[1] = 280;
            break;
        case Astro_ST7789Res_240x320:
            _screenSize[0] = 240; _screenSize[1] = 320;
            break;
        default:
            _screenSize[0] = TFT_GFX_WIDTH; _screenSize[1] = TFT_GFX_HEIGHT;
            break;
    }
}

void AstroDisplayAdafruitGFX<Adafruit_ST7789>::initBaseUIFromDefaults()
{
    getBaseUI()->init(ASTRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Astro_DisplayTheme, ASTRO_UI_GFX_DISP_THEME_BASE, ASTRO_UI_GFX_DISP_THEME_SMLMED)), Astro_TitleMode_Always, ASTRO_UI_GFX_USE_ANALOG_SLIDER, ASTRO_UI_GFX_USE_EDITING_ICONS);
}

void AstroDisplayAdafruitGFX<Adafruit_ST7789>::begin()
{
    _gfx.init(_screenSize[0], _screenSize[1]);
    _screenSize[0] = _gfx.width();
    _screenSize[1] = _gfx.height();
    _gfx.setRotation((uint8_t)_rotation);
}

AstroOverview *AstroDisplayAdafruitGFX<Adafruit_ST7789>::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new AstroOverviewGFX<Adafruit_ST7789>(this, clockFont, detailFont);
}


AstroDisplayAdafruitGFX<Adafruit_ILI9341>::AstroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Astro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
    : AstroDisplayDriver(displayRotation, _gfx.width(), _gfx.height()), // possibly incorrect until after begin
      #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, getBaseUI() ? getBaseUI()->getVRAMBufferRows() : 0),
      _renderer(ASTRO_UI_RENDERER_BUFFERSIZE, AstroDisplayDriver::getSystemName(), &_drawable)
{
    #ifdef ESP8266
        ASTRO_SOFT_ASSERT(!(bool)ASTRO_USE_SPI || displaySetup.spi == ASTRO_USE_SPI, SFP(AStr_Err_InvalidParameter));
    #endif
}

void AstroDisplayAdafruitGFX<Adafruit_ILI9341>::initBaseUIFromDefaults()
{
    getBaseUI()->init(ASTRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Astro_DisplayTheme, ASTRO_UI_GFX_DISP_THEME_BASE, ASTRO_UI_GFX_DISP_THEME_SMLMED)), Astro_TitleMode_Always, ASTRO_UI_GFX_USE_ANALOG_SLIDER, ASTRO_UI_GFX_USE_EDITING_ICONS);
}

void AstroDisplayAdafruitGFX<Adafruit_ILI9341>::begin()
{
    _gfx.begin(getController()->getDisplaySetup().cfgAs.spi.speed);
    _screenSize[0] = _gfx.width();
    _screenSize[1] = _gfx.height();
    _gfx.setRotation((uint8_t)_rotation);
}

AstroOverview *AstroDisplayAdafruitGFX<Adafruit_ILI9341>::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new AstroOverviewGFX<Adafruit_ILI9341>(this, clockFont, detailFont);
}


AstroDisplayTFTeSPI::AstroDisplayTFTeSPI(SPIDeviceSetup displaySetup, Astro_DisplayRotation displayRotation, Astro_ST77XXKind st77Kind)
    : AstroDisplayDriver(displayRotation, TFT_GFX_WIDTH, TFT_GFX_HEIGHT),
      _kind(st77Kind),
      _gfx(TFT_GFX_WIDTH, TFT_GFX_HEIGHT),
      _drawable(&_gfx, getBaseUI() ? getBaseUI()->getVRAMBufferRows() : 0),
      _renderer(ASTRO_UI_RENDERER_BUFFERSIZE, AstroDisplayDriver::getSystemName(), &_drawable)
{ ; }

void AstroDisplayTFTeSPI::initBaseUIFromDefaults()
{
    getBaseUI()->init(ASTRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Astro_DisplayTheme, ASTRO_UI_GFX_DISP_THEME_BASE, ASTRO_UI_GFX_DISP_THEME_MEDLRG)), Astro_TitleMode_Always, ASTRO_UI_GFX_USE_ANALOG_SLIDER, ASTRO_UI_GFX_USE_EDITING_ICONS);
}

void AstroDisplayTFTeSPI::begin()
{
    if (_kind == Astro_ST7735Tag_B || _kind >= Astro_ST7789Res_Start) {
        _gfx.begin();
    } else {
        _gfx.begin((uint8_t)_kind);
    }
    _gfx.setRotation((uint8_t)_rotation);
    _renderer.setDisplayDimensions(getScreenSize().first, getScreenSize().second);
}

AstroOverview *AstroDisplayTFTeSPI::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new AstroOverviewTFT(this, clockFont, detailFont);
}

#endif
