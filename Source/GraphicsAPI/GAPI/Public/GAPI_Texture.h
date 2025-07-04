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

        // TODO: mipmapping
        struct TextureCreateInfo
        {
            ResourceUsage usage;
            ElementFormat format;
            TextureType type;
            Uint32 width;
            Uint32 height;
            Uint32 depth = 1;
            Uint32 arraySize = 0;
            Uint32 mipLevels = 1;

            StringView debugName;
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
                mArraySize(info.arraySize),
                mMipLevels(info.mipLevels),
                mRowPitch(0), // Set in child class
                mBindlessIndex(-1) // Set in child class
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
            Uint32 GetMipLevels() const { return mMipLevels; }
            Uint32 GetRowPitch() const { return mRowPitch; }

            int GetBindlessIndex() const { return mBindlessIndex; }

        protected:
            ResourceUsage mUsage;
            ElementFormat mFormat;
            Uint32 mWidth;
            Uint32 mHeight;
            Uint32 mDepth;
            Uint32 mArraySize;
            Uint32 mMipLevels;
            Uint32 mRowPitch;

            int mBindlessIndex;
        };
    } // namespace gapi
} // namespace cube
