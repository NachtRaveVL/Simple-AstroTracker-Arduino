/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Activation
*/

#ifndef AstroActivation_H
#define AstroActivation_H

struct AstroActivation;
struct AstroActivationHandle;

#include "Astruino.h"

// Activation Flags
enum Astro_ActivationFlags : unsigned char {
    Astro_ActivationFlags_Forced        = 0x01,             // Force enable / ignore cursory canEnable checks
    Astro_ActivationFlags_None          = 0x00              // Placeholder
};

// Activation Data
// Activation setup data that is its own object for ease of use. Used to define what
// encapsulates an activation.
struct AstroActivation {
    Astro_DirectionMode direction;                          // Normalized driving direction
    float intensity;                                        // Normalized driving intensity ([0.0,1.0])
    millis_t duration;                                      // Duration time remaining, in milliseconds, else -1 for non-diminishing/unlimited or 0 for finished
    Astro_ActivationFlags flags;                            // Activation flags

    inline AstroActivation(Astro_DirectionMode directionIn, float intensityIn, millis_t durationIn, Astro_ActivationFlags flagsIn) : direction(directionIn), intensity(constrain(intensityIn, 0.0f, 1.0f)), duration(durationIn), flags(flagsIn) { ; }
    inline AstroActivation() : AstroActivation(Astro_DirectionMode_Undefined, 0.0f, 0, Astro_ActivationFlags_None) { ; }

    inline bool isValid() const { return direction != Astro_DirectionMode_Undefined; }
    inline bool isDone() const { return duration == millis_none; }
    inline bool isUntimed() const { return duration == (millis_t)-1; }
    inline bool isForced() const { return flags & Astro_ActivationFlags_Forced; }
    inline float getDriveIntensity() const { return direction == Astro_DirectionMode_Forward ? intensity :
                                                    direction == Astro_DirectionMode_Reverse ? -intensity : 0.0f; }
};

// Activation Handle
// Since actuators are shared objects, those wishing to enable any actuator must receive
// a valid handle. Actuators may customize how they handle multiple activation handles.
// Handles represent a driving intensity value ranged [0,1] or [-1,1] depending on the
// capabilities of the attached actuator. Handles do not guarantee activation unless their
// forced flag is set (also see Actuator activation signal), but can be set up to ensure
// actuators are enabled for a specified duration, which is able to be async updated.
struct AstroActivationHandle {
    SharedPtr<AstroActuator> actuator;                      // Actuator owner, set only when activation requested (use operator= to set)
    AstroActivation activation;                             // Activation data
    millis_t checkTime;                                     // Last check timestamp, in milliseconds, else 0 for not started
    millis_t elapsed;                                       // Elapsed time accumulator, in milliseconds, else 0

    // Handle constructor that specifies a normalized enablement, ranged: [0.0,1.0] for specified direction
    AstroActivationHandle(SharedPtr<AstroActuator> actuator, Astro_DirectionMode direction, float intensity = 1.0f, millis_t duration = -1, bool force = false);

    // Default constructor for empty handles
    inline AstroActivationHandle() : AstroActivationHandle(nullptr, Astro_DirectionMode_Undefined, 0.0f, 0, false) { ; }
    AstroActivationHandle(const AstroActivationHandle &handle);
    ~AstroActivationHandle();
    AstroActivationHandle &operator=(SharedPtr<AstroActuator> actuator);
    inline AstroActivationHandle &operator=(const AstroActivation &activationIn) { activation = activationIn; return *this; }
    inline AstroActivationHandle &operator=(const AstroActivationHandle &handle) { activation = handle.activation; return operator=(handle.actuator); }

    // Disconnects activation from an actuator (removes handle reference from actuator)
    void unset();

    // Elapses activation by delta, updating relevant activation values
    void elapseBy(millis_t delta);
    inline void elapseTo(millis_t time = nzMillis()) { elapseBy(time - checkTime); }

    inline bool isActive() const { return actuator && isValidTime(checkTime); }
    inline bool isValid() const { return activation.isValid(); }
    inline bool isDone() const { return activation.isDone(); }
    inline bool isUntimed() const { return activation.isUntimed(); }
    inline bool isForced() const { return activation.isForced(); }

    inline millis_t getTimeLeft() const { return activation.duration; }
    inline millis_t getTimeActive(millis_t time = nzMillis()) const { return isActive() ? (time - checkTime) + elapsed : elapsed; }

    // De-normalized driving intensity value [-1.0,1.0]
    inline float getDriveIntensity() const { return activation.getDriveIntensity(); }
};

#endif // /ifndef AstroActivation_H
