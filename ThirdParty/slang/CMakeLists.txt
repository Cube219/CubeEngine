cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching slang...")

# Download pre-built binaries (2026.18.3)
if(PLATFORM_WINDOWS)
    # TODO: Support arm64?
    FetchContent_Declare(
        slang
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        URL https://github.com/shader-slang/slang/releases/download/v2026.18.3/slang-2026.18.3-windows-x86_64.zip
    )
endif()
if(PLATFORM_MACOS)
    # TODO: Support x86_64?
    FetchContent_Declare(
        slang
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        URL https://github.com/shader-slang/slang/releases/download/v2026.18.3/slang-2026.18.3-macos-aarch64.zip
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
    set_target_properties(slang PROPERTIES
        IMPORTED_LOCATION
            ${slang_SOURCE_DIR}/bin/slang.dll
        IMPORTED_IMPLIB
            ${slang_SOURCE_DIR}/lib/slang.lib
    )
endif()
if(PLATFORM_MACOS)
    set_target_properties(slang PROPERTIES
        IMPORTED_LOCATION
            ${slang_SOURCE_DIR}/lib/libslang.dylib
    )
endif()

add_library(slang-compiler SHARED IMPORTED GLOBAL)
target_include_directories(slang-compiler
    INTERFACE
        ${slang_SOURCE_DIR}/include
)
if(PLATFORM_WINDOWS)
    set_target_properties(slang-compiler PROPERTIES
        IMPORTED_LOCATION
            ${slang_SOURCE_DIR}/bin/slang-compiler.dll
        IMPORTED_IMPLIB
            ${slang_SOURCE_DIR}/lib/slang-compiler.lib
    )
endif()
if(PLATFORM_MACOS)
    set_target_properties(slang-compiler PROPERTIES
        IMPORTED_LOCATION
            ${slang_SOURCE_DIR}/lib/libslang-compiler.dylib
    )
endif()
