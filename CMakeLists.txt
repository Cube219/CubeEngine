cmake_minimum_required (VERSION 3.6)

project(CubeEngine)

# Sets
set (DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/Dependencies)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (DEBUG_MODE TRUE)
endif()

set(PLATFORM_WIN32 FALSE)
if(WIN32)
    set(PLATFORM_WIN32 TRUE)
    message("Target platform: Win32")
endif()

# Set global project settings
set (CMAKE_CXX_STANDARD 17)

# Set platform-specific settings
if(PLATFORM_WIN32)
    add_definitions(-D_UNICODE -DUNICODE -DNOMINMAX)
    # Set utf-8 option
    add_compile_options(/utf-8)
endif()

# Set output directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE})

include_directories("Source")

add_subdirectory("Source/Utility")
add_subdirectory("Source/Platform")
add_subdirectory("Source/Core")
add_subdirectory("Source/RenderAPIs")
add_subdirectory("Source/Modules/InputModule")
add_subdirectory("Source/Engine")
add_subdirectory("Source/Main")
