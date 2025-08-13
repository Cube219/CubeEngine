#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        enum class BufferType
        {
            Vertex,
            Index,
            Constant
        };

        struct BufferCreateInfo
        {
            BufferType type;
            ResourceUsage usage;
            Uint64 size;

            StringView debugName;
        };

        class Buffer
        {
        public:
            Buffer(const BufferCreateInfo& info) :
                mType(info.type),
                mUsage(info.usage),
                mSize(info.size)
            {}
            virtual ~Buffer() = default;

            virtual void* Map() = 0;
            virtual void Unmap() = 0;

            virtual void SetDebugName(StringView debugName) {}

            BufferType GetType() const { return mType; }
            ResourceUsage GetUsage() const { return mUsage; }
            Uint64 GetSize() const { return mSize; }

        protected:
            BufferType mType;
            ResourceUsage mUsage;
            Uint64 mSize;
        };
    } // namespace gapi
} // namespace cube
