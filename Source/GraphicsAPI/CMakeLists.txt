cmake_minimum_required(VERSION 3.19)

# Options
option(CUBE_GAPI_ENABLE_DX12 "GraphicsAPI - Enable DirectX12" ON)
option(CUBE_GAPI_ENABLE_METAL "GraphicsAPI - Enable Metal" ON)

# Check the APIs have been supported
set(DX12_SUPPORTED FALSE)
set(METAL_SUPPORTED FALSE)

# - DX12
if(PLATFORM_WINDOWS)
    # TODO: check dx12 sdk
    set(DX12_SUPPORTED TRUE)
endif()
if(DX12_SUPPORTED)
    message(STATUS "CubeEngine: DX12 supported.")
endif()

# - Metal
if(PLATFORM_MACOS)
    # TODO check
    set(METAL_SUPPORTED TRUE)
endif()
if(METAL_SUPPORTED)
    message(STATUS "CubeEngine: Metal supported.")
endif()

# Add enabled APIs
add_subdirectory("GAPI")

if(CUBE_GAPI_ENABLE_DX12 AND DX12_SUPPORTED)
    add_subdirectory("GAPI_DX12")
endif()

if(CUBE_GAPI_ENABLE_METAL AND METAL_SUPPORTED)
    add_subdirectory("GAPI_Metal")
endif()
