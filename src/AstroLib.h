/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets Library
*/

#ifndef AstroLib_H
#define AstroLib_H

#include "AstroTargets.h"

// Target Library Book
// Holds one checked-out target entry and its checkout reference count.
struct AstroTargetsLibraryBook {
    AstroTargetData data;                                    // Data
    uint16_t count;                                          // Count
    bool userSet;                                            // User set

    AstroTargetsLibraryBook();
    AstroTargetsLibraryBook(const AstroTargetData &dataIn);
};

// Astronomy Target Library
// Provides cached access to built-in, user supplied, and externally loaded target data.
class AstroTargetsLibrary {
public:
    typedef bool (*ExternalTargetLoader)(void *context, Astro_TargetId targetId, AstroTargetData *dataOut);
    AstroTargetsLibrary();
    ~AstroTargetsLibrary();

    const AstroTargetData *checkoutTargetData(Astro_TargetId targetId);
    void returnTargetData(const AstroTargetData *targetData);

    bool setUserTargetData(const AstroTargetData *targetData);
    bool dropUserTargetData(const AstroTargetData *targetData);

    inline bool hasUserTargets() const { return _hasUserTargets; }
    uint16_t getCheckoutCount(Astro_TargetId targetId) const;
    size_t getLoadedBookCount() const;

    void beginTargetsLibraryFromSDCard(const AstroString &dataFilePrefix, bool jsonFormat = true);
    void beginTargetsLibraryFromEEPROM(size_t dataAddress, bool jsonFormat = false);
    void setExternalTargetLoader(ExternalTargetLoader loader, void *context = nullptr);

protected:
    AstroTargetsLibraryBook *_targetsData[Astro_Target_Count]; // Targets data, not owned
    bool _hasUserTargets;                                    // Has user targets
    AstroString _libSDTargetPrefix;                          // Lib sdtarget prefix
    size_t _libEEPROMDataAddress;                            // Lib eepromdata address
    bool _libSDJSONFormat;                                   // Lib sdjsonformat
    bool _libEEPROMJSONFormat;                               // Lib eepromjsonformat
    ExternalTargetLoader _externalLoader;                    // External loader
    void *_externalLoaderContext;                            // External loader context, not owned

    AstroTargetsLibraryBook *newBookFromType(Astro_TargetId targetId);
    AstroTargetsLibraryBook *newBookFromBuiltin(Astro_TargetId targetId);
};

extern AstroTargetsLibrary astroLib;

extern bool astroTargetIdToString(Astro_TargetId targetId, char *bufferOut, size_t bufferSize);
extern Astro_TargetId astroTargetIdFromString(const char *targetString);

#endif // /ifndef AstroLib_H
