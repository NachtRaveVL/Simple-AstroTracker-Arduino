#include "Astruino.h"
#include <cstdlib>
#include <iostream>

#ifndef ASTRO_ENABLE_DEBUG_OUTPUT
#error ASTRO_ENABLE_DEBUG_OUTPUT must be enabled for debug host test
#endif
#ifndef ASTRO_ENABLE_VERBOSE_DEBUG
#error ASTRO_ENABLE_VERBOSE_DEBUG must be enabled for debug host test
#endif
#ifndef ASTRO_ENABLE_DEBUG_ASSERTIONS
#error ASTRO_ENABLE_DEBUG_ASSERTIONS must be enabled for debug host test
#endif
#ifndef ASTRO_USE_VERBOSE_OUTPUT
#error ASTRO_USE_VERBOSE_OUTPUT must be enabled when debug and verbose output are enabled
#endif
#ifndef ASTRO_USE_DEBUG_ASSERTIONS
#error ASTRO_USE_DEBUG_ASSERTIONS must be enabled when debug output and assertions are enabled
#endif

static void check(bool condition, const char *message)
{
    if (!condition) { std::cerr << "FAIL: " << message << std::endl; std::exit(1); }
}

struct LogState {
    int count = 0;
    Astro_LogLevel lastLevel = Astro_LogLevel_None;
    void handle(const AstroLogEvent event) { ++count; lastLevel = event.level; }
};

int main()
{
    Astruino controller;
    controller.init();

    LogState logState;
    MethodSlot<LogState, const AstroLogEvent> logSlot(&logState, &LogState::handle);
    controller.logger.getLogSignal().attach(logSlot);

    controller.logger.logMessage("debug output");
    check(logState.count == 1 && logState.lastLevel == Astro_LogLevel_Info,
          "debug-enabled logger emits normal log event");

    ASTRO_SOFT_ASSERT(false, F("debug assertion"));
    check(logState.count == 2 && logState.lastLevel == Astro_LogLevel_Warnings,
          "debug assertion emits warning without aborting");

    flushYield();

    std::cout << "PASS debug" << std::endl;
    return 0;
}
