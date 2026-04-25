cmake_minimum_required(VERSION 3.19)

# Check the APIs have been supported
set(DX12_SUPPORTED FALSE)
set(METAL_SUPPORTED FALSE)

# - DX12
if(PLATFORM_WINDOWS)
    find_library(D3D12_LIBRARY d3d12)
    find_library(DXGI_LIBRARY dxgi)
    if(D3D12_LIBRARY AND DXGI_LIBRARY)
        set(DX12_SUPPORTED TRUE)
    endif()
endif()
if(DX12_SUPPORTED)
    message(STATUS "CubeEngine: DX12 supported.")
endif()

# - Metal
if(PLATFORM_MACOS)
    find_library(METAL_FRAMEWORK Metal)
    if(METAL_FRAMEWORK)
        set(METAL_SUPPORTED TRUE)
    endif()
endif()
if(METAL_SUPPORTED)
    message(STATUS "CubeEngine: Metal supported.")
endif()

# Add enabled APIs
add_subdirectory("GAPI")
add_subdirectory("GAPI_Base")

if(CUBE_GAPI_ENABLE_DX12 AND DX12_SUPPORTED)
    add_subdirectory("GAPI_DX12")
endif()

if(CUBE_GAPI_ENABLE_METAL AND METAL_SUPPORTED)
    add_subdirectory("GAPI_Metal")
endif()
