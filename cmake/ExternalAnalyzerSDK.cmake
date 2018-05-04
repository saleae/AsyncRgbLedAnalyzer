include(FetchContent)

function(prepare_analyzer_sdk)
FetchContent_Declare(
    analyzersdk
    GIT_REPOSITORY https://github.com/TheOneRing/analyzersdk.git
    GIT_TAG        cmake_target
    GIT_SHALLOW    True
    GIT_PROGRESS   True
)

FetchContent_GetProperties(analyzersdk)

if(NOT analyzersdk_POPULATED)
    FetchContent_Populate(analyzersdk)
    include(${analyzersdk_SOURCE_DIR}/AnalyzerSDKConfig.cmake)
endif()


endfunction()
