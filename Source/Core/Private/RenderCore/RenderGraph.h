#pragma once

#include "CoreHeader.h"

#include "Checker.h"
#include "Engine.h"
#include "GAPI_CommandList.h"
#include "RenderCore/RenderGraphTypes.h"
#include "Renderer/Renderer.h"
#include "RenderCore/ShaderParameter.h"

namespace cube
{
    class RGBuilder;

    struct RGPass
    {
        using PassFunction = std::function<void(gapi::CommandList& /*commandList*/)>;
        using TrackResourceFunction = std::function<void(RGBuilder& /*builder*/)>;

        // Set in AddPass
        String name;
        bool addTimestamp = false;

        Vector<RGShaderParameterListBaseHandle> shaderParameterLists;

        SharedPtr<GraphicsPipeline> graphicsPipeline = nullptr;
        SharedPtr<ComputePipeline> computePipeline = nullptr;

        PassFunction passFunction = nullptr;
        TrackResourceFunction trackResourceFunction = nullptr;

        bool IsGraphics() const { return graphicsPipeline != nullptr; }
        bool IsCompute() const { return computePipeline != nullptr; }

        // Set while tracking resources
        struct ResourceUsage
        {
            int rgResourceIndex;
            gapi::ResourceSyncFlags syncs;
            gapi::ResourceAccessFlags accesses;
            gapi::ResourceLayout layout;
            gapi::SubresourceRange subresourceRange;
            bool forceBarrier = false; // Used for UAV barrier
        };
        Vector<ResourceUsage> resourceUsages;

        Vector<gapi::ResourceBarrier> barriers;
    };

    // ===== Builder =====

    class RGBuilder
    {
    public:
        RGBuilder(Renderer& renderer);
        ~RGBuilder();

        // ===== Register / create resources =====

        RGBufferHandle RegisterBuffer(SharedPtr<gapi::Buffer> buffer);
        RGBufferHandle CreateBuffer(const gapi::BufferInfo& bufferInfo, StringView debugName);

        RGBufferSRVHandle CreateSRV(RGBufferHandle rgBuffer, const gapi::BufferSRVCreateInfo& createInfo = {});
        RGBufferUAVHandle CreateUAV(RGBufferHandle rgBuffer, const gapi::BufferUAVCreateInfo& createInfo = {});

        RGTextureHandle RegisterTexture(SharedPtr<gapi::Texture> texture,
            gapi::ResourceLayout srcLayout = gapi::ResourceLayout::Common,
            gapi::ResourceLayout dstLayout = gapi::ResourceLayout::Common
        );
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

            RGShaderParameterListHandle<ShaderParameterListType> rgParameterList(new RGShaderParameterList<ShaderParameterListType>(mResources.size(), parameterList, parameterListInfo));
            mResources.push_back(rgParameterList);

            return rgParameterList;
        }

    private:
        Vector<RGResourceHandle> mResources;
        Map<gapi::Buffer*, RGBufferHandle> mRegisteredBuffers;
        struct RegisteredTextureInfo
        {
            RGTextureHandle texture;
            gapi::ResourceLayout srcLayout;
            gapi::ResourceLayout dstLayout;
        };
        Map<gapi::Texture*, RegisteredTextureInfo> mRegisteredTextureInfos;

        // Caches to avoid creating duplicate views with the same parameters.
        // The view type is encoded into the cache key, so each base map can hold every view kind.
        HashMap<Uint64, RGBufferViewHandle> mCachedBufferViews;
        HashMap<Uint64, RGTextureViewHandle> mCachedTextureViews;
        RGTextureSRVHandle mDummyBlackTexture2D;
        RGTextureSRVHandle mDummyBlackTextureCube;
        RGTextureSRVHandle mDummyWhiteTexture2D;

        // ===== Pass functions =====
    public:
        template <typename... T>
            requires (std::derived_from<T, ShaderParameterList>, ...)
        static Array<RGShaderParameterListBaseHandle, sizeof...(T)> MakeParameterListArray(RGShaderParameterListHandle<T>... params)
        {
            return { params... };
        }

