cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching DirectX Agility SDK...")

# TODO: Support arm64?
FetchContent_Declare(
    dxagility
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.616.1
)
FetchContent_MakeAvailable(dxagility)

# Target settings
add_library(dxagility_d3d12core SHARED IMPORTED GLOBAL)
set_target_properties(dxagility_d3d12core PROPERTIES
    IMPORTED_LOCATION
        ${dxagility_SOURCE_DIR}/build/native/bin/x64/D3D12Core.dll
)

add_library(dxagility_d3d12sdklayers SHARED IMPORTED GLOBAL)
set_target_properties(dxagility_d3d12sdklayers PROPERTIES
    IMPORTED_LOCATION
        ${dxagility_SOURCE_DIR}/build/native/bin/x64/d3d12SDKLayers.dll
)

add_library(dxagility INTERFACE IMPORTED GLOBAL)
target_include_directories(dxagility
    INTERFACE
        ${dxagility_SOURCE_DIR}/build/native/include
)

# Not link each libraries because they doesn't have .lib files.
# set_property(TARGET dxagility PROPERTY
#     INTERFACE_LINK_LIBRARIES
#         dxagility_d3d12core
#         dxagility_d3d12sdklayers
# )
