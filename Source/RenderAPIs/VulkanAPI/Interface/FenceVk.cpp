#include "FenceVk.h"

#include "../VulkanDevice.h"
#include "../VulkanFencePool.h"

namespace cube
{
    namespace rapi
    {
        FenceVk::FenceVk(VulkanDevice& device, VulkanFencePool& allocatedPool, VkFence fence) :
            mDevice(device),
            mAllocatedPool(allocatedPool),
            mFence(fence)
        {
        }

        FenceVk::~FenceVk()
        {
            mAllocatedPool.FreeFence(*this);
        }

        void FenceVk::Release()
        {
            mAllocatedPool.FreeFence(*this);
        }

        FenceWaitResult FenceVk::Wait(float timeout)
        {
            // Null fence
            if(mFence == VK_NULL_HANDLE) {
                return FenceWaitResult::Success;
            }

            VkResult res;

            Uint64 nanoSecond = (Uint64)((double)timeout * 1000.0 * 1000.0 * 1000.0);

            res = vkWaitForFences(mDevice.GetHandle(), 1, &mFence, VK_TRUE, nanoSecond);

            switch(res) {
                case VK_SUCCESS: return FenceWaitResult::Success;
                case VK_TIMEOUT: return FenceWaitResult::Timeout;
                default:         return FenceWaitResult::Error;
            }
        }
    } // namespace rapi
} // namespace cube
