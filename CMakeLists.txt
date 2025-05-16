cmake_minimum_required(VERSION 3.19)

project(CubeEngine)

# Options
option(CUBE_FETCH_GLTF_SAMPLES "Fetch glTF sample assets" OFF)

# Sets
set (DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/Dependencies)
set (RESOURCES_DIR ${PROJECT_SOURCE_DIR}/Resources)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (DEBUG_MODE TRUE)
endif()
set (GLOBAL_FILES
    ${RESOURCES_DIR}/natvis/CubeEngine.natvis
)

set(PLATFORM_WINDOWS FALSE)
set(PLATFORM_MACOS FALSE)
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    message(STATUS "CubeEngine: Target platform: Windows")
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    message(STATUS "CubeEngine: Target platform: MacOS")
else()

endif()

# Global variables
set(G_ENABLED_GAPI_MODULES "" CACHE INTERNAL "g_enabled_gapi_modules")

# Global project settings
set (CMAKE_CXX_STANDARD 20)

# Platform-specific settings
if(PLATFORM_WINDOWS)
    add_definitions(-DCUBE_PLATFORM_WINDOWS)

    add_definitions(-D_UNICODE -DUNICODE -DNOMINMAX)
    add_compile_options(/utf-8)
endif()
if(PLATFORM_MACOS)
    add_definitions(-DCUBE_PLATFORM_MACOS)
endif()

# Output directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Binaries/${CMAKE_BUILD_TYPE})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Binaries/${CMAKE_BUILD_TYPE})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Binaries/${CMAKE_BUILD_TYPE})

add_subdirectory("ThirdParty/fmt")
add_subdirectory("ThirdParty/imgui")
add_subdirectory("ThirdParty/slang")
add_subdirectory("ThirdParty/tinygltf")
add_subdirectory("ThirdParty/tinyobjloader")
if(PLATFORM_WINDOWS)
    add_subdirectory("ThirdParty/D3D12MemoryAllocator")
    add_subdirectory("ThirdParty/dxc")
endif()

add_subdirectory("Source/Base")
add_subdirectory("Source/Logger")
add_subdirectory("Source/Platform")
add_subdirectory("Source/GraphicsAPI")
add_subdirectory("Source/Core")
add_subdirectory("Source/Main")

# Fetch model resources
add_subdirectory("Resources/Models")
