#pragma once

#include "GAPIHeader.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        class TextureSRV;
        struct TextureSRVCreateInfo;
        class TextureUAV;
        struct TextureUAVCreateInfo;

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

        enum class TextureFlag
        {
            None = 0,
            RenderTarget = 1 << 0,
            UAV = 1 << 1
        };
        using TextureFlags = Flags<TextureFlag>;
        FLAGS_OPERATOR(TextureFlag);

        struct TextureCreateInfo
        {
            ResourceUsage usage;
            ElementFormat format;
            TextureType type;
            TextureFlags flags;
            Uint32 width;
            Uint32 height;
            Uint32 depth = 1;
            Uint32 arraySize = 1;
            Uint32 mipLevels = 1;

            StringView debugName;
        };

        class Texture
        {
        public:
            Texture(const TextureCreateInfo& info) :
                mUsage(info.usage),
                mType(info.type),
                mFormat(info.format),
                mWidth(info.width),
                mHeight(info.height),
                mDepth(info.depth),
                mArraySize(info.arraySize),
                mMipLevels(info.mipLevels),
                mRowPitch(0) // Set in child class
            {}
            virtual ~Texture() = default;

            virtual void* Map() = 0;
            virtual void Unmap() = 0;

            ResourceUsage GetUsage() const { return mUsage; }
            TextureType GetType() const { return mType; }
            ElementFormat GetFormat() const { return mFormat; }
            Uint32 GetWidth() const { return mWidth; }
            Uint32 GetHeight() const { return mHeight; }
            Uint32 GetDepth() const { return mDepth; }
            Uint32 GetArraySize() const { return mArraySize; }
            Uint32 GetMipLevels() const { return mMipLevels; }
            Uint32 GetRowPitch() const { return mRowPitch; }

            virtual SharedPtr<TextureSRV> CreateSRV(const TextureSRVCreateInfo& createInfo) = 0;
            virtual SharedPtr<TextureUAV> CreateUAV(const TextureUAVCreateInfo& createInfo) = 0;

        protected:
            ResourceUsage mUsage;
            TextureType mType;
            ElementFormat mFormat;
            Uint32 mWidth;
            Uint32 mHeight;
            Uint32 mDepth;
            Uint32 mArraySize;
            Uint32 mMipLevels;
            Uint32 mRowPitch;
        };

        struct TextureSRVCreateInfo
        {
            Uint32 firstMipLevel = 0;
            Int32 mipLevels = -1;

            Uint32 firstArrayIndex = 0;
            Int32 arraySize = -1;
        };

        class TextureSRV
        {
        public:
            TextureSRV(const TextureSRVCreateInfo& createInfo, SharedPtr<Texture> texture) :
                mTexture(texture),
                mFirstMipLevel(createInfo.firstMipLevel),
                mMipLevels(createInfo.mipLevels > 0 ? createInfo.mipLevels : texture->GetMipLevels() - createInfo.firstMipLevel),
                mFirstArrayIndex(createInfo.firstArrayIndex),
                mArraySize(createInfo.arraySize > 0 ? createInfo.arraySize : texture->GetArraySize() - createInfo.firstArrayIndex),
                mBindlessIndex(-1) // Set in child class
            {}
            virtual ~TextureSRV() {}

            Uint32 GetFirstMipLevel() const { return mFirstMipLevel; }
            Uint32 GetMipLevels() const { return mMipLevels; }
            Uint32 GetFirstArrayIndex() const { return mFirstArrayIndex; }
            Uint32 GetArraySize() const { return mArraySize; }
            SubresourceRange GetSubresourceRange() const
            {
                return {
                    .firstMipLevel = mFirstMipLevel,
                    .mipLevels = mMipLevels,
                    .firstArrayIndex = mFirstArrayIndex,
                    .arraySize = mArraySize
                };
            }

            int GetBindlessIndex() const { return mBindlessIndex; }

        protected:
            SharedPtr<Texture> mTexture;

            Uint32 mFirstMipLevel;
            Uint32 mMipLevels;
            Uint32 mFirstArrayIndex;
            Uint32 mArraySize;

            int mBindlessIndex;
        };

        struct TextureUAVCreateInfo
        {
            Uint32 mipLevel = 0;

            Uint32 firstArrayIndex = 0;
            Int32 arraySize = -1;

            Uint32 firstDepthIndex = 0;
            Int32 DepthSize = -1;
        };

        class TextureUAV
        {
        public:
            TextureUAV(const TextureUAVCreateInfo& createInfo, SharedPtr<Texture> texture) :
                mTexture(texture),
                mMipLevel(createInfo.mipLevel),
                mFirstArrayIndex(createInfo.firstArrayIndex),
                mArraySize(createInfo.arraySize > 0 ? createInfo.arraySize : texture->GetArraySize() - createInfo.firstArrayIndex),
                mFirstDepthIndex(createInfo.firstDepthIndex),
                mDepthSize(createInfo.DepthSize > 0 ? createInfo.DepthSize : texture->GetDepth() - createInfo.firstDepthIndex),
                mBindlessIndex(-1) // Set in child class
            {}
            virtual ~TextureUAV() {}

            Uint32 GetMipLevel() const { return mMipLevel; }
            Uint32 GetFirstArrayIndex() const { return mFirstArrayIndex; }
            Uint32 GetArraySize() const { return mArraySize; }
            Uint32 GetFirstDepthIndex() const { return mFirstDepthIndex; }
            Uint32 GetDepthSize() const { return mDepthSize; }
            SubresourceRange GetSubresourceRange() const
            {
                return {
                    .firstMipLevel = mMipLevel,
                    .mipLevels = 1,
                    .firstArrayIndex = mFirstArrayIndex,
                    .arraySize = mArraySize
                };
            }

            int GetBindlessIndex() const { return mBindlessIndex; }

        protected:
            SharedPtr<Texture> mTexture;

            Uint32 mMipLevel;
            Uint32 mFirstArrayIndex;
            Uint32 mArraySize;
            Uint32 mFirstDepthIndex;
            Uint32 mDepthSize;

            int mBindlessIndex;
        };
    } // namespace gapi
} // namespace cube
