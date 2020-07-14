#include "VulkanStagingManager.h"

#include "VulkanDevice.h"
#include "VulkanUtility.h"
#include "VulkanDebug.h"
#include "Core/Allocator/FrameAllocator.h"

namespace cube
{
    namespace rapi
    {
        VulkanStagingBuffer::VulkanStagingBuffer(VulkanDevice& device, Uint64 size, const char* debugName) :
            mDevice(device),
            mBuffer(VK_NULL_HANDLE)
        {
            VkResult res;

            VkBufferCreateInfo info;
            info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;
            info.size = size;
            info.queueFamilyIndexCount = 0;
            info.pQueueFamilyIndices = nullptr;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            res = vkCreateBuffer(device.GetHandle(), &info, nullptr, &mBuffer);
            CHECK_VK(res, "Failed to create VkBuffer.");
            auto debugNameTemp = FrameFormat("Staging buffer for '{}'", debugName);
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mBuffer, debugNameTemp.data());

            mAllocation = device.GetAllocator().Allocate(ResourceUsage::Staging, info, &mBuffer);
        }

        VulkanStagingBuffer::~VulkanStagingBuffer()
        {
            if(mAllocation.allocation != VK_NULL_HANDLE) {
                mDevice.GetAllocator().Free(mAllocation);
            }
            if(mBuffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(mDevice.GetHandle(), mBuffer, nullptr);
            }

            mBuffer = VK_NULL_HANDLE;
            mAllocation.allocation = VK_NULL_HANDLE;
        }

        void VulkanStagingManager::Initialize()
        {
        }

        void VulkanStagingManager::Shutdown()
        {
        }

        VulkanStagingBuffer VulkanStagingManager::GetBuffer(Uint64 size, const char* debugName)
        {

            return VulkanStagingBuffer(mDevice, size, debugName);
        }

        void VulkanStagingManager::ReleaseBuffer(VulkanStagingBuffer&& stagingBuf)
        {
            Lock lock(mStagingBuffersMutex);

            mStagingBuffersToRelease.push_back(std::move(stagingBuf));
        }

        void VulkanStagingManager::ReleaseAllBuffers()
        {
            Lock lock(mStagingBuffersMutex);

            mStagingBuffersToRelease.clear();
        }
    } // namespace rapi
} // namespace cube
