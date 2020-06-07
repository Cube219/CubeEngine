#ifdef _DEBUG

#include "VulkanDebug.h"

#include "VulkanUtility.h"
#include "Core/Allocator/FrameAllocator.h"
#include "Platform/PlatformDebug.h"
#include <xmemory>

namespace cube
{
    namespace rapi
    {
        bool  VulkanDebug::mIsEnabled = false;

        VkInstance VulkanDebug::mInstance;
        VkDebugUtilsMessengerEXT VulkanDebug::mDebugMessenger;

        PFN_vkSetDebugUtilsObjectNameEXT VulkanDebug::setDebugUtilsObjectNameEXT;

        PFN_vkQueueBeginDebugUtilsLabelEXT VulkanDebug::queueBeginDebugUtilsLabelEXT;
        PFN_vkQueueEndDebugUtilsLabelEXT VulkanDebug::queueEndDebugUtilsLabelEXT;
        PFN_vkQueueInsertDebugUtilsLabelEXT VulkanDebug::queueInsertDebugUtilsLabelEXT;

        PFN_vkCmdBeginDebugUtilsLabelEXT VulkanDebug::cmdBeginDebugUtilsLabelEXT;
        PFN_vkCmdEndDebugUtilsLabelEXT VulkanDebug::cmdEndDebugUtilsLabelEXT;
        PFN_vkCmdInsertDebugUtilsLabelEXT VulkanDebug::cmdInsertDebugUtilsLabelEXT;

