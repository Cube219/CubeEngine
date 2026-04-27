#include "PipelineManager.h"

#include "Checker.h"
#include "Renderer.h"
#include "Shader.h"

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

    SharedPtr<GraphicsPipeline> PipelineManager::GetOrCreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
    {
        Uint64 hashValue = createInfo.pipelineInfo.GetHashValue();

        if (auto findIt = mCachedGraphicsPipelines.find(hashValue); findIt != mCachedGraphicsPipelines.end())
        {
            return findIt->second;
        }

        SharedPtr<GraphicsPipeline> pipeline = std::make_shared<GraphicsPipeline>(createInfo);
        CHECK(mRenderer.GetShaderParameterListManager().ValidateShader(pipeline->GetMergedShaderReflection()));
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

        SharedPtr<ComputePipeline> pipeline = std::make_shared<ComputePipeline>(createInfo);
        CHECK(mRenderer.GetShaderParameterListManager().ValidateShader(pipeline->GetShaderReflection()));
        mCachedComputePipelines[hashValue] = pipeline;

        return pipeline;
    }

    void PipelineManager::EvictStalePipelines()
    {
        for (auto it = mCachedGraphicsPipelines.begin(); it != mCachedGraphicsPipelines.end();)
        {
            if (it->second->HasRecompiledShadersInPipeline())
            {
                it = mCachedGraphicsPipelines.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (auto it = mCachedComputePipelines.begin(); it != mCachedComputePipelines.end();)
        {
            if (it->second->HasRecompiledShaderInPipeline())
            {
                it = mCachedComputePipelines.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void PipelineManager::ClearCache()
    {
        mCachedGraphicsPipelines.clear();
        mCachedComputePipelines.clear();
    }
} // namespace cube
