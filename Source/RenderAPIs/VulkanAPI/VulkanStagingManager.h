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
            VulkanStagingBuffer(VulkanDevice& device, Uint64 size, const char* debugName);
            ~VulkanStagingBuffer();

            VulkanStagingBuffer(const VulkanStagingBuffer& other) = delete;
            VulkanStagingBuffer& operator=(const VulkanStagingBuffer& rhs) = delete;

            VulkanStagingBuffer(VulkanStagingBuffer&& other) noexcept :
                mDevice(other.mDevice)
            {
                mBuffer = other.mBuffer;
                mAllocation = other.mAllocation;

                other.mBuffer = VK_NULL_HANDLE;
                other.mAllocation.allocation = VK_NULL_HANDLE;
            }
            VulkanStagingBuffer& operator=(VulkanStagingBuffer&& rhs) noexcept
            {
                if(this == &rhs) return *this;

                if(mBuffer != VK_NULL_HANDLE) {
                    this->~VulkanStagingBuffer();
                }

                mBuffer = rhs.mBuffer;
                mAllocation = rhs.mAllocation;

                rhs.mBuffer = VK_NULL_HANDLE;
                rhs.mAllocation.allocation = VK_NULL_HANDLE;

                return *this;
            }

            VkBuffer GetHandle() const { return mBuffer; }

            void* GetMappedPtr() { return mAllocation.pMappedPtr; }
            Uint64 GetSize() const { return mAllocation.size; }

        private:
            VulkanDevice& mDevice;

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

            void Initialize();
            void Shutdown();

            VulkanStagingBuffer GetBuffer(Uint64 size, const char* debugName = nullptr);
            void ReleaseBuffer(VulkanStagingBuffer&& stagingBuf);

            void ReleaseAllBuffers();

        private:
            VulkanDevice& mDevice;

            Mutex mStagingBuffersMutex;
            Vector<VulkanStagingBuffer> mStagingBuffersToRelease;
        };
    } // namespace rapi
} // namespace cube