        void VulkanDebug::Initialize(VkInstance instance)
        {
            VkResult res;

            mInstance = instance;

            PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT = RCast(PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            VkDebugUtilsMessengerCreateInfoEXT info;
            info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            info.pNext = nullptr;
            info.flags = 0;
            info.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            info.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            info.pfnUserCallback = VulkanDebug::DebugMessageCallback;
            info.pUserData = nullptr;

            res = createDebugUtilsMessengerEXT(instance, &info, nullptr, &mDebugMessenger);
            CHECK_VK(res, "Failed to create debug messenger.");

            mIsEnabled = true;
        }

        void VulkanDebug::Shutdown()
        {
            if(mIsEnabled == false)
                return;

            PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT = RCast(PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT"));
            destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);

            mIsEnabled = false;
        }

        void VulkanDebug::BeginQueueLabel(VkQueue queue, const char* name)
        {
            if(mIsEnabled == false || name == nullptr)
                return;

            VkDebugUtilsLabelEXT label;
            label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            label.pNext = nullptr;
            label.pLabelName = name;
            label.color[0] = 1.0f;
            label.color[1] = 1.0f;
            label.color[2] = 1.0f;
            label.color[3] = 1.0f;

            queueBeginDebugUtilsLabelEXT(queue, &label);
        }

        void VulkanDebug::EndQueueLabel(VkQueue queue)
        {
            if(mIsEnabled == false)
                return;

            queueEndDebugUtilsLabelEXT(queue);
        }

        void VulkanDebug::InsertQueueLabel(VkQueue queue, const char* name)
        {
            if(mIsEnabled == false || name == nullptr)
                return;

            VkDebugUtilsLabelEXT label;
            label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            label.pNext = nullptr;
            label.pLabelName = name;
            label.color[0] = 1.0f;
            label.color[1] = 1.0f;
            label.color[2] = 1.0f;
            label.color[3] = 1.0f;

            queueInsertDebugUtilsLabelEXT(queue, &label);
        }

        void VulkanDebug::BeginCommandBufferLabel(VkCommandBuffer commandBuf, const char* name)
        {
            if(mIsEnabled == false || name == nullptr)
                return;

            VkDebugUtilsLabelEXT label;
            label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            label.pNext = nullptr;
            label.pLabelName = name;
            label.color[0] = 1.0f;
            label.color[1] = 1.0f;
            label.color[2] = 1.0f;
            label.color[3] = 1.0f;

            cmdBeginDebugUtilsLabelEXT(commandBuf, &label);
        }

        void VulkanDebug::EndCommandBufferLabel(VkCommandBuffer commandBuf)
        {
            if(mIsEnabled == false)
                return;

            cmdEndDebugUtilsLabelEXT(commandBuf);
        }

        void VulkanDebug::InsertCommandBufferLabel(VkCommandBuffer commandBuf, const char* name)
        {
            if(mIsEnabled == false || name == nullptr)
                return;

            VkDebugUtilsLabelEXT label;
            label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            label.pNext = nullptr;
            label.pLabelName = name;
            label.color[0] = 1.0f;
            label.color[1] = 1.0f;
            label.color[2] = 1.0f;
            label.color[3] = 1.0f;

            cmdInsertDebugUtilsLabelEXT(commandBuf, &label);
        }

        VkBool32 VulkanDebug::DebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severities, VkDebugUtilsMessageTypeFlagsEXT types,
            const VkDebugUtilsMessengerCallbackDataEXT* pData, void* pUserData)
        {
            bool isError = false;

            // ERROR [18:22:22.1111 / VulkanDebug:VALIDATION(IDName, Code IDNum)] : Message
            //     Objects - 1
            //         Object[0] - VkObject, Handle 0x123, Name "Test"
            //     Queue Labels - 1
            //         Label[0] - Test
            //     Command Buffer Labels - 1
            //         Label[0] - Test2
            const char* prefix = "";

            switch(severities) {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                    prefix = " VERBOSE";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                    prefix = "SEVERITY";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                    prefix = " WARNING";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                    prefix = "   ERROR";
                    isError = true;
                    break;
            }

            const char* typeStr = "";
            if(types & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
                typeStr = "GENERAL";
            } else {
                if(types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
                    typeStr = "VALIDATION";

                    if(types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
                        typeStr = "VALIDATION|PERFORMANCE";
                    }
                }
            }

            FrameString res;
            if(!pData->pMessageIdName) {
                res = FrameFormat(CUBE_T("{} [ / VulkanDebug:{}(Code {:#x})] : {}"), prefix, typeStr, pData->messageIdNumber, pData->pMessage);
            } else {
                res = FrameFormat(CUBE_T("{} [ / VulkanDebug:{}({}, Code {:#x})] : {}"), prefix, typeStr, pData->pMessageIdName, pData->messageIdNumber, pData->pMessage);
            }

            if(pData->objectCount > 0) {
                res += FrameFormat(CUBE_T("\n\tObjects - {}"), pData->objectCount);
                for(Uint32 i = 0; i < pData->objectCount; i++) {
                    VkDebugUtilsObjectNameInfoEXT obj = pData->pObjects[i];

                    if(obj.pObjectName == nullptr) {
                        res += FrameFormat(CUBE_T("\n\t\tObject[{}] - {}, Handle {:#x}"), i, GetVkObjectTypeStr(obj.objectType), obj.objectHandle);
                    } else {
                        res += FrameFormat(CUBE_T("\n\t\tObject[{}] - {}, Handle {:#x}, Name {}"), i, GetVkObjectTypeStr(obj.objectType), obj.objectHandle, obj.pObjectName);
                    }
                    
                }
            }

            if(pData->queueLabelCount > 0) {
                res += FrameFormat(CUBE_T("\n\tQueue Labels - {}"), pData->queueLabelCount);
                for(Uint32 i = 0; i < pData->queueLabelCount; i++) {
                    VkDebugUtilsLabelEXT label = pData->pQueueLabels[i];
                    res += FrameFormat(CUBE_T("\n\t\tLabel[{}] - {}"), i, label.pLabelName);
                }
            }

            if(pData->cmdBufLabelCount > 0) {
                res += FrameFormat(CUBE_T("\n\tCommand Buffer Labels - {}"), pData->cmdBufLabelCount);
                for(Uint32 i = 0; i < pData->queueLabelCount; i++) {
                    VkDebugUtilsLabelEXT label = pData->pCmdBufLabels[i];
                    res += FrameFormat(CUBE_T("\n\t\tLabel[{}] - {}"), i, label.pLabelName);
                }
            }
            platform::PlatformDebug::PrintToConsole(res);
            if(isError) {
                platform::PlatformDebug::AssertionFailed(res, __FUNCTION__, __FILE__, __LINE__);
            }

            return VK_FALSE;
        }

