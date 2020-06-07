#pragma once

#include "VulkanAPIHeader.h"

#include <vk_mem_alloc.h>

#include "RenderAPIs/RenderAPI/Interface/RenderTypes.h"

namespace cube
{
    namespace rapi
    {
        struct VulkanAllocation
        {
            enum class ResourceType
            {
                Buffer, Texture
            };
            ResourceType resourceType;
            void* pResource;
            VmaAllocation allocation;
        };

        class VulkanMemoryAllocator
        {
        public:
            VulkanMemoryAllocator() {}
            ~VulkanMemoryAllocator() {}

            void Initialize(VkInstance instance, VkPhysicalDevice GPU, VkDevice device);
            void Shutdown();

            VulkanAllocation Allocate(ResourceUsage usage, VkBufferCreateInfo& bufCreateInfo, VkBuffer* pBuf);
            VulkanAllocation Allocate(ResourceUsage usage, VkImageCreateInfo& imageCreateInfo, VkImage* pImage);

            void Free(VulkanAllocation alloc);

        private:
            VmaAllocator mAllocator;
        };
    } // namespace rapi
} // namespace cube
