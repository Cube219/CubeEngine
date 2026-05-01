#pragma once

#include "CoreHeader.h"

#include "GAPI_Pipeline.h"
#include "GAPI_ShaderReflection.h"
#include "Shader.h"

namespace cube
{
    class GAPI;
    class Renderer;

    // ===== GraphicsPipeline =====

    struct GraphicsPipelineInfo
    {
        SharedPtr<Shader> vertexShader = nullptr;
        SharedPtr<Shader> pixelShader = nullptr;

        Vector<gapi::InputElement> inputLayouts;

        gapi::RasterizerState rasterizerState;

        Array<gapi::BlendState, gapi::MAX_NUM_RENDER_TARGETS> blendStates;

        gapi::DepthStencilState depthStencilState;

        gapi::PrimitiveTopologyType primitiveTopologyType = gapi::PrimitiveTopologyType::Triangle;

        Uint32 numRenderTargets;
        Array<gapi::ElementFormat, gapi::MAX_NUM_RENDER_TARGETS> renderTargetFormats;
        gapi::ElementFormat depthStencilFormat = gapi::ElementFormat::D32_Float;

        Uint64 HashValue = 0;

        Uint64 GetHashValue() const { CHECK(HashValue); return HashValue; }
        gapi::GraphicsPipelineInfo GetGAPIVersion() const;

        void CalculateHashValue();
#if CUBE_USE_CHECK
        bool ValidateHashValue() const;
#endif // CUBE_USE_CHECK
    };

    struct GraphicsPipelineCreateInfo
    {
        GraphicsPipelineInfo pipelineInfo;

        StringView debugName;
    };

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(GAPI& gapi, const GraphicsPipelineCreateInfo& createInfo);
        ~GraphicsPipeline() = default;

        SharedPtr<gapi::GraphicsPipeline> GetGAPIGraphicsPipeline() const { return mGAPIGraphicsPipeline; }
        const gapi::ShaderReflection& GetMergedShaderReflection() const { return mMergedShaderReflection; }

        bool HasRecompiledShadersInPipeline() const;

    private:
        void MergeShaderReflection(const gapi::ShaderReflection& reflection);
        void CacheShaderReflection(SharedPtr<Shader> vertexShader, SharedPtr<Shader> pixelShader);

        SharedPtr<gapi::GraphicsPipeline> mGAPIGraphicsPipeline;
        gapi::ShaderReflection mMergedShaderReflection;

        SharedPtr<Shader> mVertexShader;
        SharedPtr<Shader> mPixelShader;

        String mDebugName;
    };

    // ===== ComputePipeline =====

    struct ComputePipelineInfo
    {
        SharedPtr<Shader> shader;

        Uint64 HashValue = 0;

        Uint64 GetHashValue() const { CHECK(HashValue); return HashValue; }
        gapi::ComputePipelineInfo GetGAPIVersion() const;

        void CalculateHashValue();
#if CUBE_USE_CHECK
        bool ValidateHashValue() const;
#endif
    };

    struct ComputePipelineCreateInfo
    {
        ComputePipelineInfo pipelineInfo;

        StringView debugName;
    };

    class ComputePipeline
    {
    public:
        ComputePipeline(GAPI& gapi, const ComputePipelineCreateInfo& createInfo);
        ~ComputePipeline() = default;

        SharedPtr<gapi::ComputePipeline> GetGAPIComputePipeline() const { return mGAPIComputePipeline; }
        const gapi::ShaderReflection& GetShaderReflection() const { return mShaderReflection; }

        bool HasRecompiledShaderInPipeline() const;

    private:
        void CacheShaderReflection(SharedPtr<Shader> shader);

        SharedPtr<gapi::ComputePipeline> mGAPIComputePipeline;
        gapi::ShaderReflection mShaderReflection;

        SharedPtr<Shader> mShader;

        String mDebugName;
    };

    // ===== PipelineManager =====

    class PipelineManager
    {
    public:
        PipelineManager(Renderer& renderer);
        ~PipelineManager() = default;

        void Initialize();
        void Shutdown();

        SharedPtr<GraphicsPipeline> GetOrCreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);
        SharedPtr<ComputePipeline> GetOrCreateComputePipeline(const ComputePipelineCreateInfo& createInfo);

        void EvictStalePipelines();

        void ClearCache();

    private:
        Renderer& mRenderer;

        HashMap<Uint64, SharedPtr<GraphicsPipeline>> mCachedGraphicsPipelines;
        HashMap<Uint64, SharedPtr<ComputePipeline>> mCachedComputePipelines;
    };
} // namespace cube