        void VulkanDebug::SetObjectNameImpl(VkDevice device, VkObjectType type, Uint64 handle, const char* name)
        {
            if(mIsEnabled == false || name == nullptr)
                return;

            VkResult res;

            VkDebugUtilsObjectNameInfoEXT info = {};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            info.pNext = nullptr;
            info.objectType = type;
            info.objectHandle = handle;
            info.pObjectName = name;

            res = setDebugUtilsObjectNameEXT(device, &info);
            CHECK_VK(res, "Failed to set object name.");
        }

        const char* VulkanDebug::GetVkObjectTypeStr(VkObjectType type)
        {
            switch(type) {
                case VK_OBJECT_TYPE_INSTANCE:                   return "VkInstance";
                case VK_OBJECT_TYPE_PHYSICAL_DEVICE:            return "VkPhysicalDevice";
                case VK_OBJECT_TYPE_DEVICE:                     return "VkDevice";
                case VK_OBJECT_TYPE_QUEUE:                      return "VkQueue";
                case VK_OBJECT_TYPE_SEMAPHORE:                  return "VkSemaphore";
                case VK_OBJECT_TYPE_COMMAND_BUFFER:             return "VkCommandBuffer";
                case VK_OBJECT_TYPE_FENCE:                      return "VkFence";
                case VK_OBJECT_TYPE_DEVICE_MEMORY:              return "VkDeviceMemory";
                case VK_OBJECT_TYPE_BUFFER:                     return "VkBuffer";
                case VK_OBJECT_TYPE_IMAGE:                      return "VkImage";
                case VK_OBJECT_TYPE_EVENT:                      return "VkEvent";
                case VK_OBJECT_TYPE_QUERY_POOL:                 return "VkQueryPool";
                case VK_OBJECT_TYPE_BUFFER_VIEW:                return "VkBufferView";
                case VK_OBJECT_TYPE_IMAGE_VIEW:                 return "VkImageView";
                case VK_OBJECT_TYPE_SHADER_MODULE:              return "VkShaderModule";
                case VK_OBJECT_TYPE_PIPELINE_CACHE:             return "VkPipelineCache";
                case VK_OBJECT_TYPE_PIPELINE_LAYOUT:            return "VkPipelineLayout";
                case VK_OBJECT_TYPE_RENDER_PASS:                return "VkRenderPass";
                case VK_OBJECT_TYPE_PIPELINE:                   return "VkPipeline";
                case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:      return "VkDescriptorSetLayout";
                case VK_OBJECT_TYPE_SAMPLER:                    return "VkSampler";
                case VK_OBJECT_TYPE_DESCRIPTOR_POOL:            return "VkDescriptorPool";
                case VK_OBJECT_TYPE_DESCRIPTOR_SET:             return "VkDescriptorSet";
                case VK_OBJECT_TYPE_FRAMEBUFFER:                return "VkFramebuffer";
                case VK_OBJECT_TYPE_COMMAND_POOL:               return "VkCommandPool";
                case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:   return "VkSamplerYcbcrConversion";
                case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: return "VkDescriptorUpdateTemplate";
                case VK_OBJECT_TYPE_SURFACE_KHR:                return "VkSurfaceKHR";
                case VK_OBJECT_TYPE_SWAPCHAIN_KHR:              return "VkSwapchainKHR";
                case VK_OBJECT_TYPE_DISPLAY_KHR:                return "VkDisplayKHR";
                case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:           return "VkDisplayModeKHR";
                case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:  return "VkDebugReportCallbackEXT";
                case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:  return "VkDebugUtilsMessengerEXT";
                case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:       return "VkValidationCacheEXT";
                default:                                        return "Unknown";
            }
            return "Unknown";
        }
    } // namespace rapi
} // namespace cube

#endif // _DEBUG
