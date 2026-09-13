#pragma once

#include "GAPIHeader.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;

        enum class ASBuildPreferType
        {
            Default,
            Performance,
            BuildTime,
        };

        struct BLASCreateInfo
        {
            SharedPtr<Buffer> vertexBuffer;
            ElementFormat vertexFormat = ElementFormat::RGBA32_Float;
            Uint64 vertexStride;

            SharedPtr<Buffer> indexBuffer;

            struct GeometryInfo
            {
                Uint64 vertexOffset = 0;
                Uint64 vertexCount;
                Uint64 indexOffset = 0;
                Uint64 indexCount;

                StringView debugName;
            };
            ConstArrayView<GeometryInfo> geometryInfos;

            bool isOpaque = true;
            // bool allowUpdate = false; // TODO
            ASBuildPreferType buildPreferType;

            StringView debugName;
        };

        class BLAS
        {
        public:
            BLAS(const BLASCreateInfo& createInfo)
                : mDebugName(createInfo.debugName)
            {}
            virtual ~BLAS() = default;

            Uint64 GetScratchBufferSize() const { return mScratchBufferSize; }
            Uint64 GetUpdateScratchBufferSize() const { return mUpdateScratchBufferSize; }

            StringView GetDebugName() const { return mDebugName; }

        protected:
            Uint64 mScratchBufferSize = 0; // Set in child class
            Uint64 mUpdateScratchBufferSize = 0; // Set in child class

            String mDebugName;
        };

        struct TLASCreateInfo
        {
            StringView debugName;
        };

        class TLAS
        {
        public:
            TLAS(const TLASCreateInfo& createInfo) {}
            virtual ~TLAS() = default;
        };
    } // namespace gapi
} // namespace cube
