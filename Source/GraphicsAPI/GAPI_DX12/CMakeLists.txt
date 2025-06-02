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
    Public/GAPI_DX12Shader.h
    Public/GAPI_DX12ShaderVariable.h
    Public/GAPI_DX12Viewport.h
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
    Private/DX12Utility.h
    Private/GAPI_DX12.cpp
    Private/GAPI_DX12Buffer.cpp
    Private/GAPI_DX12CommandList.cpp
    Private/GAPI_DX12Fence.cpp
    Private/GAPI_DX12Pipeline.cpp
    Private/GAPI_DX12Resource.cpp
    Private/GAPI_DX12Shader.cpp
    Private/GAPI_DX12ShaderVariable.cpp
    Private/GAPI_DX12Viewport.cpp
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
target_link_libraries(CE-GAPI_DX12
    PRIVATE
        d3d12.lib
        dxgi.lib
        dxguid.lib
        D3D12MemoryAllocator
        imgui
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
        $<TARGET_FILE:dxc>
        $<TARGET_FILE_DIR:CE-GAPI_DX12>
    COMMAND_EXPAND_LISTS
)

list(APPEND G_ENABLED_GAPI_MODULES "CE-GAPI_DX12")
set(G_ENABLED_GAPI_MODULES "${G_ENABLED_GAPI_MODULES}" CACHE INTERNAL "g_enabled_gapi_modules")
