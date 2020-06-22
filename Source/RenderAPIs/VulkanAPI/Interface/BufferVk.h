#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Buffer.h"

#include "../VulkanMemoryAllocator.h"

namespace cube
{
    namespace rapi
    {
        class BufferVk
        {
        public:
            BufferVk(VulkanDevice& device, ResourceUsage usage, VkBufferUsageFlags bufUsage, Uint64 size, const void* pData, const char* debugName);
            virtual ~BufferVk();

        protected:
            VulkanDevice& mDevice;

            VkBuffer mBuffer;

            VulkanAllocation mAllocation;
        };

        class VertexBufferVk : public VertexBuffer, public BufferVk
        {
        public:
            VertexBufferVk(VulkanDevice& device, const VertexBufferCreateInfo& info);
            virtual ~VertexBufferVk() {}
        };

        class IndexBufferVk : public IndexBuffer, public BufferVk
        {
        public:
            IndexBufferVk(VulkanDevice& device, const IndexBufferCreateInfo& info);
            virtual ~IndexBufferVk() {}

            VkIndexType GetIndexType() const { return mIndexType; }

        private:
            VkIndexType mIndexType;
        };
    } // namespace rapi
} // namespace cube
