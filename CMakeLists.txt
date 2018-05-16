cmake_minimum_required (VERSION 3.11)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project(AsyncRgbLedAnalyzer)

if(WIN32)
  add_definitions(-DNOMINMAX)
endif()

include (Dart)
enable_testing()

option(ENABLE_ASTYLE  "Set to ON to enable AStyle formating of the code" ON)

include(ExternalAnalyzerSDK)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)

prepare_analyzer_sdk()


set(SOURCES source/AsyncRgbLedAnalyzer.cpp
            source/AsyncRgbLedAnalyzer.h
            source/AsyncRgbLedHelpers.cpp
            source/AsyncRgbLedHelpers.h
            source/AsyncRgbLedAnalyzerSettings.cpp
            source/AsyncRgbLedAnalyzerSettings.h
            source/AsyncRgbLedAnalyzerResults.cpp
            source/AsyncRgbLedAnalyzerResults.h
            source/AsyncRgbLedSimulationDataGenerator.cpp
            source/AsyncRgbLedSimulationDataGenerator.h
)

add_library(AsyncRgbLedAnalyzer SHARED ${SOURCES})
target_link_libraries(AsyncRgbLedAnalyzer PRIVATE Saleae::AnalyzerSDK)

# TODO - define installation dir
#install(TARGETS AsyncRgbLedAnalyzer RUNTIME DESTINATION ${ANALYZER_PLUGIN_DIR})

#------------------------------------------------------------------------
# Testing

add_subdirectory(AnalyzerTestHarness)

add_executable(AsyncRgbLedTest tests/AsyncRgbLedTestDriver.cpp ${SOURCES})
target_link_libraries(AsyncRgbLedTest AnalyzerTestHarness)

add_test(NAME AsyncRgbLedTest COMMAND AsyncRgbLedTest)

#------------------------------------------------------------------------
# AStyle
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include( ExternalAStyle )
