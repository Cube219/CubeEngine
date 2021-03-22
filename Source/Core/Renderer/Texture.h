#pragma once

#include "../CoreHeader.h"

#include "RenderObject.h"
#include "../Resource/Resource.h"

#include "RenderAPIs/RenderAPI/Interface/Texture.h"

namespace cube
{
    // TODO: texture 정보 더 추가(usage, array, format)
    class CORE_EXPORT TextureData
    {
    public:
        TextureData();
        TextureData(Uint32 width, Uint32 height, Uint32 depth, Uint32 mipLevel, Uint64 size, void* pData);
        ~TextureData();

        TextureData(const TextureData& other) = delete;
        TextureData& operator=(const TextureData& rhs) = delete;

        Uint32 GetWidth() const { return mWidth; }
        Uint32 GetHeight() const { return mHeight; }
        Uint32 GetDepth() const { return mDepth; }
        Uint32 GetMipLevels() const { return mMipLevels; }
        void* GetData() const { return mpData; }
        Uint64 GetDataSize() const { return mSize; }

    private:
        Uint32 mWidth;
        Uint32 mHeight;
        Uint32 mDepth;
        Uint32 mMipLevels;

        void* mpData;
        Uint64 mSize;
    };

    class CORE_EXPORT Texture : public Resource, public RenderObject
    {
    public:
        static RPtr<Texture> Load(StringView path);

    public:
        Texture(const SPtr<TextureData>& textureData);
        virtual ~Texture();

        virtual SPtr<rt::RenderObject> CreateRTObject() override;

    private:
        SPtr<TextureData> mTextureData;
    };

    namespace rt
    {
        class CORE_EXPORT Texture : public RenderObject
        {
        public:
            Texture(const SPtr<TextureData>& textureData);
            virtual ~Texture();

            void OnCreate() override;
            void OnDestroy() override;

            SPtr<rapi::Texture> GetGPUTexture() const { return mGPUTexture; }
            SPtr<rapi::TextureView> GetGPUTextureView() const { return mGPUTextureView; }
            SPtr<rapi::Sampler> GetSampler() const { return mSampler; }

        private:
            void FlushToGPU();

            SPtr<TextureData> mTextureData;

            SPtr<rapi::Texture> mGPUTexture;
            SPtr<rapi::TextureView> mGPUTextureView;
            SPtr<rapi::Sampler> mSampler; // TODO: custom sampler 기능 추가
        };
    } // namespace rt
} // namespace cube
