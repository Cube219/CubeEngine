#pragma once

// Only can be used in debug mode
#ifdef _DEBUG

#include "VulkanAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        template <typename T>
        constexpr VkObjectType GetVkObjectType() {
            static_assert(false, "This type is not a VkObjectType.");
        }

        class VulkanDebug
        {
        public:
            VulkanDebug() = delete;
            ~VulkanDebug() = delete;

            static void Initialize(VkInstance instance);
            static void Shutdown();

            template <typename T>
            static void SetObjectName(VkDevice device, T vkObject, const char* name)
            {
                SetObjectNameImpl(device, GetVkObjectType<T>(), (Uint64)vkObject, name);
            }

            static void BeginQueueLabel(VkQueue queue, const char* name);
            static void EndQueueLabel(VkQueue queue);
            static void InsertQueueLabel(VkQueue queue, const char* name);

            static void BeginCommandBufferLabel(VkCommandBuffer commandBuf, const char* name);
            static void EndCommandBufferLabel(VkCommandBuffer commandBuf);
            static void InsertCommandBufferLabel(VkCommandBuffer commandBuf, const char* name);

        private:
            static VkBool32 DebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severities,
                VkDebugUtilsMessageTypeFlagsEXT types,
                const VkDebugUtilsMessengerCallbackDataEXT* pData,
                void* pUserData);

            static void SetObjectNameImpl(VkDevice device, VkObjectType type, Uint64 handle, const char* name);

            static const char* GetVkObjectTypeStr(VkObjectType type);

            static bool mIsEnabled;

            static VkInstance mInstance;
            static VkDebugUtilsMessengerEXT mDebugMessenger;

            // Function pointers
            static PFN_vkSetDebugUtilsObjectNameEXT setDebugUtilsObjectNameEXT;

            static PFN_vkQueueBeginDebugUtilsLabelEXT queueBeginDebugUtilsLabelEXT;
            static PFN_vkQueueEndDebugUtilsLabelEXT queueEndDebugUtilsLabelEXT;
            static PFN_vkQueueInsertDebugUtilsLabelEXT queueInsertDebugUtilsLabelEXT;

            static PFN_vkCmdBeginDebugUtilsLabelEXT cmdBeginDebugUtilsLabelEXT;
            static PFN_vkCmdEndDebugUtilsLabelEXT cmdEndDebugUtilsLabelEXT;
            static PFN_vkCmdInsertDebugUtilsLabelEXT cmdInsertDebugUtilsLabelEXT;
        };

        // Template specializations
        template <>
        constexpr VkObjectType GetVkObjectType<VkInstance>() { return VK_OBJECT_TYPE_INSTANCE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkPhysicalDevice>() { return VK_OBJECT_TYPE_PHYSICAL_DEVICE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDevice>() { return VK_OBJECT_TYPE_DEVICE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkQueue>() { return VK_OBJECT_TYPE_QUEUE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkSemaphore>() { return VK_OBJECT_TYPE_SEMAPHORE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkCommandBuffer>() { return VK_OBJECT_TYPE_COMMAND_BUFFER; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkFence>() { return VK_OBJECT_TYPE_FENCE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDeviceMemory>() { return VK_OBJECT_TYPE_DEVICE_MEMORY; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkBuffer>() { return VK_OBJECT_TYPE_BUFFER; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkImage>() { return VK_OBJECT_TYPE_IMAGE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkEvent>() { return VK_OBJECT_TYPE_EVENT; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkQueryPool>() { return VK_OBJECT_TYPE_QUERY_POOL; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkBufferView>() { return VK_OBJECT_TYPE_BUFFER_VIEW; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkImageView>() { return VK_OBJECT_TYPE_IMAGE_VIEW; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkShaderModule>() { return VK_OBJECT_TYPE_SHADER_MODULE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkPipelineCache>() { return VK_OBJECT_TYPE_PIPELINE_CACHE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkPipelineLayout>() { return VK_OBJECT_TYPE_PIPELINE_LAYOUT; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkRenderPass>() { return VK_OBJECT_TYPE_RENDER_PASS; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkPipeline>() { return VK_OBJECT_TYPE_PIPELINE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDescriptorSetLayout>() { return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkSampler>() { return VK_OBJECT_TYPE_SAMPLER; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDescriptorPool>() { return VK_OBJECT_TYPE_DESCRIPTOR_POOL; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDescriptorSet>() { return VK_OBJECT_TYPE_DESCRIPTOR_SET; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkFramebuffer>() { return VK_OBJECT_TYPE_FRAMEBUFFER; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkCommandPool>() { return VK_OBJECT_TYPE_COMMAND_POOL; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkSamplerYcbcrConversion>() { return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDescriptorUpdateTemplate>() { return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkSurfaceKHR>() { return VK_OBJECT_TYPE_SURFACE_KHR; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkSwapchainKHR>() { return VK_OBJECT_TYPE_SWAPCHAIN_KHR; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDisplayKHR>() { return VK_OBJECT_TYPE_DISPLAY_KHR; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDisplayModeKHR>() { return VK_OBJECT_TYPE_DISPLAY_MODE_KHR; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDebugReportCallbackEXT>() { return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkDebugUtilsMessengerEXT>() { return VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT; }
        template <>
        constexpr VkObjectType GetVkObjectType<VkValidationCacheEXT>() { return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; }
    } // namespace rapi
} // namespace cube

#endif // _DEBUG

#ifdef _DEBUG

#define VULKAN_DEBUG_INIT(instance) cube::rapi::VulkanDebug::Initialize(instance)
#define VULKAN_DEBUG_SHUTDOWN() cube::rapi::VulkanDebug::Shutdown()

#define VULKAN_SET_OBJ_NAME(device, vkObject, name) cube::rapi::VulkanDebug::SetObjectName(device, vkObject, name)

#define VULKAN_BEGIN_QUEUE_LABEL(queue, name) cube::rapi::VulkanDebug::BeginQueueLabel(queue, name)
#define VULKAN_END_QUEUE_LABEL(queue) cube::rapi::VulkanDebug::EndQueueLabel(queue)
#define VULKAN_INSERT_QUEUE_LABEL(queue, name) cube::rapi::VulkanDebug::InsertQueueLabel(queue, name)

#define VULKAN_BEGIN_CMDBUF_LABEL(cmdBuf, name) cube::rapi::VulkanDebug::BeginCommandBufferLabel(cmdBuf, name)
#define VULKAN_END_CMDBUF_LABEL(cmdBuf) cube::rapi::VulkanDebug::EndCommandBufferLabel(cmdBuf)
#define VULKAN_INSERT_CMDBUF_LABEL(cmdBuf, name) cube::rapi::VulkanDebug::InsertCommandBufferLabel(cmdBuf, name)

#else // _DEBUG

#define VULKAN_DEBUG_INIT(instance)
#define VULKAN_DEBUG_SHUTDOWN()

#define VULKAN_SET_OBJ_NAME(device, vkObject, name)

#define VULKAN_BEGIN_QUEUE_LABEL(queue, name)
#define VULKAN_END_QUEUE_LABEL(queue)
#define VULKAN_INSERT_QUEUE_LABEL(queue, name)

#define VULKAN_BEGIN_CMDBUF_LABEL(cmdBuf, name)
#define VULKAN_END_CMDBUF_LABEL(cmdBuf)
#define VULKAN_INSERT_CMDBUF_LABEL(cmdBuf, name)

#endif // _DEBUG