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
            VulkanFence() :
                mDeviceHandle(VK_NULL_HANDLE),
                mFence(VK_NULL_HANDLE)
            {}
            VulkanFence(VkDevice deviceHandle, VkFence fence) :
                mDeviceHandle(deviceHandle),
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
            WaitResult Wait(double timeInSec);
            void Reset();

        private:
            friend class VulkanFencePool;

            VkDevice mDeviceHandle;

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
