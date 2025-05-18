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
add_library(dxc SHARED IMPORTED GLOBAL)
target_include_directories(dxc
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/inc
)

# TODO: Support arm64?
set(DXC_ARCH "x64")
set(DXC_LIB
    ${dxc_SOURCE_DIR}/lib/${DXC_ARCH}/dxcompiler.lib
    ${dxc_SOURCE_DIR}/lib/${DXC_ARCH}/dxil.lib
)
set(DXC_DLL
    ${dxc_SOURCE_DIR}/bin/${DXC_ARCH}/dxcompiler.dll
    ${dxc_SOURCE_DIR}/bin/${DXC_ARCH}/dxil.dll
)
set_target_properties(dxc PROPERTIES
    IMPORTED_LOCATION
        "${DXC_DLL}"
    IMPORTED_IMPLIB
        "${DXC_LIB}"
)
