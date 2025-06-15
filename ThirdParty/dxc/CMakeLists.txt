cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching dxc...")

# Download pre-built binaries (February 2025)
# TODO: Support arm64?
FetchContent_Declare(
    dxc
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2502/dxc_2025_02_20.zip
)
FetchContent_MakeAvailable(dxc)

# Target settings
add_library(dxc INTERFACE IMPORTED GLOBAL)
target_include_directories(dxc
    INTERFACE
        ${dxc_SOURCE_DIR}/inc
)

# TODO: Support arm64?
set(DXC_ARCH "x64")

add_library(dxc_dxcompiler SHARED IMPORTED GLOBAL)
set_target_properties(dxc_dxcompiler PROPERTIES
    IMPORTED_LOCATION
        ${dxc_SOURCE_DIR}/bin/${DXC_ARCH}/dxcompiler.dll
    IMPORTED_IMPLIB
        ${dxc_SOURCE_DIR}/lib/${DXC_ARCH}/dxcompiler.lib
)

add_library(dxc_dxil SHARED IMPORTED GLOBAL)
set_target_properties(dxc_dxil PROPERTIES
    IMPORTED_LOCATION
        ${dxc_SOURCE_DIR}/bin/${DXC_ARCH}/dxil.dll
    IMPORTED_IMPLIB
        ${dxc_SOURCE_DIR}/lib/${DXC_ARCH}/dxil.lib
)

set_property(TARGET dxc PROPERTY
    INTERFACE_LINK_LIBRARIES
        dxc_dxcompiler
        dxc_dxil
)
