#pragma once

#include "CoreHeader.h"

#include "GAPI_Texture.h"
#include "ShaderParameter.h"

namespace cube
{
    class RGBuilder;

    // ===== Resources =====

    class RGResource
    {
    public:
        bool IsTransient() const { return mIsTransient; }

        StringView GetDebugName() const { return mDebugName; }

        virtual void CreateResource(GAPI& gapi) {}
        virtual bool IsResourceCreated() const = 0;
        virtual void UpdateUsePassIndex(int passIndex)
        {
            if (mBeginPass == -1 || mBeginPass > passIndex)
            {
                mBeginPass = passIndex;
            }
            if (mEndPass < passIndex)
            {
                mEndPass = passIndex;
            }
        }

    protected:
        friend class RGBuilder;

        RGResource(int index, StringView debugName);
        virtual ~RGResource() = default;

        bool mIsTransient;

        int mIndex;
        int mBeginPass;
        int mEndPass;

        String mDebugName;
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

    class RGBuffer : public RGResource
    {
    public:
        virtual void CreateResource(GAPI& gapi) override;
        virtual bool IsResourceCreated() const override { return mBuffer != nullptr; }

    private:
        friend class RGBuilder;
        friend class RGBufferSRV;
        friend class RGBufferUAV;

        RGBuffer(int index, const gapi::BufferInfo& bufferInfo, StringView debugName);
        RGBuffer(int index, SharedPtr<gapi::Buffer> buffer);
        virtual ~RGBuffer() = default;

        SharedPtr<gapi::Buffer> mBuffer;
        gapi::BufferInfo mBufferInfo;
    };
    using RGBufferHandle = RGResourceHandler<RGBuffer>;

    class RGBufferView : public RGResource
    {
    public:

    protected:
        friend class RGBuilder;

        RGBufferView(int index, RGBuffer* rgBuffer, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements);
        virtual ~RGBufferView() = default;

        RGBuffer* mRGBuffer;

        gapi::ElementFormat mFormat;
        Uint64 mFirstElement;
        Uint64 mNumElements;
    };
    using RGBufferViewHandle = RGResourceHandler<RGBufferView>;

    class RGBufferSRV : public RGBufferView
    {
    public:
        SharedPtr<gapi::BufferSRV> GetSRV() const { return mSRV; }

        virtual void CreateResource(GAPI& gapi) override;
        virtual bool IsResourceCreated() const override { return mSRV != nullptr; }

    private:
        friend class RGBuilder;

        RGBufferSRV(int index, RGBuffer* rgBuffer, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements);
        virtual ~RGBufferSRV() = default;

        SharedPtr<gapi::BufferSRV> mSRV;
    };
    using RGBufferSRVHandle = RGResourceHandler<RGBufferSRV>;

    class RGBufferUAV : public RGBufferView
    {
    public:
        SharedPtr<gapi::BufferUAV> GetUAV() const { return mUAV; }

        virtual void CreateResource(GAPI& gapi) override;
        virtual bool IsResourceCreated() const override { return mUAV != nullptr; }

    private:
        friend class RGBuilder;

        RGBufferUAV(int index, RGBuffer* rgBuffer, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements);
        virtual ~RGBufferUAV() = default;

        SharedPtr<gapi::BufferUAV> mUAV;
    };
    using RGBufferUAVHandle = RGResourceHandler<RGBufferUAV>;

    class RGTexture : public RGResource
    {
    public:
        SharedPtr<gapi::Texture> GetGAPITexture() const { return mTexture; }
        Uint64 GetSubresourceHashKey(const gapi::SubresourceRange& range) const;

        virtual void CreateResource(GAPI& gapi) override;
        virtual bool IsResourceCreated() const override { return mTexture != nullptr; }

        const gapi::TextureInfo& GetTextureInfo() const { return mTextureInfo; }

    protected:
        friend class RGBuilder;
        friend class RGTextureSRV;
        friend class RGTextureUAV;
        friend class RGTextureRTV;
        friend class RGTextureDSV;

        RGTexture(int index, const gapi::TextureInfo& textureInfo, StringView debugName);
        RGTexture(int index, SharedPtr<gapi::Texture> texture);
        virtual ~RGTexture() = default;

        SharedPtr<gapi::Texture> mTexture;
        gapi::TextureInfo mTextureInfo;
    };
    using RGTextureHandle = RGResourceHandler<RGTexture>;

    class RGTextureView : public RGResource
    {
    public:
        virtual void UpdateUsePassIndex(int passIndex) override
        {
            RGResource::UpdateUsePassIndex(passIndex);

            mRGTexture->UpdateUsePassIndex(passIndex);
        }

