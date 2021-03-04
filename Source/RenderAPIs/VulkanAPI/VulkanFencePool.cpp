#include "VulkanFencePool.h"

#include "VulkanDevice.h"
#include "VulkanUtility.h"
#include "VulkanDebug.h"
#include "Core/Assertion.h"
#include "Core/Allocator/FrameAllocator.h"

#include "Interface/FenceVk.h"

namespace cube
{
    namespace rapi
    {
        void VulkanFencePool::Initialize()
        {
            VkResult res;
            VkFence fence;

            // Create 10 fences
            constexpr Uint32 initSize = 10;
            mFreeFences.resize(initSize);

            VkFenceCreateInfo info;
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;

            for(Uint32 i = 0; i < initSize; i++) {
                res = vkCreateFence(mDevice.GetHandle(), &info, nullptr, &fence);
                CHECK_VK(res, "Failed to create VkFence.");

                mFreeFences[i] = fence;
            }
        }

        void VulkanFencePool::Shutdown()
        {
            for(auto fence : mFreeFences) {
                vkDestroyFence(mDevice.GetHandle(), fence, nullptr);
            }
        }

        SPtr<FenceVk> VulkanFencePool::AllocateFence(const char* debugName)
        {
            VkFence fence;

            {
                Lock lock(mFencesMutex);

                if(mFreeFences.size() == 0) {
                    VkResult res;
                    VkFenceCreateInfo info;
                    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                    info.pNext = nullptr;
                    info.flags = 0;


                    res = vkCreateFence(mDevice.GetHandle(), &info, nullptr, &fence);
                    CHECK_VK(res, "Failed to create VkFence.");
                } else {
                    fence = mFreeFences.back();
                    mFreeFences.pop_back();
                }
            }

            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), fence, debugName);

            return std::make_shared<FenceVk>(mDevice, *this, fence);
        }

        void VulkanFencePool::FreeFence(FenceVk& fence)
        {
            if(fence.GetHandle() == VK_NULL_HANDLE) {
                return;
            }

            {
                Lock lock(mFencesMutex);

                mFreeFences.push_back(fence.GetHandle());
                fence.mFence = VK_NULL_HANDLE;
            }
        }

        SPtr<FenceVk> VulkanFencePool::CreateNullFence()
        {
            VkFence vkFence = VK_NULL_HANDLE;
            return std::make_shared<FenceVk>(mDevice, *this, vkFence);
        }
    } // namespace rapi
} // namespace cube
