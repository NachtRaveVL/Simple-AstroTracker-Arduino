/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino UI Data
*/

#ifndef AstroUIData_H
#define AstroUIData_H

struct AstroUIData;

#include "Astruino.h"
#include "AstroUIDefines.h"

// UI Serialization Data
// id: AUID. Astruino UI data.
struct AstroUIData : public AstroData {
    uint8_t updatesPerSec;                                  // Updates per second (1-10, default: ASTRO_UI_UPDATE_SPEED)
    Astro_DisplayTheme displayTheme;                        // Display theme (if supported)
    Astro_TitleMode titleMode;                              // Title mode
    bool analogSlider;                                      // Use analog slider
    bool editingIcons;                                      // Use editing icons
    float joystickCalib[3];                                 // Joystick calibration ({midX,midY,zeroTol}, default: {0.5,0.5,0.05})
    uint16_t touchscreenCalib[4];                           // Touchscreen calibration ({x0,y0,x1,y1}), default: {0,0,0,0})

    AstroUIData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroUIData_H
