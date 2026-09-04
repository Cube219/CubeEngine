#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        class BufferSRV;
        struct BufferSRVCreateInfo;
        class BufferUAV;
        struct BufferUAVCreateInfo;

        enum class BufferType
        {
            Typed,
            Structured,
            Raw,
            Constant
        };

        enum class BufferFlag
        {
            None = 0,
            UAV = 1 << 0,
        };
        using BufferFlags = Flags<BufferFlag>;
        FLAGS_OPERATOR(BufferFlag);

        struct BufferInfo
        {
            BufferType type;
            Uint64 size;
            Uint32 stride = 1;
            BufferFlags flags = BufferFlag::None;
        };

        struct BufferCreateInfo
        {
            ResourceUsage usage;
            BufferInfo bufferInfo;

            StringView debugName;
        };

        class Buffer
        {
        public:
            Buffer(const BufferCreateInfo& info) :
                mUsage(info.usage),
                mInfo(info.bufferInfo)
            {}
            virtual ~Buffer() = default;

            virtual SharedPtr<BufferSRV> CreateSRV(const BufferSRVCreateInfo& createInfo) = 0;
            virtual SharedPtr<BufferUAV> CreateUAV(const BufferUAVCreateInfo& createInfo) = 0;

            virtual void* Map() = 0;
            virtual void Unmap() = 0;

            virtual void SetDebugName(StringView debugName) { mDebugName = debugName; }

            ResourceUsage GetUsage() const { return mUsage; }
            const BufferInfo& GetInfo() const { return mInfo; }
            BufferType GetType() const { return mInfo.type; }
            Uint64 GetSize() const { return mInfo.size; }
            Uint32 GetStride() const { return mInfo.stride; }
            Uint64 GetNumElements() const { return mInfo.size / mInfo.stride; }
            BufferFlags GetFlags() const { return mInfo.flags; }

            StringView GetDebugName() const { return mDebugName; }

        protected:
            ResourceUsage mUsage;
            BufferInfo mInfo;

            String mDebugName;
        };

        struct BufferSRVCreateInfo
        {
            ElementFormat typedFormat = ElementFormat::Unknown;

            Uint64 firstElement = 0;
            Uint64 numElements = std::numeric_limits<Uint64>::max();
        };

        class BufferSRV
        {
        public:
            BufferSRV(const BufferSRVCreateInfo& createInfo, SharedPtr<Buffer> buffer)
                : mBuffer(buffer)
                , mFirstElement(createInfo.firstElement)
                , mNumElements(std::min(createInfo.numElements, buffer->GetNumElements() - createInfo.firstElement))
            {}
            virtual ~BufferSRV() = default;

            Uint64 GetFirstElement() const { return mFirstElement; }
            Uint64 GetNumElements() const { return mNumElements; }
            Uint64 GetOffset() const { return mFirstElement * mBuffer->GetStride(); }
            Uint64 GetSize() const { return mNumElements * mBuffer->GetStride(); }

            SharedPtr<Buffer> GetBuffer() const { return mBuffer; }

            Uint64 GetBindlessId() const { return mBindlessId; }

        protected:
            SharedPtr<Buffer> mBuffer;

            Uint64 mFirstElement;
            Uint64 mNumElements;

            Uint64 mBindlessId;
        };

        struct BufferUAVCreateInfo
        {
            ElementFormat typedFormat;

            Uint64 firstElement = 0;
            Uint64 numElements = std::numeric_limits<Uint64>::max();
        };

        class BufferUAV
        {
        public:
            BufferUAV(const BufferUAVCreateInfo& createInfo, SharedPtr<Buffer> buffer)
                : mBuffer(buffer)
                , mFirstElement(createInfo.firstElement)
                , mNumElements(std::min(createInfo.numElements, buffer->GetNumElements() - createInfo.firstElement))
            {}
            virtual ~BufferUAV() = default;

            Uint64 GetFirstElement() const { return mFirstElement; }
            Uint64 GetNumElements() const { return mNumElements; }
            Uint64 GetOffset() const { return mFirstElement * mBuffer->GetStride(); }
            Uint64 GetSize() const { return mNumElements * mBuffer->GetStride(); }

            Uint64 GetBindlessId() const { return mBindlessId; }

        protected:
            SharedPtr<Buffer> mBuffer;

            Uint64 mFirstElement;
            Uint64 mNumElements;

            Uint64 mBindlessId;
        };
    } // namespace gapi
} // namespace cube
