#pragma once

#include "../RenderAPIHeader.h"

#include "Resource.h"

namespace cube
{
    namespace rapi
    {
        struct TextureCreateInfo : public ResourceCreateInfo
        {
            const void* pData = nullptr;
            TextureFormat format;
            TextureBindTypeFlags bindTypeFlags;
            Uint32 mipLevels;
            Uint32 samplesNum;
        };
        class Texture : public Resource
        {
        public:
            Texture(ResourceUsage usage,
                Uint64 size, const void* pData, TextureFormat format, TextureBindTypeFlags bindTypeFlags, Uint32 mipLevels, Uint32 samplesNum, const char* debugName) :
                Resource(usage, debugName),
                mFormat(format), mBindTypeFlags(bindTypeFlags),  mMipLevels(mipLevels), mSamplesNum(samplesNum)
            {}
            virtual ~Texture() {}

            TextureFormat GetFormat() const { return mFormat; }
            Uint32 GetMipLevels() const { return mMipLevels; }

        protected:
            TextureFormat mFormat;
            TextureBindTypeFlags mBindTypeFlags;
            Uint32 mMipLevels;
            Uint32 mSamplesNum;
        };

        struct Texture2DCreateInfo : public TextureCreateInfo
        {
            Uint32 width;
            Uint32 height;
        };
        class Texture2D : public Texture
        {
        public:
            Texture2D(ResourceUsage usage,
                Uint32 width, Uint32 height,
                TextureFormat format, TextureBindTypeFlags bindTypeFlags, Uint32 mipLevels, Uint32 samplesNum, const char* debugName) :
                Texture(usage, format, bindTypeFlags, mipLevels, samplesNum, debugName),
                mWidth(width), mHeight(height)
            {}
            virtual ~Texture2D() {}

            Uint32 GetWidth() const { return mWidth; }
            Uint32 GetHeight() const { return mHeight; }

        protected:
            Uint32 mWidth;
            Uint32 mHeight;
        };

        struct Texture2DArrayCreateInfo : public Texture2DCreateInfo
        {
            Uint32 arraySize;
        };
        class Texture2DArray : public Texture2D
        {
        public:
            Texture2DArray(ResourceUsage usage,
                Uint32 arraySize,
                Uint32 width, Uint32 height, TextureFormat format, TextureBindTypeFlags bindTypeFlags, Uint32 mipLevels, Uint32 samplesNum, const char* debugName) :
                Texture2D(usage, width, height, format, bindTypeFlags, mipLevels, samplesNum, debugName),
                mArraySize(arraySize)
            {}
            virtual ~Texture2DArray() {}

            Uint32 GetArraySize() const { return mArraySize; }

        protected:
            Uint32 mArraySize;
        };

        struct Texture3DCreateInfo : public TextureCreateInfo
        {
            Uint32 width;
            Uint32 height;
            Uint32 depth;
        };
        class Texture3D : public Texture
        {
        public:
            Texture3D(ResourceUsage usage,
                Uint32 width, Uint32 height, Uint32 depth,
                TextureFormat format, TextureBindTypeFlags bindTypeFlags, Uint32 mipLevels, Uint32 samplesNum, const char* debugName) :
                Texture(usage, format, bindTypeFlags, mipLevels, samplesNum, debugName),
                mWidth(width), mHeight(height), mDepth(depth)
            {}
            virtual ~Texture3D() {}

            Uint32 GetWidth() const { return mWidth; }
            Uint32 GetHeight() const { return mHeight; }
            Uint32 GetDepth() const { return mDepth; }

        protected:
            Uint32 mWidth;
            Uint32 mHeight;
            Uint32 mDepth;
        };

        struct TextureCubeCreateInfo : public TextureCreateInfo
        {
            Uint32 size;
        };
        class TextureCube : public Texture
        {
        public:
            TextureCube(ResourceUsage usage,
                Uint32 size, TextureFormat format, TextureBindTypeFlags bindTypeFlags, Uint32 mipLevels, Uint32 samplesNum, const char* debugName) :
                Texture(usage, format, bindTypeFlags, mipLevels, samplesNum, debugName),
                mSize(size)
            {}
            virtual ~TextureCube() {}

            Uint32 GetSize() const { return mSize; }

        protected:
            Uint32 mSize;
        };
    } // namespace rapi
} // namespace cube
