#pragma once

#include "CoreHeader.h"

#include "ShaderParameter.h"

namespace cube
{
    namespace gapi
    {
        class Texture;
        class TextureSRV;
        class TextureUAV;
        class TextureRTV;
        class TextureDSV;
    } // namespace gapi

    class RGBuilder;

    // ===== Resources =====

    class RGResource
    {
    protected:
        friend class RGBuilder;

        RGResource(int index);
        virtual ~RGResource() = default;

        bool mIsTransient;

        int mIndex;
        int mBeginPass;
        int mEndPass;
    };

    class RGTexture : public RGResource
    {
    protected:
        friend class RGBuilder;

        RGTexture(int index, SharedPtr<gapi::Texture> texture);
        virtual ~RGTexture() = default;

        SharedPtr<gapi::Texture> mTexture;
    };

    class RGTextureView : public RGResource
    {
    public:
        Uint64 GetSubresourceHashKey() const { return mSubresourceHashKey; }

    protected:
        friend class RGBuilder;

        RGTextureView(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureView() = default;

        RGTexture* mRGTexture;
        Uint32 mMipLevel;
        Uint64 mSubresourceHashKey;
    };

    class RGTextureSRV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureSRV> GetSRV() const
        {
            CHECK(mSRV);
            return mSRV;
        }

    private:
        friend class RGBuilder;

        RGTextureSRV(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureSRV() = default;

        SharedPtr<gapi::TextureSRV> mSRV;
    };

    class RGTextureUAV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureUAV> GetUAV() const
        {
            CHECK(mUAV);
            return mUAV;
        }

    private:
        friend class RGBuilder;

        RGTextureUAV(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureUAV() = default;

        SharedPtr<gapi::TextureUAV> mUAV;
    };

    class RGTextureRTV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureRTV> GetRTV() const
        {
            CHECK(mRTV);
            return mRTV;
        }

    private:
        friend class RGBuilder;

        RGTextureRTV(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureRTV() = default;

        SharedPtr<gapi::TextureRTV> mRTV;
    };

    class RGTextureDSV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureDSV> GetDSV() const
        {
            CHECK(mDSV);
            return mDSV;
        }

    private:
        friend class RGBuilder;

        RGTextureDSV(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureDSV() = default;

        SharedPtr<gapi::TextureDSV> mDSV;
    };

    class RGShaderParameters : public RGResource
    {
    protected:
        friend class RGBuilder;

        RGShaderParameters(int index, SharedPtr<ShaderParameters> params);
        virtual ~RGShaderParameters() = default;

        SharedPtr<ShaderParameters> mParams;
    };

    template <typename ShaderParametersType>
        requires std::derived_from<ShaderParametersType, ShaderParameters>
    class TRGShaderParameters : public RGShaderParameters
    {
    public:
        ShaderParametersType* Get() { return dynamic_cast<ShaderParametersType*>(mParams.get()); }

    private:
        friend class RGBuilder;

        TRGShaderParameters(int index, SharedPtr<ShaderParametersType> params)
            : RGShaderParameters(index, params)
        {}
        virtual ~TRGShaderParameters() = default;
    };
} // namespace cube
