#pragma once

#include "CoreHeader.h"

namespace cube
{
    class ComputePipeline;
    class GraphicsPipeline;
    class Renderer;
    struct ComputePipelineCreateInfo;
    struct GraphicsPipelineCreateInfo;

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