        void AddPass(StringView name,
            RGPass::PassFunction&& passFunction, RGPass::TrackResourceFunction&& trackResourceFunction = [](RGBuilder&) {},
            bool isCompute = false, bool addTimestamp = false
        )
        {
            AddPassInternal(name, nullptr, nullptr, {},
                std::move(passFunction), std::move(trackResourceFunction),
                addTimestamp
            );
        }

        // Graphics

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline,
             RGPass::PassFunction&& passFunction,
             bool addTimestamp = false
        )
        {
            AddPassInternal(name, graphicsPipeline, nullptr, {},
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline,
            RGPass::PassFunction&& passFunction, RGPass::TrackResourceFunction&& trackResourceFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, graphicsPipeline, nullptr, {},
                std::move(passFunction), std::move(trackResourceFunction),
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, RGShaderParameterListBaseHandle parameterList,
            RGPass::PassFunction&& passFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, graphicsPipeline, nullptr, { &parameterList, 1 },
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, RGShaderParameterListBaseHandle parameterList,
            RGPass::PassFunction&& passFunction, RGPass::TrackResourceFunction&& trackResourceFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, graphicsPipeline, nullptr, { &parameterList, 1 },
                std::move(passFunction), std::move(trackResourceFunction),
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists,
            RGPass::PassFunction&& passFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, graphicsPipeline, nullptr, parameterLists,
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists,
            RGPass::PassFunction&& passFunction, RGPass::TrackResourceFunction&& trackResourceFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, graphicsPipeline, nullptr, parameterLists,
                std::move(passFunction), std::move(trackResourceFunction),
                addTimestamp
            );
        }

        // Compute

        void AddPass(StringView name, SharedPtr<ComputePipeline> computePipeline, RGShaderParameterListBaseHandle parameterList,
            RGPass::PassFunction&& passFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, nullptr, computePipeline, { &parameterList, 1 },
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<ComputePipeline> computePipeline, RGShaderParameterListBaseHandle parameterList,
            RGPass::PassFunction&& passFunction, RGPass::TrackResourceFunction&& trackResourceFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, nullptr, computePipeline, { &parameterList, 1 },
                std::move(passFunction), std::move(trackResourceFunction),
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<ComputePipeline> computePipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists,
            RGPass::PassFunction&& passFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, nullptr, computePipeline, parameterLists,
                std::move(passFunction), nullptr,
                addTimestamp
            );
        }

        void AddPass(StringView name, SharedPtr<ComputePipeline> computePipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists,
            RGPass::PassFunction&& passFunction, RGPass::TrackResourceFunction&& trackResourceFunction,
            bool addTimestamp = false
        )
        {
            AddPassInternal(name, nullptr, computePipeline, parameterLists,
                std::move(passFunction), std::move(trackResourceFunction),
                addTimestamp
            );
        }

        // Render pass

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
            DepthAttachment depthStencil;
        };
        void BeginRenderPass(const RenderPassInfo& info);
        void EndRenderPass();

        void ApplyRenderTargetFormatsFromCurrentRenderPass(GraphicsPipelineInfo& inOutGraphicsPipelineInfo) const;
        void ApplyRenderTargetFormatsFromCurrentRenderPass(MaterialPipelineStateInfo& inOutMaterialPipelineInfo) const;

        // Mesh pass

        struct DrawMeshInfo
        {
            SharedPtr<Mesh> mesh;
            gapi::RasterizerState rasterizerState;
            gapi::DepthStencilState depthStencilState;
            ConstArrayView<WeakPtr<Material>> materials;
            Matrix model;
        };
        void AddDrawMeshPass(StringView name, ArrayView<DrawMeshInfo> drawMeshInfos, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists);

        // Global shader parameter

        template <typename ShaderParameterListType>
            requires std::derived_from<ShaderParameterListType, ShaderParameterList>
        void BindGlobalShaderParameterList(RGShaderParameterListHandle<ShaderParameterListType> parameterList)
        {
            BindGlobalShaderParameterListInternal(ShaderParameterListType::GetName(), parameterList);
        }

        template <typename ShaderParameterListType>
            requires std::derived_from<ShaderParameterListType, ShaderParameterList>
        void UnbindGlobalShaderParameterList()
        {
            UnbindGlobalShaderParameterListInternal(ShaderParameterListType::GetName());
        }

    private:
        void AddPassInternal(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, SharedPtr<ComputePipeline> computePipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists,
            RGPass::PassFunction&& passFunction, RGPass::TrackResourceFunction&& trackResourceFunction,
            bool addTimestamp
        );

        void BindGlobalShaderParameterListInternal(StringView name, RGShaderParameterListBaseHandle parameterList);
        void UnbindGlobalShaderParameterListInternal(StringView name);

        Vector<RGPass> mPasses;
        RGPass mLastPass;

        struct
        {
            bool isInRenderPass = false;
            Vector<gapi::ElementFormat> renderPassRenderTargetFormats;
            gapi::ElementFormat renderPassDepthStencilFormat = gapi::ElementFormat::Unknown;

            void Reset()
            {
                isInRenderPass = false;
                renderPassRenderTargetFormats.clear();
                renderPassDepthStencilFormat = gapi::ElementFormat::Unknown;
            }
        } mInitState;

        // ===== Tracking resources =====
    public:
        void UseResource(RGBufferSRVHandle rgSRV, gapi::ResourceSyncFlags syncs);
        void UseResource(RGBufferUAVHandle rgUAV, gapi::ResourceSyncFlags syncs);
        void UseResource(RGTextureSRVHandle rgSRV, gapi::ResourceSyncFlags syncs);
        void UseResource(RGTextureUAVHandle rgUAV, gapi::ResourceSyncFlags syncs);
        void UseResource(RGTextureRTVHandle rgRTV);
        void UseResource(RGTextureDSVHandle rgDSV);
        void UseResource(RGTextureHandle rgTexture, gapi::SubresourceRangeInput range, gapi::ResourceAccessFlags accesses, gapi::ResourceLayout layout, gapi::ResourceSyncFlags syncs);

        void AddUAVBarrier(RGBufferUAVHandle rgUAV);
        void AddUAVBarrier(RGTextureUAVHandle rgUAV);

    private:
        void UpdateResourceUsages();
        void CreateAllResources();
        void ResolveBarriers();

        struct
        {
            int passIndex = -1;

            int renderPassIndex = -1;
            Vector<RGTextureRTVHandle> attachedRTVsInRenderPass;
            RGTextureDSVHandle attachedDSVInRenderPass;
            Map<String, RGShaderParameterListBaseHandle> boundGlobalShaderParameterLists;

            void Reset()
            {
                passIndex = -1;
                renderPassIndex = -1;
                attachedRTVsInRenderPass.clear();
                attachedDSVInRenderPass = nullptr;
                boundGlobalShaderParameterLists.clear();
            }
        } mTrackingResourcesState;

        // ===== Executing =====
    public:
        // TODO: Rename to Execute and make not submitting at this function.
        void ExecuteAndSubmit(gapi::CommandList& commandList);
        void Execute(gapi::CommandList& commandList);

    private:
        void ResolveShaderParameterListsAndPipeline(RGPass& pass, gapi::CommandList& commandList);
        void MarkUseResources(RGPass& pass, gapi::CommandList& commandList);

        struct ShaderParameterListBindInfo
        {
            SharedPtr<gapi::Buffer> GPUBuffer = nullptr;
            SharedPtr<gapi::BufferSRV> srv = nullptr;
            int bindIndex = -1;
        };
        struct
        {
            SharedPtr<GraphicsPipeline> boundGraphicsPipeline;
            SharedPtr<ComputePipeline> boundComputePipeline;

            Map<String, ShaderParameterListBindInfo> shaderParameterListBindInfos;

            void Reset()
            {
                boundGraphicsPipeline = nullptr;
                boundComputePipeline = nullptr;

                shaderParameterListBindInfos.clear();
            }
        } mExecuteState;

        // ===== Others =====
    private:
        void Reset();

        Renderer& mRenderer;

        enum class Phase
        {
            Init,
            TrackingResources,
            Executing,
            Executed
        };
        Phase mPhase = Phase::Init;
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
