#pragma once

#include "MetalHeader.h"

#include "GAPI_Texture.h"
#include "MetalArgumentBufferManager.h"

namespace cube
{
    class MetalDevice;

    namespace gapi
    {
        class MetalTexture : public Texture, public std::enable_shared_from_this<MetalTexture>
        {
        public:
            MetalTexture(const TextureCreateInfo& info, MetalDevice& device);
            virtual ~MetalTexture();

            virtual void* Map() override;
            virtual void Unmap() override;

            virtual SharedPtr<TextureSRV> CreateSRV(const TextureSRVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureUAV> CreateUAV(const TextureUAVCreateInfo& createInfo) override;

            id<MTLTexture> GetMTLTexture() const { return mTexture; }
            MTLPixelFormat GetPixelFormat() const { return mPixelFormat; }
            MTLTextureType GetTextureType() const { return mTextureType; }
            MTLTextureUsage GetTextureUsage() const { return mTextureUsage; }

        private:
            MetalDevice& mDevice;

            id<MTLTexture> mTexture;
            MTLPixelFormat mPixelFormat;
            MTLTextureType mTextureType;
            MTLTextureUsage mTextureUsage;

            Uint64 mTotalSize;
            void* mMappedPtr;
        };

        class MetalTextureSRV : public TextureSRV
        {
        public:
            MetalTextureSRV(const TextureSRVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device);
            virtual ~MetalTextureSRV();

        private:
            MetalDevice& mDevice;

            id<MTLTexture> mSRV;
            MetalArgumentBufferHandle mArgumentBufferHandle;
        };

        class MetalTextureUAV : public TextureUAV
        {
        public:
            MetalTextureUAV(const TextureUAVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device);
            virtual ~MetalTextureUAV();

        private:
            MetalDevice& mDevice;

            id<MTLTexture> mUAV;
            MetalArgumentBufferHandle mArgumentBufferHandle;
        };
    } // namespace gapi
} // namespace cube
