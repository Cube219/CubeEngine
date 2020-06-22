#include "BufferVk.h"

#include "../VulkanDevice.h"
#include "../VulkanUtility.h"
#include "Core/Assertion.h"
#include "../VulkanDebug.h"

namespace cube
{
    namespace rapi
    {
        BufferVk::BufferVk(VulkanDevice& device, ResourceUsage usage, VkBufferUsageFlags bufUsage, Uint64 size, const void* pData, const char* debugName) :
            mDevice(device)
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
            // If the usage is default or immutable, the buffer will be transferred from a staging buffer
            if(usage == ResourceUsage::Default || usage == ResourceUsage::Immutable)
                info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

            res = vkCreateBuffer(device.GetHandle(), &info, nullptr, &mBuffer);
            CHECK_VK(res, "Failed to create VkBuffer.");
            VULKAN_SET_OBJ_NAME(device.GetHandle(), mBuffer, debugName);

            // Allocate memory
            mAllocation = device.GetAllocator().Allocate(usage, info, &mBuffer);

            // Initialize data if it is existed
            if(pData != nullptr && size > 0) {
            }
        }

        BufferVk::~BufferVk()
        {
            mDevice.GetAllocator().Free(mAllocation);

            vkDestroyBuffer(mDevice.GetHandle(), mBuffer, nullptr);
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
    } // namespace rapi
} // namespace cube
