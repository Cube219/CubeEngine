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

            // VulkanSemaphore(const VulkanSemaphore& other) = delete;
            // VulkanSemaphore& operator=(const VulkanSemaphore& rhs) = delete;
            //
            // VulkanSemaphore(VulkanSemaphore&& other) noexcept
            // {
            //     mSemaphore = other.mSemaphore;
            //
            //     other.mSemaphore = VK_NULL_HANDLE;
            // }
            // VulkanSemaphore& operator=(VulkanSemaphore&& rhs) noexcept
            // {
            //     if(this == &rhs) return *this;
            //
            //     mSemaphore = rhs.mSemaphore;
            //
            //     rhs.mSemaphore = VK_NULL_HANDLE;
            //
            //     return *this;
            // }

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

            VulkanSemaphorePool(const VulkanSemaphorePool& other) = delete;
            VulkanSemaphorePool& operator=(const VulkanSemaphorePool& rhs) = delete;

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
