#pragma once

#include "VulkanAPIHeader.h"

#include "Core/Thread/MutexLock.h"

namespace cube
{
    namespace rapi
    {
        class VulkanSemaphore
        {
        public:
            VulkanSemaphore() :
                mSemaphore(VK_NULL_HANDLE)
            {}
            VulkanSemaphore(VkSemaphore semaphore) :
                mSemaphore(mSemaphore)
            {}
            ~VulkanSemaphore() {}

            VkSemaphore GetHandle() const { return mSemaphore; }

        private:
            friend class VulkanSemaphorePool;

            VkSemaphore mSemaphore;
        };

        class VulkanSemaphorePool
        {
        public:
            VulkanSemaphorePool(VulkanDevice& device) :
                mDevice(device)
            {}
            ~VulkanSemaphorePool() {}

            void Initialize();
            void Shutdown();

            VulkanSemaphore AllocateSemaphore(const char* debugName = "");
            void FreeSemaphore(VulkanSemaphore& semaphore);

        private:
            VulkanDevice& mDevice;

            Mutex mSemaphoresMutex;
            Vector<VulkanSemaphore> mFreeSemaphores;
            Vector<VulkanSemaphore> mUsedSemaphores;
        };
    } // namespace rapi
} // namespace cube
