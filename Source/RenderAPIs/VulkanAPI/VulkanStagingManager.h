#pragma once

#include "VulkanAPIHeader.h"

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
            VulkanStagingBuffer() :
                mBuffer(VK_NULL_HANDLE)
            {}
            VulkanStagingBuffer(Type type, VkBuffer buffer, VulkanAllocation allocation) :
                mType(type),
                mBuffer(buffer),
                mAllocation(allocation)
            {}
            ~VulkanStagingBuffer() {}

            VulkanStagingBuffer(const VulkanStagingBuffer& other) = delete;
            VulkanStagingBuffer& operator=(const VulkanStagingBuffer& rhs) = delete;

            VulkanStagingBuffer(VulkanStagingBuffer&& other) noexcept
            {
                mType = other.mType;
                mBuffer = other.mBuffer;
                mAllocation = other.mAllocation;

                other.mBuffer = VK_NULL_HANDLE;
                other.mAllocation.allocation = VK_NULL_HANDLE;
            }
            VulkanStagingBuffer& operator=(VulkanStagingBuffer&& rhs) noexcept
            {
                if(this == &rhs) return *this;

                mType = rhs.mType;
                mBuffer = rhs.mBuffer;
                mAllocation = rhs.mAllocation;

                rhs.mBuffer = VK_NULL_HANDLE;
                rhs.mAllocation.allocation = VK_NULL_HANDLE;

                return *this;
            }

            VkBuffer GetHandle() const { return mBuffer; }

            bool IsValid() const { return mBuffer != VK_NULL_HANDLE; }

            Type GetType() const { return mType; }
            void* GetMappedPtr() { return mAllocation.pMappedPtr; }
            Uint64 GetSize() const { return mAllocation.size; }

        private:
            friend class VulkanStagingManager;

            Type mType;
            VkBuffer mBuffer;
            VulkanAllocation mAllocation;
        };

        class VulkanStagingManager
        {
        public:
            VulkanStagingManager(VulkanDevice& device) :
                mDevice(device)
            {}
            ~VulkanStagingManager() {}

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
