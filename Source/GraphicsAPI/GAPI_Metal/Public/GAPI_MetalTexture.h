#pragma once

#include "MetalHeader.h"

#include "GAPI_Texture.h"

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
            virtual SharedPtr<TextureRTV> CreateRTV(const TextureRTVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureDSV> CreateDSV(const TextureDSVCreateInfo& createInfo) override;

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

            id<MTLTexture> GetMTLTexture() const { return mSRV; }

        private:
            MetalDevice& mDevice;

            id<MTLTexture> mSRV;
        };

        class MetalTextureUAV : public TextureUAV
        {
        public:
            MetalTextureUAV(const TextureUAVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device);
            virtual ~MetalTextureUAV();

            id<MTLTexture> GetMTLTexture() const { return mUAV; }

        private:
            MetalDevice& mDevice;

            id<MTLTexture> mUAV;
        };

        class MetalTextureRTV : public TextureRTV
        {
        public:
            MetalTextureRTV(const TextureRTVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device);
            // Constructor for existed MTLTexture. (Used in backbuffer in swap chain)
            MetalTextureRTV(id<MTLTexture> mtlRTV, MetalDevice& device);
            ~MetalTextureRTV() override;

            id<MTLTexture> GetMTLTexture() const { return mRTV; }

        private:
            MetalDevice& mDevice;

            bool mFromExisted = false;
            id<MTLTexture> mRTV;
        };

        class MetalTextureDSV : public TextureDSV
        {
        public:
            MetalTextureDSV(const TextureDSVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device);
            virtual ~MetalTextureDSV();

            id<MTLTexture> GetMTLTexture() const { return mDSV; }

        private:
            MetalDevice& mDevice;

            id<MTLTexture> mDSV;
        };
    } // namespace gapi
} // namespace cube
