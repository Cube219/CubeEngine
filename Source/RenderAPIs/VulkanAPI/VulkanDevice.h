#pragma once

#include "VulkanAPIHeader.h"

#include "VulkanMemoryAllocator.h"
#include "VulkanStagingManager.h"
#include "VulkanFencePool.h"
#include "VulkanSemaphorePool.h"
#include "VulkanQueueManager.h"
#include "Interface/DeviceContextVk.h"

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
            VulkanDevice(VkInstance instance, VkPhysicalDevice gpu);
            ~VulkanDevice();

            GPUType GetGPUType() const { return mType; }
            VkDevice GetHandle() const { return mDevice; }

            VulkanMemoryAllocator& GetAllocator() { return mAllocator; }
            VulkanStagingManager& GetStagingManager() { return mStagingManager; }
            VulkanFencePool& GetFencePool() { return mFencePool; }
            VulkanSemaphorePool& GetSemaphorePool() { return mSemaphorePool; }
            VulkanQueueManager& GetQueueManager() { return mQueueManager; }

            DeviceContextVk& GetImmediateContext() { return *mpImmediateContext; }
            VulkanCommandBuffer GetUploadCommandBuffer(const char* debugName = nullptr);
            void SubmitUploadCommandBuffer(VulkanCommandBuffer& cmdBuf);

        private:
            void CreateDevice();

            VkPhysicalDevice mGPU;
            VkPhysicalDeviceProperties mProps;
            VkPhysicalDeviceFeatures mFeatures;
            VkPhysicalDeviceMemoryProperties mMemProps;

            GPUType mType;

            VkDevice mDevice;

            VulkanMemoryAllocator mAllocator;
            VulkanStagingManager mStagingManager;
            VulkanFencePool mFencePool;
            VulkanSemaphorePool mSemaphorePool;
            VulkanQueueManager mQueueManager;

            DeviceContextVk* mpImmediateContext;
            VulkanCommandPool mUploadCommandPool;
            Vector<VulkanCommandBuffer> mUploadCommandBuffersToSubmit;
        };
    } // namespace rapi
} // namespace cube
