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
        class TextureRTV;
        struct TextureRTVCreateInfo;
        class TextureDSV;
        struct TextureDSVCreateInfo;

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
            UAV = 1 << 1,
            DepthStencil = 1 << 2
        };
        using TextureFlags = Flags<TextureFlag>;
        FLAGS_OPERATOR(TextureFlag);

        struct TextureInfo
        {
            ElementFormat format;
            TextureType type;
            TextureFlags flags;
            Uint32 width;
            Uint32 height;
            Uint32 depth = 1;
            Uint32 arraySize = 1;
            Uint32 mipLevels = 1;
        };

        struct TextureCreateInfo
        {
            ResourceUsage usage;
            TextureInfo textureInfo;

            StringView debugName;
        };

        class Texture
        {
        public:
            Texture(const TextureCreateInfo& createInfo)
                : mUsage(createInfo.usage)
                , mInfo(createInfo.textureInfo)
            {}
            virtual ~Texture() = default;

            virtual void* Map() = 0;
            virtual void Unmap() = 0;

            ResourceUsage GetUsage() const { return mUsage; }
            TextureType GetType() const { return mInfo.type; }
            ElementFormat GetFormat() const { return mInfo.format; }
            Uint32 GetWidth() const { return mInfo.width; }
            Uint32 GetHeight() const { return mInfo.height; }
            Uint32 GetDepth() const { return mInfo.depth; }
            Uint32 GetArraySize() const { return mInfo.arraySize; }
            Uint32 GetMipLevels() const { return mInfo.mipLevels; }

            Uint32 GetNumSlices() const { return IsCubemap() ? mInfo.arraySize * 6 : mInfo.arraySize; }
            bool IsCubemap() const { return (mInfo.type == TextureType::TextureCube) || (mInfo.type == TextureType::TextureCubeArray); }

            Uint32 GetNumSubresources() const { return mSubresourceLayouts.size(); }
            Uint32 GetSubresourceIndex(Uint32 sliceIndex, Uint32 mipLevel) const { return sliceIndex * mInfo.mipLevels + mipLevel; }
            const SubresourceLayout& GetSubresourceLayout(Uint32 subresourceIndex) const { return mSubresourceLayouts[subresourceIndex]; }

            virtual SharedPtr<TextureSRV> CreateSRV(const TextureSRVCreateInfo& createInfo) = 0;
            virtual SharedPtr<TextureUAV> CreateUAV(const TextureUAVCreateInfo& createInfo) = 0;
            virtual SharedPtr<TextureRTV> CreateRTV(const TextureRTVCreateInfo& createInfo) = 0;
            virtual SharedPtr<TextureDSV> CreateDSV(const TextureDSVCreateInfo& createInfo) = 0;

        protected:
            ResourceUsage mUsage;
            TextureInfo mInfo;

            Vector<SubresourceLayout> mSubresourceLayouts; // Set in child class
        };

        struct TextureSRVCreateInfo
        {
            // [firstMipLevel, firstMipLevel + mipLevels - 1)]
            Uint32 firstMipLevel = 0;
            Int32 mipLevels = -1;

            // [firstSliceIndex, firstSliceIndex + sliceSize - 1)]
            Uint32 firstSliceIndex = 0;
            Int32 sliceSize = -1;
        };

        class TextureSRV
        {
        public:
            TextureSRV(const TextureSRVCreateInfo& createInfo, SharedPtr<Texture> texture) :
                mTexture(texture),
                mSubresourceRange({
                    .firstMipLevel = createInfo.firstMipLevel,
                    .mipLevels = createInfo.mipLevels < 0 ? texture->GetMipLevels() - createInfo.firstMipLevel : static_cast<Uint32>(createInfo.mipLevels),
                    .firstSliceIndex = createInfo.firstSliceIndex,
                    .sliceSize = createInfo.sliceSize < 0 ? texture->GetNumSlices() - createInfo.firstSliceIndex : static_cast<Uint32>(createInfo.sliceSize)
                }),
                mBindlessId(-1) // Set in child class
            {}
            virtual ~TextureSRV() {}

            Uint32 GetFirstMipLevel() const { return mSubresourceRange.firstMipLevel; }
            Uint32 GetMipLevels() const { return mSubresourceRange.mipLevels; }
            Uint32 GetFirstSliceIndex() const { return mSubresourceRange.firstSliceIndex; }
            Uint32 GetSliceSize() const { return mSubresourceRange.sliceSize; }
            SubresourceRange GetSubresourceRange() const { return mSubresourceRange; }

            Uint64 GetBindlessId() const { return mBindlessId; }

        protected:
            SharedPtr<Texture> mTexture;

            SubresourceRange mSubresourceRange;

            Uint64 mBindlessId;
        };

        struct TextureUAVCreateInfo
        {
            Uint32 mipLevel = 0;

            // [firstSliceIndex, firstSliceIndex + sliceSize - 1)]
            Uint32 firstSliceIndex = 0;
            Int32 sliceSize = -1;

            // [firstDepthIndex, firstDepthIndex + depthSize - 1)]
            Uint32 firstDepthIndex = 0;
            Int32 depthSize = -1;
        };

        class TextureUAV
        {
        public:
            TextureUAV(const TextureUAVCreateInfo& createInfo, SharedPtr<Texture> texture) :
                mTexture(texture),
                mSubresourceRange({
                    .firstMipLevel = createInfo.mipLevel,
                    .mipLevels = 1,
                    .firstSliceIndex = createInfo.firstSliceIndex,
                    .sliceSize = createInfo.sliceSize < 0 ? texture->GetNumSlices() - createInfo.firstSliceIndex : static_cast<Uint32>(createInfo.sliceSize)
                }),
                mFirstDepthIndex(createInfo.firstDepthIndex),
                mDepthSize(createInfo.depthSize < 0 ? texture->GetDepth() - createInfo.firstDepthIndex : static_cast<Uint32>(createInfo.depthSize)),
                mBindlessId(-1) // Set in child class
            {}
            virtual ~TextureUAV() {}

            Uint32 GetMipLevel() const { return mSubresourceRange.firstMipLevel; }
            Uint32 GetFirstSliceIndex() const { return mSubresourceRange.firstSliceIndex; }
            Uint32 GetSliceSize() const { return mSubresourceRange.sliceSize; }
            Uint32 GetFirstDepthIndex() const { return mFirstDepthIndex; }
            Uint32 GetDepthSize() const { return mDepthSize; }
            SubresourceRange GetSubresourceRange() const { return mSubresourceRange; }

            Uint64 GetBindlessId() const { return mBindlessId; }

        protected:
            SharedPtr<Texture> mTexture;

            SubresourceRange mSubresourceRange;
            Uint32 mFirstDepthIndex;
            Uint32 mDepthSize;

            Uint64 mBindlessId;
        };

        struct TextureRTVCreateInfo
        {
            Uint32 mipLevel = 0;

            Uint32 firstSliceIndex = 0;
            Int32 sliceSize = -1;

            Uint32 firstDepthIndex = 0;
            Int32 depthSize = -1;
        };

        class TextureRTV
        {
        public:
            TextureRTV(const TextureRTVCreateInfo& createInfo, SharedPtr<Texture> texture)
                : mTexture(texture)
                , mSubresourceRange({
                    .firstMipLevel = createInfo.mipLevel,
                    .mipLevels = 1,
                    .firstSliceIndex = createInfo.firstSliceIndex,
                    .sliceSize = createInfo.sliceSize < 0 ? texture->GetNumSlices() - createInfo.firstSliceIndex : static_cast<Uint32>(createInfo.sliceSize)
                })
                , mFirstDepthIndex(createInfo.firstDepthIndex)
                , mDepthSize(createInfo.depthSize < 0 ? texture->GetDepth() - createInfo.firstDepthIndex : static_cast<Uint32>(createInfo.depthSize))
            {}
            TextureRTV()
                : mTexture(nullptr)
                , mSubresourceRange({ .firstMipLevel = 0, .mipLevels = 1, .firstSliceIndex = 0, .sliceSize = 1 })
                , mFirstDepthIndex(0)
                , mDepthSize(1)
            {}
            virtual ~TextureRTV() {}

            Uint32 GetMipLevel() const { return mSubresourceRange.firstMipLevel; }
            Uint32 GetFirstSliceIndex() const { return mSubresourceRange.firstSliceIndex; }
            Uint32 GetSliceSize() const { return mSubresourceRange.sliceSize; }
            Uint32 GetFirstDepthIndex() const { return mFirstDepthIndex; }
            Uint32 GetDepthSize() const { return mDepthSize; }
            SubresourceRange GetSubresourceRange() const { return mSubresourceRange; }

        protected:
            SharedPtr<Texture> mTexture;

            SubresourceRange mSubresourceRange;
            Uint32 mFirstDepthIndex;
            Uint32 mDepthSize;
        };

        struct TextureDSVCreateInfo
        {
            Uint32 mipLevel = 0;

            Uint32 firstSliceIndex = 0;
            Int32 sliceSize = -1;
        };

        class TextureDSV
        {
        public:
            TextureDSV(const TextureDSVCreateInfo& createInfo, SharedPtr<Texture> texture)
                : mTexture(texture)
                , mSubresourceRange({
                    .firstMipLevel = createInfo.mipLevel,
                    .mipLevels = 1,
                    .firstSliceIndex = createInfo.firstSliceIndex,
                    .sliceSize = createInfo.sliceSize < 0 ? texture->GetNumSlices() - createInfo.firstSliceIndex : static_cast<Uint32>(createInfo.sliceSize)
                })
            {}
            virtual ~TextureDSV() {}

            Uint32 GetMipLevel() const { return mSubresourceRange.firstMipLevel; }
            Uint32 GetFirstSliceIndex() const { return mSubresourceRange.firstSliceIndex; }
            Uint32 GetSliceSize() const { return mSubresourceRange.sliceSize; }
            SubresourceRange GetSubresourceRange() const { return mSubresourceRange; }

        protected:
            SharedPtr<Texture> mTexture;

            SubresourceRange mSubresourceRange;
        };
    } // namespace gapi
} // namespace cube
