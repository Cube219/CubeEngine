#include "PipelineManager.h"

#include "Renderer.h"
#include "Shader.h"
#include "ShaderManager.h"

namespace cube
{
    PipelineManager::PipelineManager(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    void PipelineManager::Initialize()
    {
    }

    void PipelineManager::Shutdown()
    {
        ClearCache();
    }

    SharedPtr<GraphicsPipeline> PipelineManager::GetOrCreateGraphicsPipeline(const GraphisPipelineCreateInfo& createInfo)
    {
        Uint64 hashValue = createInfo.pipelineInfo.GetHashValue();

        if (auto findIt = mCachedGraphicsPipelines.find(hashValue); findIt != mCachedGraphicsPipelines.end())
        {
            return findIt->second;
        }

        SharedPtr<GraphicsPipeline> pipeline = mRenderer.GetShaderManager().CreateGraphicsPipeline(createInfo);
        mCachedGraphicsPipelines[hashValue] = pipeline;

        return pipeline;
    }

    SharedPtr<ComputePipeline> PipelineManager::GetOrCreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        Uint64 hashValue = createInfo.pipelineInfo.GetHashValue();

        if (auto findIt = mCachedComputePipelines.find(hashValue); findIt != mCachedComputePipelines.end())
        {
            return findIt->second;
        }

        SharedPtr<ComputePipeline> pipeline = mRenderer.GetShaderManager().CreateComputePipeline(createInfo);
        mCachedComputePipelines[hashValue] = pipeline;

        return pipeline;
    }

    void PipelineManager::ClearCache()
    {
        mCachedGraphicsPipelines.clear();
        mCachedComputePipelines.clear();
    }
} // namespace cube
