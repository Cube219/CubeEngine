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

        RGBufferHandle RegisterBuffer(SharedPtr<gapi::Buffer> buffer);
        RGBufferHandle CreateBuffer(const gapi::BufferInfo& bufferInfo, StringView debugName);

        RGBufferSRVHandle CreateSRV(RGBufferHandle rgBuffer, const gapi::BufferSRVCreateInfo& createInfo = {});
        RGBufferUAVHandle CreateUAV(RGBufferHandle rgBuffer, const gapi::BufferUAVCreateInfo& createInfo = {});

        RGTextureHandle RegisterTexture(SharedPtr<gapi::Texture> texture);
        RGTextureHandle CreateTexture(const gapi::TextureInfo& textureInfo, StringView debugName);

        RGTextureSRVHandle CreateSRV(RGTextureHandle rgTexture, const gapi::TextureSRVCreateInfo& createInfo = {});
        RGTextureUAVHandle CreateUAV(RGTextureHandle rgTexture, const gapi::TextureUAVCreateInfo& createInfo = {});
        RGTextureRTVHandle CreateRTV(RGTextureHandle rgTexture, const gapi::TextureRTVCreateInfo& createInfo = {});
        RGTextureDSVHandle CreateDSV(RGTextureHandle rgTexture, const gapi::TextureDSVCreateInfo& createInfo = {});

        RGTextureSRVHandle GetDummyBlackTexture2D();
        RGTextureSRVHandle GetDummyBlackTextureCube();
        RGTextureSRVHandle GetDummyWhiteTexture2D();

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
            gapi::RasterizerState::FillMode fillMode = gapi::RasterizerState::FillMode::Solid;
            ConstArrayView<WeakPtr<Material>> materials;
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

        void AddPass(StringView name,
            PassFunction&& passFunction, UseResourceFunction&& useResourceFunction = [](RGBuilder&) {},
            bool isCompute = false, bool addTimestamp = false
        )
        {
            AddPassInternal(name, nullptr, nullptr, {},
                std::move(passFunction), std::move(useResourceFunction),
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline,
             PassFunction&& passFunction,
             bool addTimestamp = false
        )
        {
            AddPassInternal(name, graphicsPipeline, nullptr, {},
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, RGShaderParameterListBaseHandle parameterList,
            PassFunction&& passFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, graphicsPipeline, nullptr, { &parameterList, 1 },
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists,
            PassFunction&& passFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, graphicsPipeline, nullptr, parameterLists,
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<ComputePipeline> computePipeline, RGShaderParameterListBaseHandle parameterList,
            PassFunction&& passFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, nullptr, computePipeline, { &parameterList, 1 },
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<ComputePipeline> computePipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists,
            PassFunction&& passFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, nullptr, computePipeline, parameterLists,
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddDrawMeshPass(StringView name, ArrayView<DrawMeshInfo> drawMeshInfos, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists);

        void UseResource(RGBufferSRVHandle rgSRV);
        void UseResource(RGBufferUAVHandle rgUAV);
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
            bool addTimestamp = false;
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
                gapi::SubresourceRange subresourceRange;
            };
            Vector<ResourceUseInfo> resourceUseInfos;

            Vector<gapi::TransitionState> transitions;
        };

        void BindShaderParameterListInternal(StringView name, RGShaderParameterListBaseHandle parameterList);

        void AddPassInternal(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, SharedPtr<ComputePipeline> computePipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists,
            PassFunction&& passFunction, UseResourceFunction&& useResourceFunction,
            bool addTimestamp
        );

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
        Map<gapi::Buffer*, RGBufferHandle> mRegisteredBuffers;
        Map<gapi::Texture*, RGTextureHandle> mRegisteredTextures;

        // Caches to avoid creating duplicated views with the same parameters.
        // The view type is encoded into the cache key, so each base map can hold every view kind.
        HashMap<Uint64, RGBufferView*> mCachedBufferViews;
        HashMap<Uint64, RGTextureView*> mCachedTextureViews;
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
            SharedPtr<gapi::BufferSRV> srv = nullptr;
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

    class RGGPUTimestampScope
    {
    public:
        RGGPUTimestampScope(RGBuilder& builder, StringView name)
            : mBuilder(builder)
        {
            mBuilder.AddPass(CUBE_T("##BeginGPUTimestampScope"),
                [strName = String(name)](gapi::CommandList& commandList)
                {
                    commandList.BeginTimestamp(strName);
                }
            );
        }
        ~RGGPUTimestampScope()
        {
            mBuilder.AddPass(CUBE_T("##EndGPUTimestampScope"),
                [](gapi::CommandList& commandList)
                {
                    commandList.EndTimestamp();
                }
            );
        }

        RGGPUTimestampScope(const RGGPUTimestampScope& other) = delete;
        RGGPUTimestampScope& operator=(const RGGPUTimestampScope& rhs) = delete;

    private:
        RGBuilder& mBuilder;
    };
#define RG_GPU_TIMESTAMP_SCOPE(builder, name) RGGPUTimestampScope CUBE_MACRO_JOIN(_timestampScope, __LINE__)(builder, name)
} // namespace cube
