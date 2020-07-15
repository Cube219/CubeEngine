#pragma once

#include "VulkanAPIHeader.h"

#include "Core/Thread/MutexLock.h"
#include "VulkanFencePool.h"
#include "VulkanSemaphorePool.h"

namespace cube
{
    namespace rapi
    {
        struct VulkanQueue
        {
            VulkanQueue() :
                queue(VK_NULL_HANDLE), familyIndex(Uint32InvalidValue), queueIndex(Uint32InvalidValue)
            {}

            VkQueue queue;
            Uint32 familyIndex;
            Uint32 queueIndex;
        };

        class VulkanQueueManager
        {
        public:
            VulkanQueueManager(VulkanDevice& device) : mDevice(device)
            {}
            ~VulkanQueueManager() {}

            VulkanQueueManager(const VulkanQueueManager& other) = delete;
            VulkanQueueManager& operator=(const VulkanQueueManager& rhs) = delete;

            void Initialize(VkPhysicalDevice gpu);
            void Shutdown();

            Uint32 GetGraphicsQueueFamilyIndex() const { return mGraphicsQueue.familyIndex; }
            Uint32 GetComputeQueueFamilyIndex() const { return mComputeQueues[0].familyIndex; }
            Uint32 GetTransferQueueFamilyIndex() const { return mTransferQueues[0].familyIndex; }

            void SubmitCommandBuffer(VulkanCommandBuffer& commandBuffer);

        private:
            bool InitGraphicsQueue(VkQueueFamilyProperties* pProps, Uint64 propNum);
            bool InitComputeQueue(VkQueueFamilyProperties* pProps, Uint64 propNum);
            bool InitTransferQueue(VkQueueFamilyProperties* pProps, Uint64 propNum);
            Uint32 FindQueueFamilyIndex(VkQueueFamilyProperties* pProps, Uint64 propNum, VkQueueFlags flags, VkQueueFlags prohibitFlags);

            void SubmitGraphicsQueue(VulkanCommandBuffer& commandBuffer);
            void SubmitComputeQueue(VulkanCommandBuffer& commandBuffer);
            void SubmitTransferQueue(VulkanCommandBuffer& commandBuffer);

            VulkanDevice& mDevice;

            VulkanQueue mGraphicsQueue;
            VulkanFence mGraphicsQueueFence;
            Mutex mGraphicsWaitSemaphoresMutex;
            Vector<VulkanSemaphore> mGraphicsWaitSemaphores;

            Vector<VulkanQueue> mComputeQueues;
            Uint64 mComputeQueuesCurrentIndex;
            Mutex mComputeQueuesMutex;

            Vector<VulkanQueue> mTransferQueues;
            Uint64 mTransferQueuesCurrentIndex;
            Mutex mTransferQueuesMutex;

            // VulkanQueue mPresentQueue; // TODO: Implement
        };
    } // namespace rapi
} // namespace cube
