#include "BufferVk.h"

#include "../VulkanDevice.h"
#include "../VulkanUtility.h"
#include "Core/Assertion.h"
#include "../VulkanDebug.h"
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
                MapImpl(ResourceMapType::Write, p);
                memcpy(p, pData, size);

                UnmapImpl();

                mUsage = usage;
            }          
        }

        BufferVk::~BufferVk()
        {
            mDevice.GetAllocator().Free(mAllocation);

            vkDestroyBuffer(mDevice.GetHandle(), mBuffer, nullptr);
        }

        void BufferVk::MapImpl(ResourceMapType type, void*& pMappedResource)
        {
            CHECK(mUsage != ResourceUsage::Immutable, "Cannot map immutable resource.");
            if(mUsage == ResourceUsage::Staging) {
                CHECK(type == ResourceMapType::Read, "Cannot map staging resource in write mode.");
            }

            if(mUsage == ResourceUsage::Dynamic) {
                // Dynamic resource is always mapped
                pMappedResource = mAllocation.pMappedPtr;
                return;
            }

            if(mAllocation.isHostVisible == true) {
                mAllocation.Map();
                pMappedResource = mAllocation.pMappedPtr;
                return;
            }

            VulkanStagingBuffer::Type stagingBufType;
            switch(type)
            {
                case ResourceMapType::Read:      stagingBufType = VulkanStagingBuffer::Type::Read; break;
                case ResourceMapType::Write:     stagingBufType = VulkanStagingBuffer::Type::Write; break;
                case ResourceMapType::ReadWrite: // Fallthrough
                default:
                    stagingBufType = VulkanStagingBuffer::Type::ReadWrite;
                    break;
            }
            mStagingBuffer = mDevice.GetStagingManager().GetBuffer(mSize, stagingBufType, mDebugName);

            if(type != ResourceMapType::Write) {
                VulkanCommandBuffer uploadCmdBuf = mDevice.GetUploadCommandBuffer();
                uploadCmdBuf.SetMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, mBuffer, mSize);
                uploadCmdBuf.CopyBuffer(mBuffer, mStagingBuffer.GetHandle(), 0, 0, mSize);
                uploadCmdBuf.SetMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_HOST_READ_BIT, mStagingBuffer.GetHandle(), mSize);

                mDevice.SubmitUploadCommandBuffer(uploadCmdBuf, true);
            }

            pMappedResource = mStagingBuffer.GetMappedPtr();
        }

        void BufferVk::UnmapImpl()
        {
            CHECK(mUsage != ResourceUsage::Immutable, "Cannot unmap immutable resource.");

            if(mUsage == ResourceUsage::Dynamic) {
                // Dynamic resource is always mapped
                return;
            }

            if(mAllocation.isHostVisible == true) {
                mAllocation.Unmap();
                return;
            }

            if(mStagingBuffer.IsValid() && mStagingBuffer.GetType() != VulkanStagingBuffer::Type::Read) {
                VulkanCommandBuffer uploadCmdBuf = mDevice.GetUploadCommandBuffer();
                uploadCmdBuf.CopyBuffer(mStagingBuffer.GetHandle(), mBuffer, 0, 0, mSize);

                mDevice.SubmitUploadCommandBuffer(uploadCmdBuf);

                mDevice.GetStagingManager().ReleaseBuffer(std::move(mStagingBuffer));
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
