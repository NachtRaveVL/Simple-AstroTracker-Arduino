cmake_minimum_required(VERSION 3.16)
project(AstruinoHostTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Werror)
endif()

find_package(Python3 COMPONENTS Interpreter)

set(ASTRUINO_SRC
    ../src/AstroActivation.cpp
    ../src/AstroActuators.cpp
    ../src/AstroAttachments.cpp
    ../src/AstroCamera.cpp
    ../src/AstroCoreLogic.cpp
    ../src/AstroData.cpp
    ../src/AstroDatas.cpp
    ../src/AstroDrivers.cpp
    ../src/AstroFactory.cpp
    ../src/AstroTargetsLibrary.cpp
    ../src/AstroLogger.cpp
    ../src/AstroMeasurements.cpp
    ../src/AstroModules.cpp
    ../src/AstroMounts.cpp
    ../src/AstroObject.cpp
    ../src/AstroPins.cpp
    ../src/AstroPublisher.cpp
    ../src/AstroRails.cpp
    ../src/AstroScheduler.cpp
    ../src/AstroSensors.cpp
    ../src/AstroStreams.cpp
    ../src/AstroStrings.cpp
    ../src/AstroTargets.cpp
    ../src/AstroThermal.cpp
    ../src/AstroTriggers.cpp
    ../src/AstroUtils.cpp
    ../src/Astruino.cpp
    ../src/shared/AstroUIData.cpp
)

add_library(astruino_host STATIC ${ASTRUINO_SRC})
target_include_directories(astruino_host PUBLIC ../src)
target_include_directories(astruino_host SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/host/shims
)
target_compile_definitions(astruino_host PUBLIC
    ARDUINO=10819
    ASTRO_DISABLE_GUI
    ASTRO_DISABLE_MULTITASKING
)

add_library(astruino_host_debug STATIC ${ASTRUINO_SRC})
target_include_directories(astruino_host_debug PUBLIC ../src)
target_include_directories(astruino_host_debug SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/host/shims
)
target_compile_definitions(astruino_host_debug PUBLIC
    ARDUINO=10819
    ASTRO_DISABLE_GUI
    ASTRO_DISABLE_MULTITASKING
)
target_compile_definitions(astruino_host_debug PRIVATE
    ASTRO_ENABLE_DEBUG_OUTPUT
    ASTRO_ENABLE_VERBOSE_DEBUG
    ASTRO_ENABLE_DEBUG_ASSERTIONS
)

add_executable(test_astronomy host/test_astronomy.cpp)
target_link_libraries(test_astronomy PRIVATE astruino_host)

add_executable(test_catalog host/test_catalog.cpp)
target_link_libraries(test_catalog PRIVATE astruino_host)

add_executable(test_automation host/test_automation.cpp)
target_link_libraries(test_automation PRIVATE astruino_host)

add_executable(test_enums host/test_enums.cpp)
target_link_libraries(test_enums PRIVATE astruino_host)

add_executable(test_infrastructure host/test_infrastructure.cpp)
target_link_libraries(test_infrastructure PRIVATE astruino_host)

add_executable(test_serialization host/test_serialization.cpp)
target_link_libraries(test_serialization PRIVATE astruino_host)

add_executable(test_hardening host/test_hardening.cpp)
target_link_libraries(test_hardening PRIVATE astruino_host)

add_executable(test_hardware host/test_hardware.cpp)
target_link_libraries(test_hardware PRIVATE astruino_host)

add_executable(test_debug host/test_debug.cpp)
target_link_libraries(test_debug PRIVATE astruino_host_debug)
target_compile_definitions(test_debug PRIVATE
    ASTRO_ENABLE_DEBUG_OUTPUT
    ASTRO_ENABLE_VERBOSE_DEBUG
    ASTRO_ENABLE_DEBUG_ASSERTIONS
)

enable_testing()
add_test(NAME astronomy COMMAND test_astronomy)
add_test(NAME catalog COMMAND test_catalog)
add_test(NAME automation COMMAND test_automation)
add_test(NAME enums COMMAND test_enums)
add_test(NAME infrastructure COMMAND test_infrastructure)
add_test(NAME serialization COMMAND test_serialization)
add_test(NAME hardening COMMAND test_hardening)
add_test(NAME hardware COMMAND test_hardware)
add_test(NAME debug COMMAND test_debug)
