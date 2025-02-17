cmake_minimum_required(VERSION 3.19)

project(CubeEngine)

# Sets
set (DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/Dependencies)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (DEBUG_MODE TRUE)
endif()

set(PLATFORM_WINDOWS FALSE)
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    message("Target platform: Windows")
endif()

# Global project settings
set (CMAKE_CXX_STANDARD 20)

# Platform-specific settings
if(PLATFORM_WINDOWS)
    add_definitions(-DCUBE_PLATFORM_WINDOWS)

    add_definitions(-D_UNICODE -DUNICODE -DNOMINMAX)
    add_compile_options(/utf-8)
endif()

# Output directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Binaries/${CMAKE_BUILD_TYPE})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Binaries/${CMAKE_BUILD_TYPE})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Binaries/${CMAKE_BUILD_TYPE})

add_subdirectory("ThirdParty/fmt")
add_subdirectory("ThirdParty/imgui")

add_subdirectory("Source/Base")
add_subdirectory("Source/Logger")
add_subdirectory("Source/Platform")
add_subdirectory("Source/GraphicsAPI")
add_subdirectory("Source/Main")
