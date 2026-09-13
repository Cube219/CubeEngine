#pragma once

#include "GAPIHeader.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;

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
            bool allowUpdate = false;

            StringView debugName;
        };

        class BLAS
        {
        public:
            BLAS(const BLASCreateInfo& createInfo) {}
            virtual ~BLAS() = default;
        };

        struct TLASCreateInfo
        {
            const StringView debugName;
        };

        class TLAS
        {
        public:
            TLAS(const TLASCreateInfo& createInfo) {}
            virtual ~TLAS() = default;
        };
    } // namespace gapi
} // namespace cube
