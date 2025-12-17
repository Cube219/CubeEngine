cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/DX12APIObject.h
    Public/DX12Header.h
    Public/GAPI_DX12.h
    Public/GAPI_DX12Buffer.h
    Public/GAPI_DX12CommandList.h
    Public/GAPI_DX12Fence.h
    Public/GAPI_DX12Pipeline.h
    Public/GAPI_DX12Resource.h
    Public/GAPI_DX12Sampler.h
    Public/GAPI_DX12Shader.h
    Public/GAPI_DX12ShaderParameter.h
    Public/GAPI_DX12SwapChain.h
    Public/GAPI_DX12Texture.h
)

set(PRECOMPILE_HEADER_FILES
    Public/DX12Header.h
)

set(PRIVATE_FILES
    Private/DX12CommandListManager.cpp
    Private/DX12CommandListManager.h
    Private/DX12Debug.cpp
    Private/DX12Debug.h
    Private/DX12DescriptorManager.cpp
    Private/DX12DescriptorManager.h
    Private/DX12Device.cpp
    Private/DX12Device.h
    Private/DX12Fence.cpp
    Private/DX12Fence.h
    Private/DX12MemoryAllocator.cpp
    Private/DX12MemoryAllocator.h
    Private/DX12QueryManager.cpp
    Private/DX12QueryManager.h
    Private/DX12QueueManager.cpp
    Private/DX12QueueManager.h
    Private/DX12ShaderCompiler.cpp
    Private/DX12ShaderCompiler.h
    Private/DX12Types.cpp
    Private/DX12Types.h
    Private/DX12UploadManager.cpp
    Private/DX12UploadManager.h
    Private/DX12Utility.h
    Private/GAPI_DX12.cpp
    Private/GAPI_DX12Buffer.cpp
    Private/GAPI_DX12CommandList.cpp
    Private/GAPI_DX12Fence.cpp
    Private/GAPI_DX12Pipeline.cpp
    Private/GAPI_DX12Resource.cpp
    Private/GAPI_DX12Sampler.cpp
    Private/GAPI_DX12Shader.cpp
    Private/GAPI_DX12ShaderParameter.cpp
    Private/GAPI_DX12SwapChain.cpp
    Private/GAPI_DX12Texture.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

# Target settings
add_library(CE-GAPI_DX12 SHARED
    ${GLOBAL_FILES}
    ${PUBLIC_FILES} ${PRIVATE_FILES}
)
target_include_directories(CE-GAPI_DX12
    PUBLIC
        Public
    PRIVATE
        Private
)
target_precompile_headers(CE-GAPI_DX12
    PRIVATE
        ${PRECOMPILE_HEADER_FILES}
)
target_link_libraries(CE-GAPI_DX12
    PRIVATE
        dxagility
        dxguid.lib
        D3D12MemoryAllocator
        Microsoft::DirectX-Headers
        dxc
        imgui
        winpixeventruntime
        CE-Base
        CE-Logger
        CE-Platform
        CE-GAPI
        CE-GAPI_Base
        CE-Core
)
target_compile_definitions(CE-GAPI_DX12
    PRIVATE
        CUBE_MODULE_GAPI_DX12
)

# Copy thied-party DLibs
add_custom_command(
    TARGET CE-GAPI_DX12 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:dxc_dxcompiler>
        $<TARGET_FILE:dxc_dxil>
        $<TARGET_FILE:winpixeventruntime>
        $<TARGET_FILE_DIR:CE-GAPI_DX12>
    COMMAND_EXPAND_LISTS
)
add_custom_command(
    TARGET CE-GAPI_DX12 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:CE-GAPI_DX12>/D3D12
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:dxagility_d3d12core>
        $<TARGET_FILE:dxagility_d3d12sdklayers>
        $<TARGET_FILE_DIR:dxagility_d3d12core>/D3D12Core.pdb
        $<TARGET_FILE_DIR:dxagility_d3d12sdklayers>/d3d12SDKLayers.pdb
        $<TARGET_FILE_DIR:CE-GAPI_DX12>/D3D12
    COMMAND_EXPAND_LISTS
)

list(APPEND G_ENABLED_GAPI_MODULES "CE-GAPI_DX12")
set(G_ENABLED_GAPI_MODULES "${G_ENABLED_GAPI_MODULES}" CACHE INTERNAL "g_enabled_gapi_modules")
