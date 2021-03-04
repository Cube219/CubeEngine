#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Fence.h"

namespace cube
{
    namespace rapi
    {
        class FenceVk : public Fence
        {
        public:
            FenceVk(VulkanDevice& device, VulkanFencePool& allocatedPool, VkFence fence);
            virtual ~FenceVk();

            virtual void Release() override;
            virtual FenceWaitResult Wait(float timeout) override;

            VkFence GetHandle() const { return mFence; }

        private:
            friend class VulkanFencePool;

            VulkanDevice& mDevice;
            VulkanFencePool& mAllocatedPool;

            VkFence mFence;
        };
    } // namespace rapi
} // namespace cube
