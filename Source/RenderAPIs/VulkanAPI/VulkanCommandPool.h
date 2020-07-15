#pragma once

#include "VulkanAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        enum class VulkanCommandBufferType
        {
            Graphics,
            Compute,
            Transfer
        };

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
                mType = other.mType;
                mIsTransient = other.mIsTransient;

                other.mCommandBuffer = VK_NULL_HANDLE;
            }
            VulkanCommandBuffer& operator=(VulkanCommandBuffer&& rhs) noexcept
            {
                if(this == &rhs) return *this;

                Free(true);

                mCommandBuffer = rhs.mCommandBuffer;
                mType = rhs.mType;
                mIsTransient = rhs.mIsTransient;

                rhs.mCommandBuffer = VK_NULL_HANDLE;
            }

            VkCommandBuffer GetHandle() const { return mCommandBuffer; }
            VulkanCommandBufferType GetType() const { return mType; }

            void Begin();
            void End();
            void Reset();

            void Free(bool immediately = false);

            void SetMemoryBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkBuffer buffer, Uint64 size);
            void CopyBuffer(VkBuffer src, VkBuffer dst, Uint64 srcOffset, Uint64 dstOffset, Uint64 size);
            void CopyBufferToImage(VkBuffer src, VkImage dst, VkBufferImageCopy& region);

        private:
            friend class VulkanCommandPool;

            VulkanDevice& mDevice;
            VulkanCommandPool& mCommandPool;

            VkCommandBuffer mCommandBuffer;
            VulkanCommandBufferType mType;
            bool mIsTransient;
        };

        class VulkanCommandPool
        {
        public:
            VulkanCommandPool(VulkanDevice& device) :
                mDevice(device),
                mCommandPool(VK_NULL_HANDLE),
                mType(VulkanCommandBufferType::Graphics),
                mQueueFamilyIndex(0),
                mIsTransient(false)
            {}
            ~VulkanCommandPool() {}

            void CreatePool(VulkanCommandBufferType type, bool isTransient = false);
            void DestroyPool();

            VkCommandPool GetHandle() const { return mCommandPool; }

            VulkanCommandBuffer AllocateCommandBuffer(const char* debugName = nullptr);
            void FreeCommandBuffer(VulkanCommandBuffer& cmdBuf, bool immediately = false);

            void FreeCommandBuffersInQueue();

        private:
            VulkanDevice& mDevice;

            VkCommandPool mCommandPool;
            VulkanCommandBufferType mType;
            Uint32 mQueueFamilyIndex;
            bool mIsTransient;
            Vector<VkCommandBuffer> mFreeCommandBuffers;
        };
    } // namespace rapi
} // namespace cube
