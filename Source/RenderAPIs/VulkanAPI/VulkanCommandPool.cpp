#include "VulkanCommandPool.h"

#include "VulkanDevice.h"
#include "VulkanUtility.h"
#include "VulkanDebug.h"

namespace cube
{
    namespace rapi
    {
        VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, VulkanCommandPool& commandPool) :
            mDevice(device),
            mCommandPool(commandPool),
            mCommandBuffer(VK_NULL_HANDLE)
        {
            // Allocated VkCommandBuffer in VulkanCommandPool
            /*
            VkResult res;

            VkCommandBufferAllocateInfo info;
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.pNext = nullptr;
            info.commandPool = commandPool.GetHandle();
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;

            res = vkAllocateCommandBuffers(mDevice.GetHandle(), &info, &mCommandBuffer);
            CHECK_VK(res, "Failed to allocate command buffer.");*/
        }

        VulkanCommandBuffer::~VulkanCommandBuffer()
        {
            if(mCommandBuffer != VK_NULL_HANDLE) {
                vkFreeCommandBuffers(mDevice.GetHandle(), mCommandPool.GetHandle(), 1, &mCommandBuffer);
            }
        }

        void VulkanCommandBuffer::Begin()
        {
            VkResult res;

            VkCommandBufferBeginInfo beginInfo;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;

            res = vkBeginCommandBuffer(mCommandBuffer, &beginInfo);
            CHECK_VK(res, "Failed to begin the command buffer.");
        }

        void VulkanCommandBuffer::End()
        {
            VkResult res;

            res = vkEndCommandBuffer(mCommandBuffer);
            CHECK_VK(res, "Failed to end the command buffer.");
        }

        void VulkanCommandBuffer::Reset()
        {
            VkResult res;

            res = vkResetCommandBuffer(mCommandBuffer, 0);
            CHECK_VK(res, "Failed to reset command buffer.");
        }

        void VulkanCommandBuffer::SetMemoryBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkBuffer buffer, Uint64 size)
        {
            VkBufferMemoryBarrier memBarrier;
            memBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            memBarrier.pNext = nullptr;
            memBarrier.srcAccessMask = srcAccessMask;
            memBarrier.dstAccessMask = dstAccessMask;
            memBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memBarrier.buffer = buffer;
            memBarrier.offset = 0;
            memBarrier.size = size;

            vkCmdPipelineBarrier(mCommandBuffer, srcStageMask, dstStageMask, 0,
                0, nullptr, 1, &memBarrier, 0, nullptr);
        }

        void VulkanCommandBuffer::CopyBuffer(VkBuffer src, VkBuffer dst, Uint64 srcOffset, Uint64 dstOffset, Uint64 size)
        {
            VkBufferCopy bufCopy;
            bufCopy.srcOffset = srcOffset;
            bufCopy.dstOffset = dstOffset;
            bufCopy.size = size;

            vkCmdCopyBuffer(mCommandBuffer, src, dst, 1, &bufCopy);
        }

        void VulkanCommandBuffer::CopyBufferToImage(VkBuffer src, VkImage dst, VkBufferImageCopy& region)
        {
            vkCmdCopyBufferToImage(mCommandBuffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        }

        void VulkanCommandBuffer::FreeCommandBuffer()
        {
            if(mCommandBuffer != VK_NULL_HANDLE) {
                mCommandPool.FreeCommandBuffer(*this, true);
            }
        }

        VulkanCommandPool::VulkanCommandPool(VulkanDevice& device, Uint32 queueFamilyIndex, bool isTransient) :
            mDevice(device),
            mCommandPool(VK_NULL_HANDLE)
        {
            VkResult res;

            VkCommandPoolCreateInfo info;
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.pNext = nullptr;
            if(isTransient == true) info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            else                    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            info.queueFamilyIndex = queueFamilyIndex;

            res = vkCreateCommandPool(mDevice.GetHandle(), &info, nullptr, &mCommandPool);
            CHECK_VK(res, "Failed to create command pool.");
        }

        VulkanCommandPool::~VulkanCommandPool()
        {
            vkDestroyCommandPool(mDevice.GetHandle(), mCommandPool, nullptr);
        }

        VulkanCommandBuffer VulkanCommandPool::AllocateCommandBuffer(const char* debugName)
        {
            VkResult res;

            VulkanCommandBuffer cmdBuf(mDevice, *this);

            VkCommandBufferAllocateInfo info;
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.pNext = nullptr;
            info.commandPool = mCommandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;

            res = vkAllocateCommandBuffers(mDevice.GetHandle(), &info, &cmdBuf.mCommandBuffer);
            CHECK_VK(res, "Failed to allocate command buffer.");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), cmdBuf.mCommandBuffer, debugName);

            return cmdBuf;
        }

        void VulkanCommandPool::FreeCommandBuffer(VulkanCommandBuffer& cmdBuf, bool immediately)
        {
            if(immediately == true) {
                vkFreeCommandBuffers(mDevice.GetHandle(), mCommandPool, 1, &cmdBuf.mCommandBuffer);
            } else {
                mFreeCommandBuffers.push_back(cmdBuf.mCommandBuffer);
            }

            cmdBuf.mCommandBuffer = VK_NULL_HANDLE;
        }
    }
} // namespace cube
