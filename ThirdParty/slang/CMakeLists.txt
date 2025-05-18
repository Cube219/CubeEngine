cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching slang...")

# Download pre-built binaries (2025.8.1)
if(PLATFORM_WINDOWS)
    # TODO: Support arm64?
    FetchContent_Declare(
        slang
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        URL https://github.com/shader-slang/slang/releases/download/v2025.8.1/slang-2025.8.1-windows-x86_64.zip
    )
endif()
if(PLATFORM_MACOS)
    # TODO: Support x86_64?
    FetchContent_Declare(
        slang
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        URL https://github.com/shader-slang/slang/releases/download/v2025.8.1/slang-2025.8.1-macos-aarch64.zip
    )
endif()
FetchContent_MakeAvailable(slang)

# Target settings
add_library(slang SHARED IMPORTED GLOBAL)
target_include_directories(slang
    INTERFACE
        ${slang_SOURCE_DIR}/include
)
if(PLATFORM_WINDOWS)
    set(SLANG_LIB
        ${slang_SOURCE_DIR}/lib/slang.lib
    )
    set(SLANG_DLL
        ${slang_SOURCE_DIR}/bin/slang.dll
    )
    set_target_properties(slang PROPERTIES
        IMPORTED_LOCATION
            "${SLANG_DLL}"
        IMPORTED_IMPLIB
            "${SLANG_LIB}"
    )
endif()
if(PLATFORM_MACOS)
    set(SLANG_LIB
        ${slang_SOURCE_DIR}/lib/libslang.dylib
    )
    set_target_properties(slang PROPERTIES
        IMPORTED_LOCATION
            "${SLANG_LIB}"
    )
endif()
