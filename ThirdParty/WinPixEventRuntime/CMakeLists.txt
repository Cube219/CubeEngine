cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching WinPixEventRuntime...")

FetchContent_Declare(
    winpixeventruntime
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.240308001
)
FetchContent_MakeAvailable(winpixeventruntime)

add_library(winpixeventruntime SHARED IMPORTED GLOBAL)
target_include_directories(winpixeventruntime
    INTERFACE
        ${winpixeventruntime_SOURCE_DIR}/Include
)
# TODO: Support arm64?
set_target_properties(winpixeventruntime PROPERTIES
    IMPORTED_LOCATION
        ${winpixeventruntime_SOURCE_DIR}/bin/x64/WinPixEventRuntime.dll
    IMPORTED_IMPLIB
        ${winpixeventruntime_SOURCE_DIR}/bin/x64/WinPixEventRuntime.lib
)
