#pragma once

#include "VulkanAPIHeader.h"

#include <vk_mem_alloc.h>

#include "VulkanUtility.h"
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

            VmaAllocator allocator;
            VmaAllocation allocation;
            Uint64 size;
            void* pMappedPtr;
            bool isHostVisible;

            void Map()
            {
                if(pMappedPtr != nullptr) return;

                VkResult res = vmaMapMemory(allocator, allocation, &pMappedPtr);
                CHECK_VK(res, "Failed to map memory.");
            }

            void Unmap()
            {
                if(pMappedPtr == nullptr) return;

                vmaUnmapMemory(allocator, allocation);
                pMappedPtr = nullptr;
            }
        };

        class VulkanMemoryAllocator
        {
        public:
            VulkanMemoryAllocator() :
                mAllocator(VK_NULL_HANDLE)
            {}
            ~VulkanMemoryAllocator() {}

            VulkanMemoryAllocator(const VulkanMemoryAllocator& other) = delete;
            VulkanMemoryAllocator& operator=(const VulkanMemoryAllocator& rhs) = delete;

            void Initialize(VkInstance instance, VkPhysicalDevice GPU, VkDevice device);
            void Shutdown();

            VulkanAllocation Allocate(ResourceUsage usage, VkBufferCreateInfo& bufCreateInfo, VkBuffer* pBuf);
            VulkanAllocation Allocate(ResourceUsage usage, VkImageCreateInfo& imageCreateInfo, VkImage* pImage);

            void Free(VulkanAllocation alloc);

        private:
            VmaAllocationCreateInfo CreateVmaAllocationCreateInfo(ResourceUsage usage);
            void UpdateVulkanAllocation(VulkanAllocation& allocation, VmaAllocation vmaAllocation, const VmaAllocationInfo& info);

            VmaAllocator mAllocator;
        };
    } // namespace rapi
} // namespace cube
