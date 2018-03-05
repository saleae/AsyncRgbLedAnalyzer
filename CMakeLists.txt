cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

project(AsyncRgbLedAnalyzer)

set(SOURCES source/AsyncRgbLedAnalyzer.cpp
            source/AsyncRgbLedAnalyzer.h
            source/AsyncRgbLedAnalyzerSettings.cpp
            source/AsyncRgbLedAnalyzerSettings.h
            source/AsyncRgbLedAnalyzerResults.cpp
            source/AsyncRgbLedAnalyzerResults.h
            source/AsyncRgbLedSimulationDataGenerator.cpp
            source/AsyncRgbLedSimulationDataGenerator.h
)

add_library(AsyncRgbLedAnalyzer SHARED ${SOURCES})

# TODO - make an imported target for the AnalyzerLib
set (ANALYZER_SDK_ROOT "${PROJECT_SOURCE_DIR}/AnalyzerSDK")
if (APPLE)
    target_link_libraries(AsyncRgbLedAnalyzer ${ANALYZER_SDK_ROOT}/lib/libAnalyzer.dylib)
elseif(MSVC)
    if( CMAKE_CL_64 )
        target_link_libraries(AsyncRgbLedAnalyzer ${ANALYZER_SDK_ROOT}/lib/Analyzer64.lib)
    else()
         target_link_libraries(AsyncRgbLedAnalyzer ${ANALYZER_SDK_ROOT}/lib/Analyzer.lib)
    endif()
elseif(UNIX AND NOT APPLE)
 	target_link_libraries(AsyncRgbLedAnalyzer ${ANALYZER_SDK_ROOT}/lib/libAnalyzer.so)
endif()

target_include_directories(AsyncRgbLedAnalyzer PRIVATE ${ANALYZER_SDK_ROOT}/include)

# TODO - define installation dir
#install(TARGETS AsyncRgbLedAnalyzer RUNTIME DESTINATION ${ANALYZER_PLUGIN_DIR})
