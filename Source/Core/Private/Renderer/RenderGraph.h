#pragma once

#include "CoreHeader.h"

#include "GAPI_CommandList.h"

namespace cube
{
    // ===== Builder =====

    class RGBuilder
    {
    public:
        using PassFunction = std::function<void(gapi::CommandList&)>;
        struct PassInfo
        {
            String name;
            PassFunction passFunction;
        };

    public:
        // void CreateTexture();
        // void CreateShaderParameter();

        // TODO: Automatically collect attachments before executing
        void BeginRenderPass(ArrayView<gapi::ColorAttachment> colors, gapi::DepthStencilAttachment depthStencil);
        void EndRenderPass();

        void AddPass(StringView name, PassFunction&& passFunction);

        void ExecuteAndSubmit(gapi::CommandList& commandList);

    private:
        Vector<PassInfo> mPasses;

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
