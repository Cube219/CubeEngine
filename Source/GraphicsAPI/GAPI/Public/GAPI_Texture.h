#pragma once

#include "GAPIHeader.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        enum class TextureType
        {
            Texture1D,
            Texture1DArray,
            Texture2D,
            Texture2DArray,
            Texture3D,
            TextureCube,
            TextureCubeArray
        };

        struct TextureCreateInfo
        {
            ResourceUsage usage;
            ElementFormat format;
            TextureType type;
            Uint32 width;
            Uint32 height;
            Uint32 depth = 1;
            Uint32 arraySize = 0;

            const char* debugName = "Unknown";
        };

        class Texture
        {
        public:
            Texture(const TextureCreateInfo& info) :
                mUsage(info.usage),
                mFormat(info.format),
                mWidth(info.width),
                mHeight(info.height),
                mDepth(info.depth),
                mArraySize(info.arraySize)
            {}
            virtual ~Texture() = default;

            virtual void* Map() = 0;
            virtual void Unmap() = 0;

            ResourceUsage GetUsage() const { return mUsage; }
            ElementFormat GetFormat() const { return mFormat; }
            Uint32 GetWidth() const { return mWidth; }
            Uint32 GetHeight() const { return mHeight; }
            Uint32 GetDepth() const { return mDepth; }
            Uint32 GetArraySize() const { return mArraySize; }
            Uint32 GetRowPitch() const { return mRowPitch; }

        protected:
            ResourceUsage mUsage;
            ElementFormat mFormat;
            Uint32 mWidth;
            Uint32 mHeight;
            Uint32 mDepth;
            Uint32 mArraySize;
            Uint32 mRowPitch;
        };
    } // namespace gapi
} // namespace cube
