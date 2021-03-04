#include "VulkanStagingManager.h"

#include "VulkanDevice.h"
#include "VulkanUtility.h"
#include "VulkanDebug.h"
#include "Core/Allocator/FrameAllocator.h"

namespace cube
{
    namespace rapi
    {
        VulkanStagingBuffer::VulkanStagingBuffer() :
            mBuffer(VK_NULL_HANDLE)
        {
        }

        VulkanStagingBuffer::VulkanStagingBuffer(VulkanDevice& device, Type type, VkBuffer buffer, VulkanAllocation allocation):
            mpDevice(&device),
            mType(type),
            mBuffer(buffer),
            mAllocation(allocation)
        {
            CommandListAllocateInfo cmdInfo;
            cmdInfo.type = CommandListType::Transfer;

            mSubmitCommandBuffer = mpDevice->GetCommandPoolManager().AllocateCommandBuffer(cmdInfo);
        }

        VulkanStagingBuffer::~VulkanStagingBuffer()
        {
            CHECK(mBuffer == VK_NULL_HANDLE, "VulkanStagingBuffer must be released by VulkanStagingManager before destoryed.");
            mpDevice->GetCommandPoolManager().FreeCommandBuffer(mSubmitCommandBuffer);
        }

        VulkanStagingBuffer::VulkanStagingBuffer(VulkanStagingBuffer&& other) noexcept
        {
            mpDevice = other.mpDevice;
            mType = other.mType;
            mBuffer = other.mBuffer;
            mAllocation = other.mAllocation;
            mSubmitCommandBuffer = other.mSubmitCommandBuffer;

            other.mBuffer = VK_NULL_HANDLE;
            other.mAllocation.allocation = VK_NULL_HANDLE;
            other.mSubmitCommandBuffer = VulkanCommandBuffer();
        }

        VulkanStagingBuffer& VulkanStagingBuffer::operator=(VulkanStagingBuffer&& rhs) noexcept
        {
            if(this == &rhs) return *this;

            mpDevice = rhs.mpDevice;
            mType = rhs.mType;
            mBuffer = rhs.mBuffer;
            mAllocation = rhs.mAllocation;
            mSubmitCommandBuffer = rhs.mSubmitCommandBuffer;

            rhs.mBuffer = VK_NULL_HANDLE;
            rhs.mAllocation.allocation = VK_NULL_HANDLE;
            rhs.mSubmitCommandBuffer = VulkanCommandBuffer();

            return *this;
        }

        void VulkanStagingBuffer::InitCommandBuffer()
        {
            VkResult res;

            res = vkResetCommandBuffer(mSubmitCommandBuffer.handle, 0);
            CHECK_VK(res, "Failed to reset command buffer.");

            VkCommandBufferBeginInfo beginInfo;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;

            res = vkBeginCommandBuffer(mSubmitCommandBuffer.handle, &beginInfo);
            CHECK_VK(res, "Failed to begin the command buffer.");
        }

        void VulkanStagingBuffer::CopyBuffer(VkBuffer srcBuffer, Uint64 srcOffset, Uint64 size)
        {
            VkBufferMemoryBarrier memBarrier;
            memBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            memBarrier.pNext = nullptr;
            memBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memBarrier.buffer = srcBuffer;
            memBarrier.offset = 0;
            memBarrier.size = size;
            vkCmdPipelineBarrier(mSubmitCommandBuffer.handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr, 1, &memBarrier, 0, nullptr);

            VkBufferCopy bufCopy;
            bufCopy.srcOffset = srcOffset;
            bufCopy.dstOffset = 0;
            bufCopy.size = size;
            vkCmdCopyBuffer(mSubmitCommandBuffer.handle, srcBuffer, mBuffer, 1, &bufCopy);

            memBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            memBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
            memBarrier.buffer = mBuffer;
            memBarrier.size = mAllocation.size;
            vkCmdPipelineBarrier(mSubmitCommandBuffer.handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                0, nullptr, 1, &memBarrier, 0, nullptr);
        }

        void VulkanStagingBuffer::FlushBuffer(VkBuffer dstBuffer, Uint64 dstOffset, Uint64 size)
        {
            VkBufferCopy bufCopy;
            bufCopy.srcOffset = 0;
            bufCopy.dstOffset = dstOffset;
            bufCopy.size = size;
            vkCmdCopyBuffer(mSubmitCommandBuffer.handle, mBuffer, dstBuffer, 1, &bufCopy);
        }

        void VulkanStagingBuffer::CopyImage(VkImage srcImage, const VkBufferImageCopy& region)
        {
            vkCmdCopyImageToBuffer(mSubmitCommandBuffer.handle, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mBuffer, 1, &region);

            VkBufferMemoryBarrier memBarrier;
            memBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            memBarrier.pNext = nullptr;
            memBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            memBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
            memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memBarrier.buffer = mBuffer;
            memBarrier.offset = 0;
            memBarrier.size = mAllocation.size;
            vkCmdPipelineBarrier(mSubmitCommandBuffer.handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                0, nullptr, 1, &memBarrier, 0, nullptr);
        }

        void VulkanStagingBuffer::FlushImage(VkImage dstImage, const VkBufferImageCopy& region)
        {
            vkCmdCopyBufferToImage(mSubmitCommandBuffer.handle, mBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        }

        SPtr<FenceVk> VulkanStagingBuffer::SubmitCommandBuffer()
        {
            VkResult res;

            res = vkEndCommandBuffer(mSubmitCommandBuffer.handle);
            CHECK_VK(res, "Failed to end the command buffer.");

            return mpDevice->GetQueueManager().SubmitCommandBufferWithFence(mSubmitCommandBuffer);
        }

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

            return VulkanStagingBuffer(mDevice, type, buffer, alloc);
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
