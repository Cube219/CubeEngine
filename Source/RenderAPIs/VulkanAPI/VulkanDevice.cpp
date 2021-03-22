#include "VulkanDevice.h"

#include "VulkanDebug.h"
#include "VulkanUtility.h"

#include "RenderAPIs/RenderAPI/RenderAPI.h"
#include "Core/Assertion.h"
#include "Core/Allocator/FrameAllocator.h"

namespace cube
{
    namespace rapi
    {
        VulkanDevice::VulkanDevice(const RenderAPICreateInfo& apiInfo, VkInstance instance, VkPhysicalDevice gpu) :
            mGPU(gpu),
            mDevice(nullptr),
            mCommandPoolManager(*this),
            mStagingManager(*this),
            mFencePool(*this),
            mSemaphorePool(*this),
            mShaderVariableManager(*this),
            mQueueManager(*this)
        {
            vkGetPhysicalDeviceProperties(mGPU, &mProps);
            switch(mProps.deviceType)
            {
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    mType = GPUType::Other;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    mType = GPUType::Integrated;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    mType = GPUType::Discrete;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    mType = GPUType::Virtual;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    mType = GPUType::CPU;
                    break;
                default:
                    mType = GPUType::Unknown;
                    break;
            }

            vkGetPhysicalDeviceFeatures(mGPU, &mFeatures);

            vkGetPhysicalDeviceMemoryProperties(mGPU, &mMemProps);

            CreateDevice();

            mQueueManager.Initialize(mGPU);
            mCommandPoolManager.Initialize(apiInfo.numGraphicsCommandPools, apiInfo.numTransferCommandPools, apiInfo.numComputeCommandPools);
            mAllocator.Initialize(instance, mGPU, mDevice);
            mStagingManager.Initialize();
            mFencePool.Initialize();
            mSemaphorePool.Initialize();
            mShaderVariableManager.Initialize();
        }

        VulkanDevice::~VulkanDevice()
        {
            mShaderVariableManager.Shutdown();
            mSemaphorePool.Shutdown();
            mFencePool.Shutdown();
            mStagingManager.Shutdown();
            mAllocator.Shutdown();
            mCommandPoolManager.Shutdown();
            mQueueManager.Shutdown();

            if(mDevice != nullptr) {
                vkDestroyDevice(mDevice, nullptr);
            }
        }

        void VulkanDevice::CreateDevice()
        {
            VkResult res;

            VkDeviceCreateInfo deviceCreateInfo;
            deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceCreateInfo.pNext = nullptr;
            deviceCreateInfo.flags = 0;

            Uint32 queueFamilyNum = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(mGPU, &queueFamilyNum, nullptr);
            CHECK(queueFamilyNum > 0, "Cannot find physical device queue family.");

            FrameVector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyNum);
            vkGetPhysicalDeviceQueueFamilyProperties(mGPU, &queueFamilyNum, queueFamilyProps.data());

            // Put all queue familiy of the physical device into device queue
            FrameVector<VkDeviceQueueCreateInfo> queueInfos(queueFamilyProps.size());
            int prioritiesNum = 0;
            for(Uint64 i = 0; i < queueFamilyProps.size(); i++) {
                VkDeviceQueueCreateInfo info;
                info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0;
                info.queueFamilyIndex = (Uint32)i;
                info.queueCount = queueFamilyProps[i].queueCount;
                prioritiesNum += info.queueCount;

                queueInfos[i] = info;
            }
            FrameVector<float> queuePriorities(prioritiesNum);
            auto priorityIter = queuePriorities.begin();
            for(Uint64 i = 0; i < queueInfos.size(); i++) {
                VkDeviceQueueCreateInfo& info = queueInfos[i];
                info.pQueuePriorities = &(*priorityIter);
                priorityIter += info.queueCount;
            }
            deviceCreateInfo.queueCreateInfoCount = SCast(Uint32)(queueInfos.size());
            deviceCreateInfo.pQueueCreateInfos = queueInfos.data();

            deviceCreateInfo.enabledLayerCount = 0; // Deprecated
            deviceCreateInfo.ppEnabledLayerNames = nullptr; // Deprecated

            FrameVector<const char*> extensions;
            extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            extensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
            extensions.push_back("VK_KHR_get_memory_requirements2");
            deviceCreateInfo.enabledExtensionCount = SCast(Uint32)(extensions.size());
            deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

            VkPhysicalDeviceFeatures features = {};
            deviceCreateInfo.pEnabledFeatures = &features;

            res = vkCreateDevice(mGPU, &deviceCreateInfo, nullptr, &mDevice);
            CHECK_VK(res, "Failed to create VkDevice.");
        }
    } // namespace rapi
} // namespace cube
