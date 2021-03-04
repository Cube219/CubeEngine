#pragma once

#include "VulkanAPIHeader.h"

#include "Core/Thread/MutexLock.h"

namespace cube
{
    namespace rapi
    {
        class VulkanFencePool
        {
        public:
            VulkanFencePool(VulkanDevice& device) :
                mDevice(device)
            {}
            ~VulkanFencePool() = default;

            VulkanFencePool(const VulkanFencePool& other) = delete;
            VulkanFencePool& operator=(const VulkanFencePool& rhs) = delete;

            void Initialize();
            void Shutdown();

            SPtr<FenceVk> AllocateFence(const char* debugName = nullptr);
            void FreeFence(FenceVk& fence);

            SPtr<FenceVk> CreateNullFence();

        private:
            VulkanDevice& mDevice;

            Mutex mFencesMutex;
            Vector<VkFence> mFreeFences;
        };
    } // namespace rapi
} // namespace cube