        const gapi::SubresourceRange GetSubresourceRange() const
        {
            CHECK(mRGTexture->GetGAPITexture());
            return mSubresourceRange;
        }
        Uint64 GetSubresourceHashKey() const
        {
            CHECK(mSubresourceHashKey);
            return mSubresourceHashKey;
        }

        bool IsOverlap(RGTextureView* rhs) const
        {
            if (mRGTexture != rhs->mRGTexture)
            {
                return false;
            }
            // Use subresource range input so it can be called before creating resource.
            return mSubresourceRangeInput.IsOverlap(rhs->mSubresourceRangeInput);
        }

    protected:
        friend class RGBuilder;

        RGTextureView(int index, RGTexture* rgTexture);
        virtual ~RGTextureView() = default;

        RGTexture* mRGTexture;
        gapi::SubresourceRangeInput mSubresourceRangeInput;
        gapi::SubresourceRange mSubresourceRange;
        Uint64 mSubresourceHashKey;
    };
    using RGTextureViewHandle = RGResourceHandler<RGTextureView>;

    class RGTextureSRV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureSRV> GetSRV() const { return mSRV; }

        virtual void CreateResource(GAPI& gapi) override;
        virtual bool IsResourceCreated() const override { return mSRV != nullptr; }

    private:
        friend class RGBuilder;

        RGTextureSRV(int index, RGTexture* rgTexture, Uint32 firstMipLevel, Uint32 mipLevels);
        virtual ~RGTextureSRV() = default;

        SharedPtr<gapi::TextureSRV> mSRV;
    };
    using RGTextureSRVHandle = RGResourceHandler<RGTextureSRV>;

    class RGTextureUAV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureUAV> GetUAV() const { return mUAV; }

        virtual void CreateResource(GAPI& gapi) override;
        virtual bool IsResourceCreated() const override { return mUAV != nullptr; }

    private:
        friend class RGBuilder;

        RGTextureUAV(int index, RGTexture* rgTexture, Uint32 mipLevel, Uint32 firstSliceIndex, Uint32 sliceSize);
        virtual ~RGTextureUAV() = default;

        SharedPtr<gapi::TextureUAV> mUAV;
    };
    using RGTextureUAVHandle = RGResourceHandler<RGTextureUAV>;

    class RGTextureRTV : public RGTextureView
    {
    public:
        SharedPtr<gapi::TextureRTV> GetRTV() const { return mRTV; }

        virtual void CreateResource(GAPI& gapi) override;
        virtual bool IsResourceCreated() const override { return mRTV != nullptr; }

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

        virtual void CreateResource(GAPI& gapi) override;
        virtual bool IsResourceCreated() const override { return mDSV != nullptr; }

    private:
        friend class RGBuilder;

        RGTextureDSV(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureDSV() = default;

        SharedPtr<gapi::TextureDSV> mDSV;
    };
    using RGTextureDSVHandle = RGResourceHandler<RGTextureDSV>;

    class RGShaderParameterListBase : public RGResource
    {
    protected:
        friend class RGBuilder;

        RGShaderParameterListBase(int index, const ShaderParameterListInfo& parameterListInfo, SharedPtr<ShaderParameterList> parameterList);
        virtual ~RGShaderParameterListBase() = default;

        virtual bool IsResourceCreated() const override { return true; }

        const ShaderParameterListInfo& mParameterListInfo;
        SharedPtr<ShaderParameterList> mParameterList;
    };
    using RGShaderParameterListBaseHandle = RGResourceHandler<RGShaderParameterListBase>;

    template <typename ShaderParameterListType>
        requires std::derived_from<ShaderParameterListType, ShaderParameterList>
    class RGShaderParameterList : public RGShaderParameterListBase
    {
    public:
        ShaderParameterListType* Get() { return mCastedPtr; }

    private:
        friend class RGBuilder;

        RGShaderParameterList(int index, SharedPtr<ShaderParameterListType> parameterList, const ShaderParameterListInfo& parameterListInfo)
            : RGShaderParameterListBase(index, parameterListInfo, parameterList)
            , mCastedPtr(dynamic_cast<ShaderParameterListType*>(mParameterList.get()))
        {}
        virtual ~RGShaderParameterList() = default;

        ShaderParameterListType* mCastedPtr;
    };
    template <typename ShaderParameterListType>
        requires std::derived_from<ShaderParameterListType, ShaderParameterList>
    using RGShaderParameterListHandle = RGResourceHandler<RGShaderParameterList<ShaderParameterListType>>;


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
