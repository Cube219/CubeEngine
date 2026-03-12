#pragma once

#include "CoreHeader.h"

#include "Checker.h"
#include "GAPI_CommandList.h"

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
        virtual ~RGResource();

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
        virtual ~RGTexture();

        SharedPtr<gapi::Texture> mTexture;
    };

    class RGTextureView : public RGResource
    {
    public:
        Uint64 GetSubresourceHashKey() const { return mSubresourceHashKey; }

    protected:
        friend class RGBuilder;

        RGTextureView(int index, RGTexture* rgTexture, Uint32 mipLevel);
        virtual ~RGTextureView();

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
        virtual ~RGTextureSRV();

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
        virtual ~RGTextureUAV();

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
        virtual ~RGTextureRTV();

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
        virtual ~RGTextureDSV();

        SharedPtr<gapi::TextureDSV> mDSV;
    };

    // ===== Builder =====

    class RGBuilder
    {
    public:
        using UseResourceFunction = std::function<void(RGBuilder&)>;
        using PassFunction = std::function<void(gapi::CommandList& /*commandList*/)>;

    public:
        RGBuilder() = default;
        ~RGBuilder();

        // void CreateTexture();
        // void CreateShaderParameter();
        RGTextureSRV* CreateSRV(RGTexture* rgTexture, Uint32 mipLevel);
        RGTextureUAV* CreateUAV(RGTexture* rgTexture, Uint32 mipLevel);
        RGTextureRTV* CreateRTV(RGTexture* rgTexture, Uint32 mipLevel);
        RGTextureDSV* CreateDSV(RGTexture* rgTexture, Uint32 mipLevel);

        RGTexture* RegisterTexture(SharedPtr<gapi::Texture> texture);

        // TODO: Automatically collect attachments before executing.
        struct RenderPassInfo
        {
            struct ColorAttachment
            {
                RGTextureRTV* color = nullptr;
                gapi::LoadOperation loadOperation = gapi::LoadOperation::Load;
                gapi::StoreOperation storeOperation = gapi::StoreOperation::Store;
                Float4 clearColor;
            };
            Vector<ColorAttachment> colors;
            struct DepthAttachment
            {
                RGTextureDSV* dsv = nullptr;
                gapi::LoadOperation loadOperation = gapi::LoadOperation::Load;
                gapi::StoreOperation storeOperation = gapi::StoreOperation::Store;
                float clearDepth;
            };
            DepthAttachment depthstencil;
        };
        void BeginRenderPass(const RenderPassInfo& info);
        void EndRenderPass();

        void AddPass(StringView name, PassFunction&& passFunction, UseResourceFunction&& useResourceFunction = [](RGBuilder&){}, bool isCompute = false);

        // TODO: Automatically collect use resource based on shader parameter.
        void UseResource(RGTextureSRV* rgSRV);
        void UseResource(RGTextureUAV* rgUAV);
        void UseResource(RGTextureRTV* rgRTV);
        void UseResource(RGTextureDSV* rgDSV);

        void ExecuteAndSubmit(gapi::CommandList& commandList);

    private:
        void ResolveTransitions();

        void RollbackResourceStates();

        void Reset();

        struct PassInfo
        {
            // Set in AddPass
            String name;
            int index;
            PassFunction passFunction;
            bool isCompute;

            // Set while executing
            struct UseState
            {
                int resourceIndex;
                gapi::ResourceStateFlags state;
            };
            Vector<UseState> useStates;
            Vector<gapi::TransitionState> transitions;
        };
        Vector<PassInfo> mPasses;
        Vector<UseResourceFunction> mUseResourceFunction;
        int mCurrentPassIndex;

        Vector<RGResource*> mResources;

        bool mIsExecuting = false;
        bool mIsInRenderPass = false;
    };

    // ===== Utility =====

    class RGGPUEventScope
	{
	public:
		RGGPUEventScope(RGBuilder& builder, StringView name)
		    : mCurrentBuilder(builder)
		{
            mCurrentBuilder.AddPass(CUBE_T("##BeginGPUEventScope"), [strName = String(name)](gapi::CommandList& commandList){ commandList.BeginEvent(strName); });
		}
		~RGGPUEventScope()
		{
            mCurrentBuilder.AddPass(CUBE_T("##EndGPUEventScope"), [](gapi::CommandList& commandList){ commandList.EndEvent(); });
		}

		RGGPUEventScope(const RGGPUEventScope& other) = delete;
		RGGPUEventScope& operator=(const RGGPUEventScope& rhs) = delete;

	private:
        RGBuilder& mCurrentBuilder;
	};
#define RG_GPU_EVENT_SCOPE(builder, name) RGGPUEventScope CUBE_MACRO_JOIN(_eventScope, __LINE__)(builder, name)

} // namespace cube
