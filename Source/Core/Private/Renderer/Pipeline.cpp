#include "Pipeline.h"

#include "Checker.h"
#include "Engine.h"
#include "Logger.h"
#include "Renderer.h"

namespace cube
{
    gapi::GraphicsPipelineInfo GraphicsPipelineInfo::GetGAPIVersion() const
    {
        return gapi::GraphicsPipelineInfo{
            .vertexShader = vertexShader ? vertexShader->GetGAPIShader() : nullptr,
            .pixelShader = pixelShader ? pixelShader->GetGAPIShader() : nullptr,
            .inputLayouts = inputLayouts,
            .rasterizerState = rasterizerState,
            .blendStates = blendStates,
            .depthStencilState = depthStencilState,
            .primitiveTopologyType = primitiveTopologyType,
            .numRenderTargets = numRenderTargets,
            .renderTargetFormats = renderTargetFormats,
            .depthStencilFormat = depthStencilFormat
        };
    }

    void GraphicsPipelineInfo::CalculateHashValue()
    {
        HashValue = GetGAPIVersion().GetHashValue();
    }

    bool GraphicsPipelineInfo::ValidateHashValue() const
    {
        return HashValue == GetGAPIVersion().GetHashValue();
    }

    gapi::ComputePipelineInfo ComputePipelineInfo::GetGAPIVersion() const
    {
        return gapi::ComputePipelineInfo{
            .shader = shader ? shader->GetGAPIShader() : nullptr
        };
    }

    void ComputePipelineInfo::CalculateHashValue()
    {
        HashValue = GetGAPIVersion().GetHashValue();
    }

    bool ComputePipelineInfo::ValidateHashValue() const
    {
        return HashValue == GetGAPIVersion().GetHashValue();
    }

    GraphicsPipeline::GraphicsPipeline(GAPI& gapi, const GraphicsPipelineCreateInfo& createInfo)
    {
        mVertexShader = createInfo.pipelineInfo.vertexShader;
        mPixelShader = createInfo.pipelineInfo.pixelShader;
        mDebugName = createInfo.debugName;

        mGAPIGraphicsPipeline = gapi.CreateGraphicsPipeline({
            .pipelineInfo = createInfo.pipelineInfo.GetGAPIVersion(),
            .debugName = createInfo.debugName
        });

        CacheShaderReflection(mVertexShader, mPixelShader);
    }

    bool GraphicsPipeline::HasRecompiledShadersInPipeline() const
    {
        bool result = false;

        if (mVertexShader && mVertexShader->HasRecompiledShader())
        {
            result = true;
        }
        else if (mPixelShader && mPixelShader->HasRecompiledShader())
        {
            result = true;
        }

        return result;
    }

    void GraphicsPipeline::MergeShaderReflection(const gapi::ShaderReflection& reflection)
    {
        for (const gapi::ShaderParameterBlockReflection& block : reflection.blocks)
        {
            bool found = false;
            for (const gapi::ShaderParameterBlockReflection& existingBlock : mMergedShaderReflection.blocks)
            {
                if (existingBlock.typeName == block.typeName)
                {
                    if (existingBlock.index == block.index)
                    {
                        CUBE_LOG(Error, Pipeline, "Found duplicated binding index in the same shader type! (Name: {0} / index: {1}, {2}) Use the former one.",
                            block.typeName, existingBlock.index, block.index);
                        break;
                    }
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                mMergedShaderReflection.blocks.push_back(block);
            }
        }

        mMergedShaderReflection.name += CUBE_T(";");
        mMergedShaderReflection.name += reflection.name;
    }

    void GraphicsPipeline::CacheShaderReflection(SharedPtr<Shader> vertexShader, SharedPtr<Shader> pixelShader)
    {
        mMergedShaderReflection = {};

        if (vertexShader)
        {
            MergeShaderReflection(vertexShader->GetGAPIShader()->GetReflection());
        }

        if (pixelShader)
        {
            MergeShaderReflection(pixelShader->GetGAPIShader()->GetReflection());
        }
    }

    ComputePipeline::ComputePipeline(GAPI& gapi, const ComputePipelineCreateInfo& createInfo)
    {
        mShader = createInfo.pipelineInfo.shader;
        mDebugName = createInfo.debugName;

        mGAPIComputePipeline = gapi.CreateComputePipeline({
            .pipelineInfo = createInfo.pipelineInfo.GetGAPIVersion(),
            .debugName = createInfo.debugName
        });

        CacheShaderReflection(mShader);
    }

    bool ComputePipeline::HasRecompiledShaderInPipeline() const
    {
        return mShader && mShader->HasRecompiledShader();
    }

    void ComputePipeline::CacheShaderReflection(SharedPtr<Shader> shader)
    {
        if (shader)
        {
            mShaderReflection = shader->GetGAPIShader()->GetReflection();
        }
        else
        {
            mShaderReflection = {};
        }
    }

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
#if CUBE_USE_CHECK
        if (!createInfo.pipelineInfo.ValidateHashValue())
        {
            NO_ENTRY_FORMAT("Hash value mismatch found! Did you forget to call CalculateHashValue() after changing pipeline data?");
        }
#endif

        if (auto findIt = mCachedGraphicsPipelines.find(hashValue); findIt != mCachedGraphicsPipelines.end())
        {
            return findIt->second;
        }

        SharedPtr<GraphicsPipeline> pipeline = std::make_shared<GraphicsPipeline>(mRenderer.GetGAPI(), createInfo);
        CHECK(mRenderer.GetShaderParameterListManager().ValidateShader(pipeline->GetMergedShaderReflection()));
        mCachedGraphicsPipelines[hashValue] = pipeline;

        return pipeline;
    }

    SharedPtr<ComputePipeline> PipelineManager::GetOrCreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        Uint64 hashValue = createInfo.pipelineInfo.GetHashValue();
#if CUBE_USE_CHECK
        if (!createInfo.pipelineInfo.ValidateHashValue())
        {
            NO_ENTRY_FORMAT("Hash value mismatch found! Did you forget to call CalculateHashValue() after changing pipeline data?");
        }
#endif

        if (auto findIt = mCachedComputePipelines.find(hashValue); findIt != mCachedComputePipelines.end())
        {
            return findIt->second;
        }

        SharedPtr<ComputePipeline> pipeline = std::make_shared<ComputePipeline>(mRenderer.GetGAPI(), createInfo);
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
