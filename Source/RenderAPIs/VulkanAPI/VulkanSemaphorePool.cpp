#include "VulkanSemaphorePool.h"

#include "VulkanDevice.h"
#include "VulkanUtility.h"
#include "Core/Assertion.h"
#include "VulkanDebug.h"

namespace cube
{
    namespace rapi
    {
        void VulkanSemaphorePool::Initialize()
        {
            VkResult res;
            VkSemaphore semaphore;

            // Create 20 semaphores
            constexpr Uint32 initSize = 20;
            mFreeSemaphores.resize(20);
            mUsedSemaphores.reserve(20);

            VkSemaphoreCreateInfo info;
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;

            for(Uint32 i = 0; i < initSize; i++) {
                res = vkCreateSemaphore(mDevice.GetHandle(), &info, nullptr, &semaphore);
                CHECK_VK(res, "Failed to create semaphore.");

                mFreeSemaphores[i] = VulkanSemaphore(semaphore);
            }
        }

        void VulkanSemaphorePool::Shutdown()
        {
            CHECK(mUsedSemaphores.size() == 0, "Cannot shduwon VulkanSemaphorePool becausae some used semaphores existed.");

            for(auto s : mFreeSemaphores) {
                vkDestroySemaphore(mDevice.GetHandle(), s.GetHandle(), nullptr);
            }
        }

        VulkanSemaphore VulkanSemaphorePool::AllocateSemaphore(const char* debugName)
        {
            VulkanSemaphore semaphore;

            Lock lock(mSemaphoresMutex);

            if(mFreeSemaphores.size() == 0) {
                VkResult res;

                VkSemaphoreCreateInfo info;
                info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0;

                VkSemaphore s;
                res = vkCreateSemaphore(mDevice.GetHandle(), &info, nullptr, &s);
                CHECK_VK(res, "Failed to create semaphore.");

                semaphore.mSemaphore = s;
            } else {
                semaphore = mFreeSemaphores.back();
                mFreeSemaphores.pop_back();
            }

            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), semaphore.mSemaphore, debugName);
            mUsedSemaphores.push_back(semaphore);

            return semaphore;
        }

        void VulkanSemaphorePool::FreeSemaphore(VulkanSemaphore& semaphore)
        {
            if(semaphore.mSemaphore == VK_NULL_HANDLE) {
                return;
            }

            Lock lock(mSemaphoresMutex);

            mFreeSemaphores.push_back(semaphore);

            semaphore.mSemaphore = VK_NULL_HANDLE;
        }
    } // namespace rapi
} // namespace cube
