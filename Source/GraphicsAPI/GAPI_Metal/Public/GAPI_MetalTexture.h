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
            MetalTexture(const TextureCreateInfo& createInfo, MetalDevice& device, bool skipResourceCreation = false);
            virtual ~MetalTexture();

            virtual void* Map() override;
            virtual void Unmap() override;

            virtual SharedPtr<TextureSRV> CreateSRV(const TextureSRVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureUAV> CreateUAV(const TextureUAVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureRTV> CreateRTV(const TextureRTVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureDSV> CreateDSV(const TextureDSVCreateInfo& createInfo) override;

            id<MTLTexture> GetMTLTexture() const { return mMTLTexture; }
            MTLPixelFormat GetPixelFormat() const { return mMTLPixelFormat; }
            MTLTextureType GetMTLTextureType() const { return mMTLTextureType; }
            MTLTextureUsage GetMTLTextureUsage() const { return mMTLTextureUsage; }

        protected:
            MetalDevice& mDevice;

            id<MTLTexture> mMTLTexture;
            MTLPixelFormat mMTLPixelFormat;
            MTLTextureType mMTLTextureType;
            MTLTextureUsage mMTLTextureUsage;

            Uint64 mTotalSize;
            void* mMappedPtr;

            bool mFromExisted;
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
            ~MetalTextureRTV() override;

            id<MTLTexture> GetMTLTexture() const { return mRTV; }

        private:
            MetalDevice& mDevice;

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
