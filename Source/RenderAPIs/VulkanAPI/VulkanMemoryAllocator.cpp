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
            VulkanAllocation alloc;
            alloc.resourceType = VulkanAllocation::ResourceType::Buffer;
            alloc.pResource = pBuf;

            VmaAllocationCreateInfo info = {};
            switch(usage)
            {
                case ResourceUsage::Default:
                case ResourceUsage::Immutable:
                    info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                    break;
                case ResourceUsage::Dynamic:
                    info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                    break;
                case ResourceUsage::Staging:
                    info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
                    break;
                default:
                    ASSERTION_FAILED("Invalid resource type {}", (int)usage);
                    break;
            }

            vmaCreateBuffer(mAllocator, &bufCreateInfo, &info, pBuf, &(alloc.allocation), nullptr);
            
            return alloc;
        }

        VulkanAllocation VulkanMemoryAllocator::Allocate(ResourceUsage usage, VkImageCreateInfo& imageCreateInfo, VkImage* pImage)
        {
            VulkanAllocation alloc;
            alloc.resourceType = VulkanAllocation::ResourceType::Texture;
            alloc.pResource = pImage;

            VmaAllocationCreateInfo info = {};
            switch(usage)
            {
                case ResourceUsage::Default:
                case ResourceUsage::Immutable:
                    info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                    break;
                case ResourceUsage::Dynamic:
                    info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                    break;
                case ResourceUsage::Staging:
                    info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
                    break;
                default:
                    ASSERTION_FAILED("Invalid resource type. ({})", (int)usage);
                    break;
            }

            vmaCreateImage(mAllocator, &imageCreateInfo, &info, pImage, &(alloc.allocation), nullptr);

            return alloc;
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
    } // namespace rapi
} // namespace cube
