#include "BufferVk.h"

#include "../VulkanDevice.h"
#include "../VulkanUtility.h"
#include "Core/Assertion.h"
#include "../VulkanDebug.h"
#include "FenceVk.h"
#include "Core/Allocator/FrameAllocator.h"

namespace cube
{
    namespace rapi
    {
        BufferVk::BufferVk(VulkanDevice& device, ResourceUsage usage, VkBufferUsageFlags bufUsage, Uint64 size, const void* pData, const char* debugName) :
            mDevice(device),
            mUsage(usage),
            mSize(size),
            mDebugName(debugName)
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

            info.usage = bufUsage;
            // If the usage is default or immutable, the buffer can be transferred from a staging buffer or to
            if(usage == ResourceUsage::Default || usage == ResourceUsage::Immutable)
                info.usage |= (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

            res = vkCreateBuffer(device.GetHandle(), &info, nullptr, &mBuffer);
            CHECK_VK(res, "Failed to create VkBuffer.");
            VULKAN_SET_OBJ_NAME(device.GetHandle(), mBuffer, debugName);

            // Allocate memory
            mAllocation = device.GetAllocator().Allocate(usage, info, &mBuffer);

            // Initialize data if it is existed
            if(pData != nullptr && size > 0) {
                // Change the usage temporarily to enable map
                if(usage == ResourceUsage::Immutable) mUsage = ResourceUsage::Default;

                void* p;
                MapImpl(ResourceMapType::Write, p, true);
                memcpy(p, pData, size);

                UnmapImpl(true);

                mUsage = usage;
            }          
        }

        BufferVk::~BufferVk()
        {
            mDevice.GetAllocator().Free(mAllocation);
            if(mStagingBuffer.IsValid()) {
                mDevice.GetStagingManager().ReleaseBuffer(std::move(mStagingBuffer));
            }

            vkDestroyBuffer(mDevice.GetHandle(), mBuffer, nullptr);
        }

        SPtr<Fence> BufferVk::MapImpl(ResourceMapType type, void*& pMappedResource, bool waitUntilFinished)
        {
            CHECK(mUsage != ResourceUsage::Immutable, "Cannot map immutable resource.");
            if(mUsage == ResourceUsage::Staging) {
                CHECK(type == ResourceMapType::Read, "Cannot map staging resource in write mode.");
            }

            if(mUsage == ResourceUsage::Dynamic) {
                // Dynamic resource is always mapped
                pMappedResource = mAllocation.pMappedPtr;
                return nullptr;
            }

            if(mAllocation.isHostVisible == true) {
                mAllocation.Map();
                pMappedResource = mAllocation.pMappedPtr;
                return nullptr;
            }

            // Using staging buffer
            VulkanStagingBuffer::Type stagingBufType;
            switch(type)
            {
                case ResourceMapType::Read:      stagingBufType = VulkanStagingBuffer::Type::Read;  break;
                case ResourceMapType::Write:     stagingBufType = VulkanStagingBuffer::Type::Write; break;
                case ResourceMapType::ReadWrite: // Fallthrough
                default:
                    stagingBufType = VulkanStagingBuffer::Type::ReadWrite;
                    break;
            }
            mStagingBuffer = mDevice.GetStagingManager().GetBuffer(mSize, stagingBufType, mDebugName);
            pMappedResource = mStagingBuffer.GetMappedPtr();

            if(type != ResourceMapType::Write) {
                mStagingBuffer.InitCommandBuffer();
                mStagingBuffer.CopyBuffer(mBuffer, 0, mSize);
                auto fence = mStagingBuffer.SubmitCommandBuffer();
                if(waitUntilFinished == true) {
                    fence->Wait(10.0f);
                    return nullptr;
                } else {
                    return fence;
                }
            }

            if(waitUntilFinished == true) {
                return nullptr;
            } else {
                return mDevice.GetFencePool().CreateNullFence();
            }
        }

        SPtr<Fence> BufferVk::UnmapImpl(bool waitUntilFinished)
        {
            CHECK(mUsage != ResourceUsage::Immutable, "Cannot unmap immutable resource.");

            if(mUsage == ResourceUsage::Dynamic) {
                // Dynamic resource is always mapped
                return nullptr;
            }

            if(mAllocation.isHostVisible == true) {
                mAllocation.Unmap();
                return nullptr;
            }

            if(mStagingBuffer.IsValid() && mStagingBuffer.GetType() != VulkanStagingBuffer::Type::Read) {
                mStagingBuffer.InitCommandBuffer();
                mStagingBuffer.FlushBuffer(mBuffer, 0, mSize);
                auto fence = mStagingBuffer.SubmitCommandBuffer();
                mDevice.GetStagingManager().ReleaseBuffer(std::move(mStagingBuffer));

                if(waitUntilFinished == true) {
                    fence->Wait(10.0f);
                    return nullptr;
                } else {
                    return fence;
                }
            }

            if(mStagingBuffer.IsValid() == true) {
                mDevice.GetStagingManager().ReleaseBuffer(std::move(mStagingBuffer));
            }

            if(waitUntilFinished == true) {
                return nullptr;
            } else {
                return mDevice.GetFencePool().CreateNullFence();
            }
        }

        VertexBufferVk::VertexBufferVk(VulkanDevice& device, const VertexBufferCreateInfo& info) :
            VertexBuffer(info.usage, info.size, info.debugName),
            BufferVk(device, info.usage, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, info.size, info.pData, info.debugName)
        {}

        IndexBufferVk::IndexBufferVk(VulkanDevice& device, const IndexBufferCreateInfo& info) :
            IndexBuffer(info.usage, info.size, info.debugName),
            BufferVk(device, info.usage, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, info.size, info.pData, info.debugName)
        {
            switch(info.strideType)
            {
                case IndexBufferCreateInfo::StrideType::Uint16:
                    mIndexType = VK_INDEX_TYPE_UINT16;
                    break;
                case IndexBufferCreateInfo::StrideType::Uint32:
                    mIndexType = VK_INDEX_TYPE_UINT32;
                    break;

                default:
                    ASSERTION_FAILED("Invalid stride type {}.", info.strideType);
                    break;
            }
        }

        ConstantBufferVk::ConstantBufferVk(VulkanDevice& device, const ConstantBufferCreateInfo& info) :
            ConstantBuffer(info.usage, info.size, info.debugName),
            BufferVk(device, info.usage, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, info.size, info.pData, info.debugName)
        {}

        StructuredBufferVk::StructuredBufferVk(VulkanDevice& device, const StructuredBufferCreateInfo& info) :
            StructuredBuffer(info.usage, info.size, info.debugName),
            BufferVk(device, info.usage, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, info.size, info.pData, info.debugName)
        {}
    } // namespace rapi
} // namespace cube
