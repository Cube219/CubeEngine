#pragma once

#include "VulkanAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        class VulkanCommandBuffer
        {
        public:
            VulkanCommandBuffer(VulkanDevice& device, VulkanCommandPool& commandPool);
            ~VulkanCommandBuffer();

            VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;
            VulkanCommandBuffer& operator=(const VulkanCommandBuffer& rhs) = delete;
            VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept :
                mDevice(other.mDevice),
                mCommandPool(other.mCommandPool)
            {
                mCommandBuffer = other.mCommandBuffer;

                other.mCommandBuffer = VK_NULL_HANDLE;
            }
            VulkanCommandBuffer& operator=(VulkanCommandBuffer&& rhs) noexcept
            {
                if(this == &rhs) return *this;

                FreeCommandBuffer();

                mCommandBuffer = rhs.mCommandBuffer;

                rhs.mCommandBuffer = VK_NULL_HANDLE;
            }

            VkCommandBuffer GetHandle() const { return mCommandBuffer; }

            void Begin();
            void End();
            void Reset();

            void SetMemoryBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkBuffer buffer, Uint64 size);
            void CopyBuffer(VkBuffer src, VkBuffer dst, Uint64 srcOffset, Uint64 dstOffset, Uint64 size);
            void CopyBufferToImage(VkBuffer src, VkImage dst, VkBufferImageCopy& region);

        private:
            friend class VulkanCommandPool;

            void FreeCommandBuffer();

            VulkanDevice& mDevice;
            VulkanCommandPool& mCommandPool;

            VkCommandBuffer mCommandBuffer;
        };

        class VulkanCommandPool
        {
        public:
            VulkanCommandPool(VulkanDevice& device, Uint32 queueFamilyIndex, bool isTransient = false);
            ~VulkanCommandPool();

            VkCommandPool GetHandle() const { return mCommandPool; }

            VulkanCommandBuffer AllocateCommandBuffer(const char* debugName = nullptr);
            void FreeCommandBuffer(VulkanCommandBuffer& cmdBuf, bool immediately = false);

        private:
            VulkanDevice& mDevice;

            VkCommandPool mCommandPool;
            Vector<VkCommandBuffer> mFreeCommandBuffers;
        };
    } // namespace rapi
} // namespace cube
