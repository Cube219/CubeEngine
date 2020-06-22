#include "VulkanFencePool.h"

#include "VulkanDevice.h"
#include "VulkanUtility.h"
#include "VulkanDebug.h"
#include "Core/Assertion.h"

namespace cube
{
    namespace rapi
    {
        VulkanFence::WaitResult VulkanFence::Wait(VkDevice device, double timeInSec)
        {
            CHECK(mFence != VK_NULL_HANDLE, "The fence is already released.");

            Uint64 nanoSecond = (Uint64)(timeInSec * 1000.0 * 1000.0 * 1000.0);
            VkResult res = vkWaitForFences(device, 1, &mFence, VK_TRUE, nanoSecond);

            switch(res) {
                case VK_SUCCESS: return WaitResult::Success;
                case VK_TIMEOUT: return WaitResult::Timeout;
                default:         return WaitResult::Error;
            }
        }

        void VulkanFence::Reset(VkDevice device)
        {
            CHECK(mFence != VK_NULL_HANDLE, "The fence is already released.");

            VkResult res = vkResetFences(device, 1, &mFence);
            CHECK_VK(res, "Failed to reset the fence.");
        }

        void VulkanFencePool::Initialize()
        {
            VkResult res;
            VkFence fence;

            // Create 10 fences
            constexpr Uint32 initSize = 10;
            mFreeFences.resize(initSize);
            mUsedFences.reserve(initSize);

            VkFenceCreateInfo info;
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;

            for(Uint32 i = 0; i < initSize; i++) {
                res = vkCreateFence(mDevice.GetHandle(), &info, nullptr, &fence);
                CHECK_VK(res, "Failed to create VkFence.");

                mFreeFences[i] = VulkanFence(fence);
            }
        }

        void VulkanFencePool::Shutdown()
        {
            CHECK(mUsedFences.size() == 0, "Cannot shduwon VulkanFencePool becausae some used fences existed.");

            for(auto f : mFreeFences) {
                vkDestroyFence(mDevice.GetHandle(), f.mFence, nullptr);
            }
        }

        VulkanFence VulkanFencePool::AllocateFence(const char* debugName)
        {
            VulkanFence fence;

            Lock lock(mFencesMutex);

            if(mFreeFences.size() == 0) {
                VkResult res;
                VkFenceCreateInfo info;
                info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0;

                VkFence f;
                res = vkCreateFence(mDevice.GetHandle(), &info, nullptr, &f);
                CHECK_VK(res, "Failed to create VkFence.");

                fence.mFence = f;
            } else {
                fence = mFreeFences.back();
                mFreeFences.pop_back();
            }

            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), fence.mFence, debugName);
            mUsedFences.push_back(fence);

            return fence;
        }

        void VulkanFencePool::FreeFence(VulkanFence& fence)
        {
            CHECK(fence.mFence != VK_NULL_HANDLE, "The fence is already freed.");

            Lock lock(mFencesMutex);

            mFreeFences.push_back(fence);

            fence.mFence = VK_NULL_HANDLE;
        }
    } // namespace rapi
} // namespace cube
