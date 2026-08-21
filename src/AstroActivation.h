/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Activation
*/

#ifndef AstroActivation_H
#define AstroActivation_H

#include "AstroDefines.h"

class AstroActuator;

enum Astro_ActivationFlags : uint8_t {
    Astro_ActivationFlags_Forced = 0x01,                     // Forced
    Astro_ActivationFlags_None = 0x00                          // No special activation flags
};

// Actuator Activation Data
// Requested direction, normalized intensity, duration, and control flags.
struct AstroActivation {
    Astro_DirectionMode direction;                           // Normalized driving direction
    float intensity;                                         // Normalized driving intensity
    millis_t duration;                                       // Remaining activation duration, in milliseconds
    Astro_ActivationFlags flags;                             // Activation flags

    AstroActivation(Astro_DirectionMode directionIn = Astro_DirectionMode_Undefined,
                    float intensityIn = 0.0f,
                    millis_t durationIn = 0,
                    Astro_ActivationFlags flagsIn = Astro_ActivationFlags_None)
        : direction(directionIn), intensity(constrain(intensityIn, 0.0f, 1.0f)),
          duration(durationIn), flags(flagsIn)
    { ; }

    inline bool isValid() const { return direction != Astro_DirectionMode_Undefined; }
    inline bool isDone() const { return duration == 0; }
    inline bool isUntimed() const { return duration == (millis_t)-1; }
    inline bool isForced() const { return flags & Astro_ActivationFlags_Forced; }
    inline float getDriveIntensity() const {
        return direction == Astro_DirectionMode_Forward ? intensity :
               direction == Astro_DirectionMode_Reverse ? -intensity : 0.0f;
    }
};

// Actuator Activation Handle
// Keeps an actuator request alive while tracking elapsed and remaining activation time.
struct AstroActivationHandle {
    SharedPtr<AstroActuator> actuator;                      // Actuator owner, set only when activation requested
    AstroActivation activation;                              // Activation
    millis_t checkTime;                                      // Last activation update time, in milliseconds
    millis_t elapsed;                                        // Accumulated activation time, in milliseconds

    AstroActivationHandle(SharedPtr<AstroActuator> actuatorIn = nullptr,
                          Astro_DirectionMode direction = Astro_DirectionMode_Undefined,
                          float intensity = 0.0f,
                          millis_t duration = -1,
                          bool force = false);               // Force
    AstroActivationHandle(const AstroActivationHandle &handle);
    ~AstroActivationHandle();

    AstroActivationHandle &operator=(SharedPtr<AstroActuator> actuatorIn);
    AstroActivationHandle &operator=(const AstroActivation &activationIn);
    AstroActivationHandle &operator=(const AstroActivationHandle &handle);

    void unset();
    void elapseBy(millis_t delta);
    void elapseTo(millis_t time = nzMillis());

    inline bool isActive() const { return actuator && checkTime; }
    inline bool isValid() const { return activation.isValid(); }
    inline bool isDone() const { return activation.isDone(); }
    inline bool isUntimed() const { return activation.isUntimed(); }
    inline bool isForced() const { return activation.isForced(); }
    inline millis_t getTimeLeft() const { return activation.duration; }
    inline millis_t getTimeActive(millis_t time = nzMillis()) const { return isActive() ? (time - checkTime) + elapsed : elapsed; }
    inline float getDriveIntensity() const { return activation.getDriveIntensity(); }
};

#endif // /ifndef AstroActivation_H
