#include "VulkanStagingManager.h"

#include "VulkanDevice.h"
#include "VulkanUtility.h"
#include "VulkanDebug.h"
#include "Core/Allocator/FrameAllocator.h"

namespace cube
{
    namespace rapi
    {
        void VulkanStagingManager::Initialize()
        {
        }

        void VulkanStagingManager::Shutdown()
        {
        }

        VulkanStagingBuffer VulkanStagingManager::GetBuffer(Uint64 size, VulkanStagingBuffer::Type type, const char* debugName)
        {
            VkResult res;

            ResourceUsage usage;

            VkBufferCreateInfo info;
            info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;
            info.size = size;
            info.queueFamilyIndexCount = 0;
            info.pQueueFamilyIndices = nullptr;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            if(type == VulkanStagingBuffer::Type::Read) {
                info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                usage = ResourceUsage::Staging;
            } else if(type == VulkanStagingBuffer::Type::Write) {
                info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                usage = ResourceUsage::Dynamic;
            } else {
                info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                usage = ResourceUsage::Dynamic;
            }

            VkBuffer buffer;
            res = vkCreateBuffer(mDevice.GetHandle(), &info, nullptr, &buffer);
            CHECK_VK(res, "Failed to create VkBuffer.");
            auto debugNameTemp = FrameFormat("Staging buffer for '{}'", debugName);
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), buffer, debugNameTemp.data());

            VulkanAllocation alloc;
            alloc = mDevice.GetAllocator().Allocate(usage, info, &buffer);

            return VulkanStagingBuffer(type, buffer, alloc);
        }

        void VulkanStagingManager::ReleaseBuffer(VulkanStagingBuffer&& stagingBuf)
        {
            Lock lock(mStagingBuffersMutex);

            mStagingBuffersToRelease.push_back(std::move(stagingBuf));
        }

        void VulkanStagingManager::ReleaseAllBuffers()
        {
            Lock lock(mStagingBuffersMutex);

            for(auto& stagingBuf : mStagingBuffersToRelease) {
                if(stagingBuf.mAllocation.allocation != VK_NULL_HANDLE) {
                    mDevice.GetAllocator().Free(stagingBuf.mAllocation);
                }
                if(stagingBuf.mBuffer != VK_NULL_HANDLE) {
                    vkDestroyBuffer(mDevice.GetHandle(), stagingBuf.mBuffer, nullptr);
                }
            }
            mStagingBuffersToRelease.clear();
        }
    } // namespace rapi
} // namespace cube
