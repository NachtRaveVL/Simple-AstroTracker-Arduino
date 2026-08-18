cmake_minimum_required(VERSION 3.16)
project(AstruinoHostTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

find_package(Python3 COMPONENTS Interpreter)

set(ASTRUINO_SRC
    ../src/AstroActivation.cpp
    ../src/AstroActuators.cpp
    ../src/AstroAttachments.cpp
    ../src/AstroCamera.cpp
    ../src/AstroCoordinates.cpp
    ../src/AstroCover.cpp
    ../src/AstroData.cpp
    ../src/AstroDatas.cpp
    ../src/AstroDrivers.cpp
    ../src/AstroEphemeris.cpp
    ../src/AstroFactory.cpp
    ../src/AstroInterfaces.cpp
    ../src/AstroLib.cpp
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
)

add_library(astruino_host STATIC ${ASTRUINO_SRC})
target_include_directories(astruino_host PUBLIC ../src)

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

enable_testing()
add_test(NAME astronomy COMMAND test_astronomy)
add_test(NAME catalog COMMAND test_catalog)
add_test(NAME automation COMMAND test_automation)
add_test(NAME enums COMMAND test_enums)
add_test(NAME infrastructure COMMAND test_infrastructure)
add_test(NAME serialization COMMAND test_serialization)

if(Python3_Interpreter_FOUND)
    add_test(NAME enum_trie_generated COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generate_enum_trie.py --check)
    add_test(NAME source_validation COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/validate_source.py)
    add_test(NAME example_syntax COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/validate_examples.py)
endif()
