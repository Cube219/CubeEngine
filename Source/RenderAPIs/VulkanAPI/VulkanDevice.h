#pragma once

#include "VulkanAPIHeader.h"

#include "VulkanCommandPoolManager.h"
#include "VulkanMemoryAllocator.h"
#include "VulkanStagingManager.h"
#include "VulkanFencePool.h"
#include "VulkanSemaphorePool.h"
#include "VulkanShaderVariableManager.h"
#include "VulkanQueueManager.h"

namespace cube
{
    namespace rapi
    {
        enum class GPUType
        {
            Discrete,
            Integrated,
            Virtual,
            CPU,
            Other,
            Unknown
        };

        class VulkanDevice
        {
        public:
            VulkanDevice(const RenderAPICreateInfo& apiInfo, VkInstance instance, VkPhysicalDevice gpu);
            ~VulkanDevice();

            GPUType GetGPUType() const { return mType; }
            VkDevice GetHandle() const { return mDevice; }
            VkPhysicalDevice GetGPU() const { return mGPU; }
            const VkPhysicalDeviceProperties& GetProperties() const { return mProps; }

            VulkanCommandPoolManager& GetCommandPoolManager() { return mCommandPoolManager; }
            VulkanMemoryAllocator& GetAllocator() { return mAllocator; }
            VulkanStagingManager& GetStagingManager() { return mStagingManager; }
            VulkanFencePool& GetFencePool() { return mFencePool; }
            VulkanSemaphorePool& GetSemaphorePool() { return mSemaphorePool; }
            VulkanShaderVariableManager& GetShaderVariableManager() { return mShaderVariableManager; }
            VulkanQueueManager& GetQueueManager() { return mQueueManager; }

        private:
            void CreateDevice();

            VkPhysicalDevice mGPU;
            VkPhysicalDeviceProperties mProps;
            VkPhysicalDeviceFeatures mFeatures;
            VkPhysicalDeviceMemoryProperties mMemProps;

            GPUType mType;

            VkDevice mDevice;

            VulkanCommandPoolManager mCommandPoolManager;
            VulkanMemoryAllocator mAllocator;
            VulkanStagingManager mStagingManager;
            VulkanFencePool mFencePool;
            VulkanSemaphorePool mSemaphorePool;
            VulkanShaderVariableManager mShaderVariableManager;
            VulkanQueueManager mQueueManager;
        };
    } // namespace rapi
} // namespace cube
