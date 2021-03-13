cmake_minimum_required (VERSION 3.6)

set(VULKAN_SUPPORTED FALSE)

option(RENDERAPI_ENABLE_VULKAN "Enable VulkanAPI" ON)

# Check if each api is supported
if(PLATFORM_WIN32)
    if(DEFINED ENV{VULKAN_SDK})
        set(VULKAN_SUPPORTED TRUE)
    else()
        message(WARNING "VulkanSDK is not installed.")
    endif()
endif()

message("VULKAN_SUPPORTED: " ${VULKAN_SUPPORTED})

add_subdirectory("RenderAPI")
if(RENDERAPI_ENABLE_VULKAN AND VULKAN_SUPPORTED)
    add_subdirectory("VulkanAPI")
endif()
