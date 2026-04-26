#pragma once

#include "CoreHeader.h"

namespace cube
{
    class ComputePipeline;
    class GraphicsPipeline;
    class Renderer;
    struct ComputePipelineCreateInfo;
    struct GraphisPipelineCreateInfo;

    class PipelineManager
    {
    public:
        PipelineManager(Renderer& renderer);
        ~PipelineManager() = default;

        void Initialize();
        void Shutdown();

        SharedPtr<GraphicsPipeline> GetOrCreateGraphicsPipeline(const GraphisPipelineCreateInfo& createInfo);
        SharedPtr<ComputePipeline> GetOrCreateComputePipeline(const ComputePipelineCreateInfo& createInfo);

        void ClearCache();

    private:
        Renderer& mRenderer;

        HashMap<Uint64, SharedPtr<GraphicsPipeline>> mCachedGraphicsPipelines;
        HashMap<Uint64, SharedPtr<ComputePipeline>> mCachedComputePipelines;
    };
} // namespace cube
