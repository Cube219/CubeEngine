#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Buffer.h"

#include "../VulkanMemoryAllocator.h"
#include "../VulkanStagingManager.h"

namespace cube
{
    namespace rapi
    {
        class BufferVk
        {
        public:
            BufferVk(VulkanDevice& device, ResourceUsage usage, VkBufferUsageFlags bufUsage, Uint64 size, const void* pData, const char* debugName);
            virtual ~BufferVk();

            void MapImpl(ResourceMapType type, void*& pMappedResource);
            void UnmapImpl();

        protected:
            VulkanDevice& mDevice;

            VkBuffer mBuffer;
            VulkanAllocation mAllocation;
            VulkanStagingBuffer mStagingBuffer;

            ResourceUsage mUsage;
            Uint64 mSize;
            const char* mDebugName;
        };

        class VertexBufferVk : public VertexBuffer, public BufferVk
        {
        public:
            VertexBufferVk(VulkanDevice& device, const VertexBufferCreateInfo& info);
            virtual ~VertexBufferVk() {}

            virtual void Map(ResourceMapType type, void*& pMappedResource) override
            {
                MapImpl(type, pMappedResource);
            }
            virtual void Unmap() override
            {
                UnmapImpl();
            }
        };

        class IndexBufferVk : public IndexBuffer, public BufferVk
        {
        public:
            IndexBufferVk(VulkanDevice& device, const IndexBufferCreateInfo& info);
            virtual ~IndexBufferVk() {}

            virtual void Map(ResourceMapType type, void*& pMappedResource) override
            {
                MapImpl(type, pMappedResource);
            }
            virtual void Unmap() override
            {
                UnmapImpl();
            }

            VkIndexType GetIndexType() const { return mIndexType; }

        private:
            VkIndexType mIndexType;
        };

        class ConstantBufferVk : public ConstantBuffer, public BufferVk
        {
        public:
            ConstantBufferVk(VulkanDevice& device, const ConstantBufferCreateInfo& info);
            virtual ~ConstantBufferVk() {}

            virtual void Map(ResourceMapType type, void*& pMappedResource) override
            {
                MapImpl(type, pMappedResource);
            }
            virtual void Unmap() override
            {
                UnmapImpl();
            }
        };

        class StructuredBufferVk : public StructuredBuffer, public BufferVk
        {
        public:
            StructuredBufferVk(VulkanDevice& device, const StructuredBufferCreateInfo& info);
            virtual ~StructuredBufferVk() {}

            virtual void Map(ResourceMapType type, void*& pMappedResource) override
            {
                MapImpl(type, pMappedResource);
            }
            virtual void Unmap() override
            {
                UnmapImpl();
            }
        };
    } // namespace rapi
} // namespace cube
