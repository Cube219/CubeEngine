#pragma once

#include "VulkanAPIHeader.h"

#include "VulkanCommandPoolManager.h"
#include "VulkanMemoryAllocator.h"

#include "Core/Thread/MutexLock.h"

namespace cube
{
    namespace rapi
    {
        class VulkanStagingBuffer
        {
        public:
            enum class Type
            {
                Read,
                Write,
                ReadWrite
            };

        public:
            VulkanStagingBuffer();
            VulkanStagingBuffer(VulkanDevice& device, Type type, VkBuffer buffer, VulkanAllocation allocation);
            ~VulkanStagingBuffer();

            VulkanStagingBuffer(const VulkanStagingBuffer& other) = delete;
            VulkanStagingBuffer& operator=(const VulkanStagingBuffer& rhs) = delete;

            VulkanStagingBuffer(VulkanStagingBuffer&& other) noexcept;
            VulkanStagingBuffer& operator=(VulkanStagingBuffer&& rhs) noexcept;

            VkBuffer GetHandle() const { return mBuffer; }

            bool IsValid() const { return mBuffer != VK_NULL_HANDLE; }

            Type GetType() const { return mType; }
            void* GetMappedPtr() { return mAllocation.pMappedPtr; }
            Uint64 GetSize() const { return mAllocation.size; }

            VulkanCommandBuffer& GetCommandBuffer() { return mSubmitCommandBuffer; }
            void InitCommandBuffer();

            void CopyBuffer(VkBuffer srcBuffer, Uint64 srcOffset, Uint64 size);
            void FlushBuffer(VkBuffer dstBuffer, Uint64 dstOffset, Uint64 size);
            void CopyImage(VkImage srcImage, const VkBufferImageCopy& region);
            void FlushImage(VkImage dstImage, const VkBufferImageCopy& region);

            SPtr<FenceVk> SubmitCommandBuffer();

        private:
            friend class VulkanStagingManager;

            VulkanDevice* mpDevice;

            Type mType;
            VkBuffer mBuffer;
            VulkanAllocation mAllocation;

            VulkanCommandBuffer mSubmitCommandBuffer;
        };

        class VulkanStagingManager
        {
        public:
            VulkanStagingManager(VulkanDevice& device) :
                mDevice(device)
            {}
            ~VulkanStagingManager() = default;

            VulkanStagingManager(const VulkanStagingManager& other) = delete;
            VulkanStagingManager& operator=(const VulkanStagingManager& rhs) = delete;

            void Initialize();
            void Shutdown();

            VulkanStagingBuffer GetBuffer(Uint64 size, VulkanStagingBuffer::Type type, const char* debugName = nullptr);
            void ReleaseBuffer(VulkanStagingBuffer&& stagingBuf);

            void ReleaseAllBuffers();

        private:
            VulkanDevice& mDevice;

            Mutex mStagingBuffersMutex;
            Vector<VulkanStagingBuffer> mStagingBuffersToRelease;
        };
    } // namespace rapi
} // namespace cube
