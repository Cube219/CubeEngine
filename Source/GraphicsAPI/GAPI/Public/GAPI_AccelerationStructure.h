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
            Uint32 vertexOffset = 0;
            Uint32 vertexCount;
            ElementFormat vertexFormat = ElementFormat::RGBA32_Float;

            SharedPtr<Buffer> indexBuffer;
            Uint32 indexOffset = 0;
            Uint32 indexCount;
            ElementFormat indexFormat = ElementFormat::R32_UInt;

            StringView debugName;
        };

        class BLAS
        {
        public:
            BLAS(const BLASCreateInfo& createInfo) {}
            virtual ~BLAS() = default;
        };
    } // namespace gapi
} // namespace cube
