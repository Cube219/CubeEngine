#pragma once

#include "VulkanAPIHeader.h"

#include "Core/Thread/MutexLock.h"

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

            void Initialize(VkPhysicalDevice gpu);
            void Shutdown();

        private:
            bool InitGraphicsQueue(VkQueueFamilyProperties* pProps, Uint64 propNum);
            bool InitComputeQueue(VkQueueFamilyProperties* pProps, Uint64 propNum);
            bool InitTransferQueue(VkQueueFamilyProperties* pProps, Uint64 propNum);
            Uint32 FindQueueFamilyIndex(VkQueueFamilyProperties* pProps, Uint64 propNum, VkQueueFlags flags, VkQueueFlags prohibitFlags);

            VulkanDevice& mDevice;

            VulkanQueue mGraphicsQueue;
            Vector<VulkanQueue> mComputeQueues;
            Uint64 mComputeQueuesCurrentIndex;
            Vector<VulkanQueue> mTransferQueues;
            Uint64 mTransferQueuesCurrentIndex;
            // VulkanQueue mPresentQueue; // TODO: Implement
        };
    } // namespace rapi
} // namespace cube
