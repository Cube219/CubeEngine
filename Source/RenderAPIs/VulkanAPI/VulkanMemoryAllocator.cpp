#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "VulkanMemoryAllocator.h"

#include "VulkanUtility.h"
#include "Core/Assertion.h"

namespace cube
{
    namespace rapi
    {
        void VulkanMemoryAllocator::Initialize(VkInstance instance, VkPhysicalDevice GPU, VkDevice device)
        {
            VkResult res;

            VmaAllocatorCreateInfo info = {};
            info.instance = instance;
            info.physicalDevice = GPU;
            info.device = device;

            res = vmaCreateAllocator(&info, &mAllocator);
            CHECK_VK(res, "Failed to create vulkan memory allocator.");
        }

        void VulkanMemoryAllocator::Shutdown()
        {
        }

        VulkanAllocation VulkanMemoryAllocator::Allocate(ResourceUsage usage, VkBufferCreateInfo& bufCreateInfo, VkBuffer* pBuf)
        {
            VmaAllocationCreateInfo createInfo = CreateVmaAllocationCreateInfo(usage);
            VmaAllocationInfo allocationInfo;
            VmaAllocation allocation;
            vmaCreateBuffer(mAllocator, &bufCreateInfo, &createInfo, pBuf, &allocation, &allocationInfo);

            VulkanAllocation res;
            res.resourceType = VulkanAllocation::ResourceType::Buffer;
            res.pResource = pBuf;
            
            UpdateVulkanAllocation(res, allocation, allocationInfo);
            
            return res;
        }

        VulkanAllocation VulkanMemoryAllocator::Allocate(ResourceUsage usage, VkImageCreateInfo& imageCreateInfo, VkImage* pImage)
        {
            VmaAllocationCreateInfo createInfo = CreateVmaAllocationCreateInfo(usage);
            VmaAllocationInfo allocationInfo;
            VmaAllocation allocation;
            vmaCreateImage(mAllocator, &imageCreateInfo, &createInfo, pImage, &allocation, &allocationInfo);

            VulkanAllocation res;
            res.resourceType = VulkanAllocation::ResourceType::Texture;
            res.pResource = pImage;

            UpdateVulkanAllocation(res, allocation, allocationInfo);

            return res;
        }

        void VulkanMemoryAllocator::Free(VulkanAllocation alloc)
        {
            switch(alloc.resourceType)
            {
                case VulkanAllocation::ResourceType::Buffer:
                    vmaDestroyBuffer(mAllocator, *(VkBuffer*)alloc.pResource, alloc.allocation);
                    break;
                case VulkanAllocation::ResourceType::Texture:
                    vmaDestroyImage(mAllocator, *(VkImage*)alloc.pResource, alloc.allocation);
                    break;
                default:
                    ASSERTION_FAILED("Invalid resource type in vulkan allocation. ({})", (int)alloc.resourceType);
                    break;
            }
        }

        VmaAllocationCreateInfo VulkanMemoryAllocator::CreateVmaAllocationCreateInfo(ResourceUsage usage)
        {
            VmaAllocationCreateInfo info = {};
            switch(usage)
            {
                case ResourceUsage::Default:
                case ResourceUsage::Immutable:
                    info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                    break;
                case ResourceUsage::Dynamic:
                    info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                    info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                    break;
                case ResourceUsage::Staging:
                    info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
                    break;
                default:
                    ASSERTION_FAILED("Invalid resource type {}", (int)usage);
                    break;
            }

            return info;
        }

        void VulkanMemoryAllocator::UpdateVulkanAllocation(VulkanAllocation& allocation, VmaAllocation vmaAllocation, const VmaAllocationInfo& info)
        {
            allocation.allocator = mAllocator;
            allocation.allocation = vmaAllocation;
            allocation.pMappedPtr = info.pMappedData;
            allocation.size = info.size;

            VkMemoryPropertyFlags memFlags;
            vmaGetMemoryTypeProperties(mAllocator, info.memoryType, &memFlags);
            if((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) > 0) {
                allocation.isHostVisible = true;
            } else {
                allocation.isHostVisible = false;
            }
        }
    } // namespace rapi
} // namespace cube
