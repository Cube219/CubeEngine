#pragma once

#include "../RenderAPIHeader.h"

#include "Resource.h"

namespace cube
{
    namespace rapi
    {
        struct BufferCreateInfo : public ResourceCreateInfo
        {
            Uint64 size;
            const void* pData = nullptr;
        };

        class Buffer : public Resource
        {
        public:
            Buffer(ResourceUsage usage, Uint64 size, const char* debugName) :
                Resource(usage, debugName),
                mSize(size)
            {}
            virtual ~Buffer() {}

            Uint64 GetSize() const { return mSize; }

        protected:
            Uint64 mSize;
        };

        struct VertexBufferCreateInfo : public BufferCreateInfo
        {
        };
        class VertexBuffer : public Buffer
        {
        public:
            VertexBuffer(ResourceUsage usage, Uint64 size, const char* debugName) :
                Buffer(usage, size, debugName)
            {}
            virtual ~VertexBuffer() {}
        };

        struct IndexBufferCreateInfo : public BufferCreateInfo
        {
            enum class StrideType
            {
                Uint16,
                Uint32
            };
            StrideType strideType;
        };
        class IndexBuffer : public Buffer
        {
        public:
            IndexBuffer(ResourceUsage usage, Uint64 size, const char* debugName) :
                Buffer(usage, size, debugName)
            {}
            virtual ~IndexBuffer() {}
        };
    } // namespace rapi
} // namespace cube