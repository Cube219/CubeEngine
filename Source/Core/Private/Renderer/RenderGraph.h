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
        RGBuilder(Renderer& renderer);
        ~RGBuilder();

        RGTextureHandle CreateTexture(const gapi::TextureInfo& textureInfo, StringView debugName);
        template <typename ShaderParameterListType>
            requires std::derived_from<ShaderParameterListType, ShaderParameterList>
        RGShaderParameterListHandle<ShaderParameterListType> CreateShaderParameterList()
        {
            ShaderParameterListManager& shaderParameterListManager = Engine::GetRenderer()->GetShaderParameterListManager();
            SharedPtr<ShaderParameterListType> parameterList = shaderParameterListManager.CreateShaderParameterList<ShaderParameterListType>();
            const ShaderParameterListInfo& parameterListInfo = ShaderParameterListManager::GetShaderParameterListInfo<ShaderParameterListType>();

            RGShaderParameterList<ShaderParameterListType>* rgParameterList = new RGShaderParameterList<ShaderParameterListType>(mResources.size(), parameterList, parameterListInfo);
            mResources.push_back(rgParameterList);

            return RGShaderParameterListHandle<ShaderParameterListType>(rgParameterList);
        }

        // TODO: Remove duplication
        RGTextureSRVHandle CreateSRV(RGTextureHandle rgTexture, Uint32 firstMipLevel = 0, Int32 mipLevels = -1);
        RGTextureUAVHandle CreateUAV(RGTextureHandle rgTexture, Uint32 mipLevel = 0, Uint32 firstSliceIndex = 0, Int32 sliceSize = -1);
        RGTextureRTVHandle CreateRTV(RGTextureHandle rgTexture, Uint32 mipLevel = 0);
        RGTextureDSVHandle CreateDSV(RGTextureHandle rgTexture, Uint32 mipLevel = 0);

        RGTextureSRVHandle GetDummyBlackTexture2D();
        RGTextureSRVHandle GetDummyBlackTextureCube();
        RGTextureSRVHandle GetDummyWhiteTexture2D();

        RGTextureHandle RegisterTexture(SharedPtr<gapi::Texture> texture);

        // TODO: Automatically collect attachments before executing.
        struct RenderPassInfo
        {
            struct ColorAttachment
            {
                RGTextureRTVHandle color;
                gapi::LoadOperation loadOperation = gapi::LoadOperation::Load;
                gapi::StoreOperation storeOperation = gapi::StoreOperation::Store;
                Float4 clearColor;
            };
            Vector<ColorAttachment> colors;
            struct DepthAttachment
            {
                RGTextureDSVHandle dsv;
                gapi::LoadOperation loadOperation = gapi::LoadOperation::Load;
                gapi::StoreOperation storeOperation = gapi::StoreOperation::Store;
                float clearDepth;
            };
            DepthAttachment depthstencil;
        };
        void BeginRenderPass(const RenderPassInfo& info);
        void EndRenderPass();

        struct DrawMeshInfo
        {
            SharedPtr<Mesh> mesh;
            MeshMetadata meshMetaData;
            gapi::RasterizerState::FillMode fillMode = gapi::RasterizerState::FillMode::Solid;
            ArrayView<SharedPtr<Material>> materials;
            Matrix model;
        };

        template <typename ShaderParameterListType>
            requires std::derived_from<ShaderParameterListType, ShaderParameterList>
        void BindShaderParameterList(RGShaderParameterListHandle<ShaderParameterListType> parameterList)
        {
            BindShaderParameterListInternal(ShaderParameterListType::GetName(), parameterList);
        }

        template <typename... T>
            requires (std::derived_from<T, ShaderParameterList>, ...)
        static Array<RGShaderParameterListBaseHandle, sizeof...(T)> MakeParameterListArray(RGShaderParameterListHandle<T>... params)
        {
            return { params... };
        }

        void AddPass(StringView name, PassFunction&& passFunction, UseResourceFunction&& useResourceFunction = [](RGBuilder&) {}, bool isCompute = false)
        {
            AddPassInternal(name, nullptr, nullptr, {}, std::move(passFunction), std::move(useResourceFunction));
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, RGShaderParameterListBaseHandle parameterList, PassFunction&& passFunction)
        {
            AddPassInternal(name, graphicsPipeline, nullptr, { &parameterList, 1 }, std::move(passFunction), nullptr);
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists, PassFunction&& passFunction)
        {
            AddPassInternal(name, graphicsPipeline, nullptr, parameterLists, std::move(passFunction), nullptr);
        }

        void AddPass(StringView name, SharedPtr<ComputePipeline> computePipeline, RGShaderParameterListBaseHandle parameterList, PassFunction&& passFunction)
        {
            AddPassInternal(name, nullptr, computePipeline, { &parameterList, 1 }, std::move(passFunction), nullptr);
        }

        void AddPass(StringView name, SharedPtr<ComputePipeline> computePipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists, PassFunction&& passFunction)
        {
            AddPassInternal(name, nullptr, computePipeline, parameterLists, std::move(passFunction), nullptr);
        }

        void AddDrawMeshPass(StringView name, ArrayView<DrawMeshInfo> drawMeshInfos, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists);

        void UseResource(RGTextureSRVHandle rgSRV);
        void UseResource(RGTextureUAVHandle rgUAV);
        void UseResource(RGTextureRTVHandle rgRTV);
        void UseResource(RGTextureDSVHandle rgDSV);
        void UseResource(RGTextureHandle rgTexture, gapi::SubresourceRangeInput range, gapi::ResourceStateFlags states);

        void ExecuteAndSubmit(gapi::CommandList& commandList);

    private:
        struct PassInfo
        {
            // Set in AddPass
            String name;
            int index = -1;

            Vector<RGShaderParameterListBaseHandle> shaderParameterLists;

            SharedPtr<GraphicsPipeline> graphicsPipeline = nullptr;
            SharedPtr<ComputePipeline> computePipeline = nullptr;

            PassFunction passFunction = nullptr;
            UseResourceFunction useResourceFunction = nullptr;

            // Set while executing
            struct ResourceUseInfo
            {
                int rgResourceIndex;
                gapi::ResourceStateFlags state;
                gapi::SubresourceRangeInput subresourceRange;
            };
            Vector<ResourceUseInfo> resourceUseInfos;

            Vector<gapi::TransitionState> transitions;
        };

        void BindShaderParameterListInternal(StringView name, RGShaderParameterListBaseHandle parameterList);

        void AddPassInternal(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, SharedPtr<ComputePipeline> computePipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists, PassFunction&& passFunction, UseResourceFunction&& useResourceFunction);

        void ResolveShaderParameterListsAndPipeline(PassInfo& pass, gapi::CommandList& commandList);
        void MarkUseResources(PassInfo& pass, gapi::CommandList& commandList);

        void UpdateResourceUsages();
        void CreateAllResources();
        void ResolveTransitions();

        void Reset();

        Renderer& mRenderer;

        Vector<PassInfo> mPasses;
        int mCurrentPassIndex = -1;
        PassInfo mLastPass;

        Vector<RGResource*> mResources;
        Map<gapi::Texture*, RGTextureHandle> mRegisteredTextures;
        RGTextureSRVHandle mDummyBlackTexture2D;
        RGTextureSRVHandle mDummyBlackTextureCube;
        RGTextureSRVHandle mDummyWhiteTexture2D;

        enum class State
        {
            Init,
            ResourceTracking,
            Executing,
            Submitted
        };
        State mState = State::Init;
        bool mIsInRenderPass = false;
        // TODO: Group variables in each used states.
        int mRenderPassIndex = -1;
        Vector<RGTextureRTVHandle> mAttachedRTVsInRenderPass;
        RGTextureDSVHandle mAttachedDSVInRenderPass;

        struct ShaderParameterListBindInfo
        {
            SharedPtr<gapi::Buffer> GPUBuffer = nullptr;
            int bindIndex = -1;
        };
        Map<String, ShaderParameterListBindInfo> mShaderParameterListBindInfos;

        SharedPtr<GraphicsPipeline> mCurrentBoundGraphicsPipeline;
        SharedPtr<ComputePipeline> mCurrentBoundComputePipeline;
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
