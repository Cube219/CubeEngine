cmake_minimum_required(VERSION 3.19)

# Options
option(CE_GAPI_ENABLE_DX12 "GraphicsAPI - Enable DirectX12" ON)

# Check the APIs have been supported
set(DX12_SUPPORTED FALSE)

# - DX12
if(PLATFORM_WINDOWS)
    # TODO: check dx12 sdk
    set(DX12_SUPPORTED TRUE)
endif()
if(DX12_SUPPORTED)
    message("GAPI: DX12 supported.")
endif()


# Add enabled APIs
add_subdirectory("GAPI")

if(CE_GAPI_ENABLE_DX12 AND DX12_SUPPORTED)
    add_subdirectory("GAPI_DX12")
endif()
