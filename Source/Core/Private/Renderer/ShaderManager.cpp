#include "ShaderManager.h"

#include "Checker.h"
#include "Shader.h"
#include "Allocator/FrameAllocator.h"

namespace cube
{
    ShaderManager::ShaderManager()
        : mMaterialShaderManager(*this)
    {
    }

    void ShaderManager::Initialize(GAPI* gapi)
    {
        mGAPI = gapi;

        mMaterialShaderManager.Initialize(gapi);
    }

    void ShaderManager::Shutdown()
    {
        mMaterialShaderManager.Shutdown();

        CHECK_FORMAT(mCreatedShaders.size() == 0, "Not all shaders are freeed!");
        CHECK_FORMAT(mCreatedGraphicsPipelines.size() == 0, "Not all graphics pipelines are freeed!");
        CHECK_FORMAT(mCreatedComputePipelines.size() == 0, "Not all compute pipelines are freeed!");
    }

    SharedPtr<Shader> ShaderManager::CreateShader(const ShaderCreateInfo& createInfo)
    {
        SharedPtr<Shader> shader = std::make_shared<Shader>(*this, createInfo);
        mCreatedShaders.insert(shader.get());

        return shader;
    }

    void ShaderManager::FreeShader(Shader* shader)
    {
        auto it = mCreatedShaders.find(shader);
        CHECK(it != mCreatedShaders.end());

        mCreatedShaders.erase(it);
    }

    SharedPtr<GraphicsPipeline> ShaderManager::CreateGraphicsPipeline(const GraphisPipelineCreateInfo& createInfo)
    {
        SharedPtr<GraphicsPipeline> graphicsPipeline = std::make_shared<GraphicsPipeline>(*this, createInfo);
        mCreatedGraphicsPipelines.insert(graphicsPipeline.get());

        return graphicsPipeline;
    }

    void ShaderManager::FreeGraphicsPipeline(GraphicsPipeline* graphicsPipeline)
    {
        auto it = mCreatedGraphicsPipelines.find(graphicsPipeline);
        CHECK(it != mCreatedGraphicsPipelines.end());

        mCreatedGraphicsPipelines.erase(it);
    }

    SharedPtr<ComputePipeline> ShaderManager::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        SharedPtr<ComputePipeline> computePipeline = std::make_shared<ComputePipeline>(*this, createInfo);
        mCreatedComputePipelines.insert(computePipeline.get());

        return computePipeline;
    }

    void ShaderManager::FreeComputePipeline(ComputePipeline* computePipeline)
    {
        auto it = mCreatedComputePipelines.find(computePipeline);
        CHECK(it != mCreatedComputePipelines.end());

        mCreatedComputePipelines.erase(it);
    }

    void ShaderManager::RecompileShaders(bool forceAll)
    {
        CUBE_LOG(Info, Shader, "Try to recompile shaders... (Force: {0})", forceAll);

        struct FailedShader
        {
            Shader* shader;
            String message;
        };

        String errorMessageBuffer;
        int numRecompileShaders = 0;
        Vector<Shader*> succeededShaders;
        Vector<FailedShader> failedShaders;

        for (Shader* shader : mCreatedShaders)
        {
            errorMessageBuffer.clear();
            Shader::RecompileResult result = shader->TryRecompileShader(errorMessageBuffer, forceAll);

            if (result == Shader::RecompileResult::Unmodified)
            {
                continue;
            }

            numRecompileShaders++;
            if (result == Shader::RecompileResult::Success)
            {
                succeededShaders.push_back(shader);
            }
            else if (result == Shader::RecompileResult::Failed)
            {
                failedShaders.push_back({ .shader = shader, .message = errorMessageBuffer });
            }
        }

        CUBE_LOG(Info, Shader, "Shader recompilation finished. (Success: {0} / Fail: {1})", succeededShaders.size(), failedShaders.size());

        if (!succeededShaders.empty())
        {
            Vector<GraphicsPipeline*> graphicsPipelinesToRecreate;
            Vector<ComputePipeline*> computePipelinesToRecreate;

            for (GraphicsPipeline* graphicsPipeline : mCreatedGraphicsPipelines)
            {
                if (graphicsPipeline->HasRecompiledShadersInPipeline())
                {
                    graphicsPipelinesToRecreate.push_back(graphicsPipeline);
                }
            }
            for (ComputePipeline* computePipeline : mCreatedComputePipelines)
            {
                if (computePipeline->HasRecompiledShaderInPipeline())
                {
                    computePipelinesToRecreate.push_back(computePipeline);
                }
            }

            CUBE_LOG(Info, Shader, "===== Succeeded recompile shaders =====");

            for (Shader* shader : succeededShaders)
            {
                shader->ApplyRecompiledShader();

                CUBE_LOG(Info, Shader, "\nDebugName: {0}\nPath: {1}\nEntryPoint: {2}\n",
                    shader->GetDebugName(), shader->GetFilePathsString(), shader->GetEntryPoint());
            }

            for (GraphicsPipeline* graphicsPipeline : graphicsPipelinesToRecreate)
            {
                graphicsPipeline->RecreateGraphicsPipeline();
            }
            for (ComputePipeline* computePipeline : computePipelinesToRecreate)
            {
                computePipeline->RecreateComputePipeline();
            }
        }

        if (!failedShaders.empty())
        {
            CUBE_LOG(Info, Shader, "===== Failed recompile shaders =====");
            for (const FailedShader& failedShader : failedShaders)
            {
                CUBE_LOG(Info, Shader, "\nDebugName: {0}\nPath: {1}\nEntryPoint: {2}\nMessages: {3}",
                    failedShader.shader->GetDebugName(), failedShader.shader->GetFilePathsString(), failedShader.shader->GetEntryPoint(),
                    failedShader.message);

                failedShader.shader->DiscardRecompiledShader();
            }
        }
    }
} // namespace cube
