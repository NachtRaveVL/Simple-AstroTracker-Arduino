/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Callback Helpers
*/

#ifndef AstroCallback_HH
#define AstroCallback_HH

#include "AstroCompat.h"

template<class TArg, size_t Slots = 4>
// Signal Callback Helper
// Small fixed-slot callback signal used by the embedded object layer.
class AstroSignal {
public:
    typedef void (*Callback)(void *context, TArg arg);

    AstroSignal() : _slots{nullptr}, _contexts{nullptr} { ; }

    bool attach(Callback callback, void *context = nullptr)
    {
        if (!callback) { return false; }
        for (size_t i = 0; i < Slots; ++i) {
            if (_slots[i] == callback && _contexts[i] == context) { return true; }
            if (!_slots[i]) {
                _slots[i] = callback;                        // Slots
                _contexts[i] = context;                      // Contexts
                return true;
            }
        }
        return false;
    }

    bool detach(Callback callback, void *context = nullptr)
    {
        for (size_t i = 0; i < Slots; ++i) {
            if (_slots[i] == callback && _contexts[i] == context) {
                _slots[i] = nullptr;                         // Slots
                _contexts[i] = nullptr;                      // Contexts
                return true;
            }
        }
        return false;
    }

    void fire(TArg arg)
    {
        for (size_t i = 0; i < Slots; ++i) {
            if (_slots[i]) { _slots[i](_contexts[i], arg); }
        }
    }

protected:
    Callback _slots[Slots];                                  // Slots
    void *_contexts[Slots];                                  // Contexts, not owned
};

#endif // /ifndef AstroCallback_HH
