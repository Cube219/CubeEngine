#pragma once

#include "CoreHeader.h"

#include "Checker.h"
#include "Engine.h"
#include "GAPI_CommandList.h"
#include "Renderer/RenderGraphTypes.h"
#include "Renderer/Renderer.h"
#include "Renderer/ShaderParameter.h"

namespace cube
{
    class RGBuilder;

    // ===== Builder =====

    class RGBuilder
    {
    public:
        using UseResourceFunction = std::function<void(RGBuilder& /*builder*/)>;
        using PassFunction = std::function<void(gapi::CommandList& /*commandList*/)>;

    public:
        RGBuilder() = default;
        ~RGBuilder();

        // void CreateTexture();
        template <typename ShaderParametersType>
            requires std::derived_from<ShaderParametersType, ShaderParameters>
        TRGShaderParameters<ShaderParametersType>* CreateShaderParameters()
        {
            ShaderParametersManager& shaderParametersManager = Engine::GetRenderer()->GetShaderParametersManager();
            SharedPtr<ShaderParametersType> parameters = shaderParametersManager.CreateShaderParameters<ShaderParametersType>();

            TRGShaderParameters<ShaderParametersType>* rgParameters = new TRGShaderParameters<ShaderParametersType>(mResources.size(), parameters);
            mResources.push_back(rgParameters);

            return rgParameters;
        }

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

        template <typename ShaderParametersType>
            requires std::derived_from<ShaderParametersType, ShaderParameters>
        void BindShaderParameters(RGShaderParameters<ShaderParametersType>* parameters)
        {
            BindShaderParametersInternal(ShaderParametersType::GetDebugName, parameters);
        }

        void AddPass(StringView name, PassFunction&& passFunction, UseResourceFunction&& useResourceFunction = [](RGBuilder&) {}, bool isCompute = false);

        template <typename ShaderParametersType>
            requires std::derived_from<ShaderParametersType, ShaderParameters>
        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, RGShaderParameters<ShaderParametersType>* parameters, PassFunction&& passFunction)
        {
            AddPassInternal(name, parameters, graphicsPipeline, nullptr, ShaderParametersType::GetParameterInfos(), std::move(passFunction));
        }

        template <typename ShaderParametersType>
            requires std::derived_from<ShaderParametersType, ShaderParameters>
        void AddPass(StringView name, SharedPtr<ComputePipeline> computePipeline, RGShaderParameters<ShaderParametersType>* parameters, PassFunction&& passFunction)
        {
            AddPassInternal(name, parameters, nullptr, computePipeline, ShaderParametersType::GetParameterInfos(), std::move(passFunction));
        }

        // TODO: Automatically collect use resource based on shader parameter.
        void UseResource(RGTextureSRV* rgSRV);
        void UseResource(RGTextureUAV* rgUAV);
        void UseResource(RGTextureRTV* rgRTV);
        void UseResource(RGTextureDSV* rgDSV);

        void ExecuteAndSubmit(gapi::CommandList& commandList);

    private:
        void BindShaderParametersInternal(StringView name, RGShaderParameters* parameters);

        void AddPassInternal(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, SharedPtr<ComputePipeline> computePipeline, RGShaderParameters* parameters, const Vector<ShaderParameterInfo>& parameterInfos, PassFunction&& passFunction);

        void ResolveTransitions();

        void RollbackResourceStates();

        void Reset();

        struct PassInfo
        {
            // Set in AddPass
            String name;
            int index;
            SharedPtr<GraphicsPipeline> graphicsPipeline;
            SharedPtr<ComputePipeline> computePipeline;
            PassFunction passFunction;

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

        enum class State
        {
            Init,
            ResourceTracking,
            Executing,
            Submitted
        };
        State mState = State::Init;
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
