/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets Library
*/

#ifndef AstroTargetsLibrary_H
#define AstroTargetsLibrary_H

class AstroTargetsLibrary;
struct AstroTargetsLibraryBook;

#include "Astruino.h"

// Targets Library
// Target data is vast and most microcontrollers don't have the memory to load all the target
// data up at once. The targets library uses a library book like checkout and return system,
// in which case reference counting is performed to see which targets need to be loaded and
// which ones can unload. It is recommended to use the AstroTargetsLibData constructor
// if using a temporary, otherwise this checkout/return system. The returned target lib data
// instance is guaranteed to stay unique for as long as it is allocated.
// Unless the Astro_DISABLE_BUILTIN_DATA define is defined, all target data is
// internally stored as JSON strings in the Flash PROGMEM memory space. See the Data
// Writer Example sketch on how to program an EEPROM or SD card with such data.
class AstroTargetsLibrary {
public:
    // Begins targets library from external SD card library, with specified file prefix and data format.
    void beginTargetsLibraryFromSDCard(String dataFilePrefix, bool jsonFormat = true);

    // Begins targets library from external EEPROM, with specified data begin address and data format.
    void beginTargetsLibraryFromEEPROM(size_t dataAddress = 0, bool jsonFormat = false);

    // Checks out the target data for this target from the library, created via the JSON from
    // PROGMEM if needed (nullptr return -> failure). Increments target data ref count by one.
    const AstroTargetsLibData *checkoutTargetsData(Astro_TargetType targetType);

    // Returns target data back to the library, to delete when no longer used. Decrements target
    // data internal ref count by one, deleting on zero.
    void returnTargetsData(const AstroTargetsLibData *targetData);

    // Adds/updates custom target data to the library, returning success flag
    bool setUserTargetData(const AstroTargetsLibData *targetData);

    // Drops/removes custom target data from the library, returning success flag
    bool dropUserTargetData(const AstroTargetsLibData *targetData);

    // Returns if there are custom targets in the library
    inline bool hasUserTargets() const { return _hasUserTargets; }

protected:
    Map<Astro_TargetType, AstroTargetsLibraryBook *, ASTRO_TARGETS_TARGETSLIB_MAXSIZE> _targetsData; // Loaded targets library data
    bool _hasUserTargets = false;                             // Has user targets flag

    String _libSDTargetPrefix;                                // Library data files prefix for SD card, else ""/unused
    bool _libSDJSONFormat = false;                          // Library SD card data files JSON format tracking flag
    size_t _libEEPROMDataAddress = (size_t)-1;              // Library EEPROM data begin address, else -1/unused
    bool _libEEPROMJSONFormat = false;                      // Library EEPROM data JSON format tracking flag

    AstroTargetsLibraryBook *newBookFromType(Astro_TargetType targetType);
    bool updateHasUserTargets();
    void updateTargetsOfType(Astro_TargetType targetType);

    friend class Astruino;
};

// Targets Library Book
// Holds one checked-out target entry and its checkout reference count.
struct AstroTargetsLibraryBook {
    AstroTargetsLibData data;                                 // Target library data
    int count;                                              // Reference count
    bool userSet;                                           // If data was user set (not read from device)

    AstroTargetsLibraryBook();
    AstroTargetsLibraryBook(String jsonStringIn);
    AstroTargetsLibraryBook(Stream &streamIn, bool jsonFormat);
    AstroTargetsLibraryBook(const AstroTargetsLibData &dataIn);
    inline Astro_TargetType getKey() const { return data.targetType; }
};
    
extern AstroTargetsLibrary AstroTargetsLib;

#endif // /ifndef AstroTargetsLibrary_H

