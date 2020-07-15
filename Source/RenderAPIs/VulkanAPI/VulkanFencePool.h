#pragma once

#include "VulkanAPIHeader.h"

#include "Core/Thread/MutexLock.h"

namespace cube
{
    namespace rapi
    {
        class VulkanFence
        {
        public:
            VulkanFence() : mFence(VK_NULL_HANDLE) {}
            VulkanFence(VkFence fence) :
                mFence(fence)
            {}
            ~VulkanFence() {}

            VkFence GetHandle() const { return mFence; }

            enum class WaitResult
            {
                Success,
                Timeout,
                Error
            };
            WaitResult Wait(VkDevice device, double timeInSec);
            void Reset(VkDevice device);

        private:
            friend class VulkanFencePool;

            VkFence mFence;
        };

        class VulkanFencePool
        {
        public:
            VulkanFencePool(VulkanDevice& device) :
                mDevice(device)
            {}
            ~VulkanFencePool() {}

            VulkanFencePool(const VulkanFencePool& other) = delete;
            VulkanFencePool& operator=(const VulkanFencePool& rhs) = delete;

            void Initialize();
            void Shutdown();

            VulkanFence AllocateFence(const char* debugName = nullptr);
            void FreeFence(VulkanFence& fence);

        private:
            VulkanDevice& mDevice;

            Mutex mFencesMutex;
            Vector<VulkanFence> mFreeFences;
            Vector<VulkanFence> mUsedFences;
        };
    } // namespace rapi
} // namespace cube
