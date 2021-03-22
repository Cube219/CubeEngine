cmake_minimum_required (VERSION 3.6)

set(INCLUDE
    Interface/BufferVk.h
    Interface/CommandListVk.h
    Interface/FenceVk.h
    Interface/FramebufferVk.h
    Interface/PipelineStateVk.h
    Interface/RenderPassVk.h
    Interface/SamplerVk.h
    Interface/ShaderVk.h
    Interface/ShaderVariablesVk.h
    Interface/SwapChainVk.h
    Interface/TextureVk.h
    Interface/TextureViewVk.h
    Tools/GLSLTool.h
    VulkanAPI.h
    VulkanAPIFwd.h
    VulkanAPIHeader.h
    VulkanCommandPoolManager.h
    VulkanDebug.h
    VulkanDevice.h
    VulkanFencePool.h
    VulkanLayout.h
    VulkanMemoryAllocator.h
    VulkanQueueManager.h
    VulkanSemaphorePool.h
    VulkanShaderVariableManager.h
    VulkanStagingManager.h
    VulkanTypeConversion.h
    VulkanUtility.h
)
set(SOURCE
    Interface/BufferVk.cpp
    Interface/CommandListVk.cpp
    Interface/FenceVk.cpp
    Interface/FramebufferVk.cpp
    Interface/PipelineStateVk.cpp
    Interface/RenderPassVk.cpp
    Interface/SamplerVk.cpp
    Interface/ShaderVk.cpp
    Interface/ShaderVariablesVk.cpp
    Interface/SwapChainVk.cpp
    Interface/TextureVk.cpp
    Interface/TextureViewVk.cpp
    Tools/GLSLTool.cpp
    VulkanAPI.cpp
    VulkanCommandPoolManager.cpp
    VulkanDebug.cpp
    VulkanDevice.cpp
    VulkanFencePool.cpp
    VulkanLayout.cpp
    VulkanMemoryAllocator.cpp
    VulkanQueueManager.cpp
    VulkanSemaphorePool.cpp
    VulkanShaderVariableManager.cpp
    VulkanStagingManager.cpp
    VulkanTypeConversion.cpp
    VulkanUtility.cpp
)

# Target
add_library(CE-VulkanAPI SHARED ${INCLUDE} ${SOURCE})

# Defines
target_compile_definitions(CE-VulkanAPI PRIVATE VK_API_EXPORTS)

# Internal libs
target_link_libraries(CE-VulkanAPI PUBLIC CE-Utility CE-Platform CE-Core)

# External includes
target_include_directories(CE-VulkanAPI PRIVATE $ENV{VULKAN_SDK}/Include ${DEPENDENCIES_DIR}/VulkanMemoryAllocator/src)

# Platform-dependent
if(PLATFORM_WIN32)
    # Vulkan lib
    target_compile_definitions(CE-VulkanAPI PRIVATE VK_USE_PLATFORM_WIN32_KHR)
    target_link_libraries(CE-VulkanAPI PRIVATE $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)

    # GLSL libs (Debug)
    target_link_libraries(CE-VulkanAPI PRIVATE debug ${DEPENDENCIES_DIR}/glslang/lib/Win64/glslangd.lib)
    target_link_libraries(CE-VulkanAPI PRIVATE debug ${DEPENDENCIES_DIR}/glslang/lib/Win64/SPIRVd.lib)
    target_link_libraries(CE-VulkanAPI PRIVATE debug ${DEPENDENCIES_DIR}/glslang/lib/Win64/OSDependentd.lib)
    target_link_libraries(CE-VulkanAPI PRIVATE debug ${DEPENDENCIES_DIR}/glslang/lib/Win64/OGLCompilerd.lib)
    # GLSL libs (Release)
    target_link_libraries(CE-VulkanAPI PRIVATE optimized $ENV{VULKAN_SDK}/Lib/shaderc_combined.lib)
endif()
