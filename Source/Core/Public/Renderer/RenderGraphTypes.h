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

    template <typename RGResourceType>
        requires std::derived_from<RGResourceType, RGResource>
    class RGResourceHandler
    {
    public:
        RGResourceHandler() = default;

        RGResourceType* operator->() const { return mResource; }

        template <typename RGResourceUpcastType>
            requires std::derived_from<RGResourceType, RGResourceUpcastType> && std::derived_from<RGResourceUpcastType, RGResource>
        operator RGResourceHandler<RGResourceUpcastType>() const
        {
            return RGResourceHandler<RGResourceUpcastType>(static_cast<RGResourceUpcastType*>(mResource));
        }

        template <typename RGResourceCastType>
            requires std::derived_from<RGResourceCastType, RGResource>
        RGResourceHandler<RGResourceCastType> Cast() const
        {
            return RGResourceHandler<RGResourceCastType>(dynamic_cast<RGResourceCastType*>(mResource));
        }

        bool IsValid() const { return (mResource != nullptr); }

    private:
        template <typename OtherType>
            requires std::derived_from<OtherType, RGResource>
        friend class RGResourceHandler;

        friend class RGBuilder;

        RGResourceHandler(RGResourceType* resource)
            : mResource(resource)
        {
        }

        RGResourceType* mResource = nullptr;
    };

    class RGTexture : public RGResource
    {
    protected:
        friend class RGBuilder;
        friend class RGTextureSRV;
        friend class RGTextureUAV;
        friend class RGTextureRTV;
        friend class RGTextureDSV;

        RGTexture(int index, SharedPtr<gapi::Texture> texture);
        virtual ~RGTexture() = default;

        SharedPtr<gapi::Texture> mTexture;
    };
    using RGTextureHandle = RGResourceHandler<RGTexture>;

    class RGTextureView : public RGResource
    {
    public:
        Uint64 GetSubresourceHashKey() const
        {
            CHECK(mSubresourceHashKey);
            return mSubresourceHashKey;
        }

    protected:
        friend class RGBuilder;

        RGTextureView(int index, RGTexture* rgTexture);
        virtual ~RGTextureView() = default;

        RGTexture* mRGTexture;
        gapi::SubresourceRange mSubresourceRange;
        Uint64 mSubresourceHashKey;
    };
    using RGTextureViewHandle = RGResourceHandler<RGTextureView>;

    class RGTextureSRV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureSRV> GetSRV() const { return mSRV; }

    private:
        friend class RGBuilder;

        RGTextureSRV(int index, RGTexture* rgTexture, Uint32 firstMipLevel, Int32 mipLevels);
        virtual ~RGTextureSRV() = default;

        SharedPtr<gapi::TextureSRV> mSRV;
    };
    using RGTextureSRVHandle = RGResourceHandler<RGTextureSRV>;

    class RGTextureUAV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureUAV> GetUAV() const { return mUAV; }

    private:
        friend class RGBuilder;

        RGTextureUAV(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureUAV() = default;

        SharedPtr<gapi::TextureUAV> mUAV;
    };
    using RGTextureUAVHandle = RGResourceHandler<RGTextureUAV>;

    class RGTextureRTV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureRTV> GetRTV() const { return mRTV; }

    private:
        friend class RGBuilder;

        RGTextureRTV(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureRTV() = default;

        SharedPtr<gapi::TextureRTV> mRTV;
    };
    using RGTextureRTVHandle = RGResourceHandler<RGTextureRTV>;

    class RGTextureDSV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureDSV> GetDSV() const { return mDSV; }

    private:
        friend class RGBuilder;

        RGTextureDSV(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureDSV() = default;

        SharedPtr<gapi::TextureDSV> mDSV;
    };
    using RGTextureDSVHandle = RGResourceHandler<RGTextureDSV>;

    class RGShaderParametersBase : public RGResource
    {
    protected:
        friend class RGBuilder;

        RGShaderParametersBase(int index, const ShaderParametersInfo& parametersInfo, SharedPtr<ShaderParameters> params);
        virtual ~RGShaderParametersBase() = default;

        const ShaderParametersInfo& mParametersInfo;
        SharedPtr<ShaderParameters> mParams;
    };
    using RGShaderParametersBaseHandle = RGResourceHandler<RGShaderParametersBase>;

    template <typename ShaderParametersType>
        requires std::derived_from<ShaderParametersType, ShaderParameters>
    class RGShaderParameters : public RGShaderParametersBase
    {
    public:
        ShaderParametersType* Get() { return mCastedPtr; }

    private:
        friend class RGBuilder;

        RGShaderParameters(int index, SharedPtr<ShaderParametersType> params, const ShaderParametersInfo& parametersInfo)
            : RGShaderParametersBase(index, parametersInfo, params)
            , mCastedPtr(dynamic_cast<ShaderParametersType*>(mParams.get()))
        {}
        virtual ~RGShaderParameters() = default;

        ShaderParametersType* mCastedPtr;
    };
    template <typename ShaderParametersType>
        requires std::derived_from<ShaderParametersType, ShaderParameters>
    using RGShaderParametersHandle = RGResourceHandler<RGShaderParameters<ShaderParametersType>>;


    // ===== ShaderParameterTypeInfo specializations for RG handles =====

    template <>
    struct ShaderParameterTypeInfo<RGTextureSRVHandle>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::RGTextureSRV;
        static constexpr Uint32 size = sizeof(RGTextureSRVHandle);
    };

    template <>
    struct ShaderParameterTypeInfo<RGTextureUAVHandle>
    {
        static constexpr ShaderParameterType type = ShaderParameterType::RGTextureUAV;
        static constexpr Uint32 size = sizeof(RGTextureUAVHandle);
    };
} // namespace cube
