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
    public:
        SharedPtr<gapi::TextureSRV> GetSRV() const { CHECK(mSRV); return mSRV; }
        SharedPtr<gapi::TextureUAV> GetUAV() const { CHECK(mUAV); return mUAV; }
        SharedPtr<gapi::TextureRTV> GetRTV() const { CHECK(mRTV); return mRTV; }
        SharedPtr<gapi::TextureDSV> GetDSV() const { CHECK(mDSV); return mDSV; }

    protected:
        friend class RGBuilder;

        RGTexture(int index, SharedPtr<gapi::Texture> texture, Uint32 mipLevel);
        virtual ~RGTexture();

        SharedPtr<gapi::Texture> mTexture;
        SharedPtr<gapi::TextureSRV> mSRV;
        SharedPtr<gapi::TextureUAV> mUAV;
        SharedPtr<gapi::TextureRTV> mRTV;
        SharedPtr<gapi::TextureDSV> mDSV;
        Uint32 mMipLevel;
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
        RGTexture* RegisterTexture(SharedPtr<gapi::Texture> texture, Uint32 mipLevel);

        // TODO: Automatically collect attachments before executing.
        struct RenderPassInfo
        {
            struct ColorAttachment
            {
                RGTexture* color = nullptr;
                gapi::LoadOperation loadOperation = gapi::LoadOperation::Load;
                gapi::StoreOperation storeOperation = gapi::StoreOperation::Store;
                Float4 clearColor;
            };
            Vector<ColorAttachment> colors;
            struct DepthAttachment
            {
                RGTexture* dsv = nullptr;
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
        void UseSRV(RGTexture* texture);
        void UseUAV(RGTexture* texture);
        void UseRTV(RGTexture* texture);
        void UseDSV(RGTexture* texture);

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
