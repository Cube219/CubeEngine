#include "ShaderManager.h"

#include "Checker.h"
#include "Shader.h"

namespace cube
{
    void ShaderManager::Initialize(GAPI* gapi)
    {
        mGAPI = gapi;
    }

    void ShaderManager::Shutdown()
    {
        CHECK_FORMAT(mCreatedShaders.size() == 0, "Not all shaders are freeed!");
        CHECK_FORMAT(mCreatedGraphicsPipeline.size() == 0, "Not all graphics pipelines are freeed!");
        CHECK_FORMAT(mCreatedComputePipeline.size() == 0, "Not all compute pipelines are freeed!");
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
        mCreatedGraphicsPipeline.insert(graphicsPipeline.get());

        return graphicsPipeline;
    }

    void ShaderManager::FreeGraphicsPipeline(GraphicsPipeline* graphicsPipeline)
    {
        auto it = mCreatedGraphicsPipeline.find(graphicsPipeline);
        CHECK(it != mCreatedGraphicsPipeline.end());

        mCreatedGraphicsPipeline.erase(it);
    }

    SharedPtr<ComputePipeline> ShaderManager::CreateComputePipeline(const ComputePipelineCreateInfo& createInfo)
    {
        SharedPtr<ComputePipeline> computePipeline = std::make_shared<ComputePipeline>(*this, createInfo);
        mCreatedComputePipeline.insert(computePipeline.get());

        return computePipeline;
    }

    void ShaderManager::FreeComputePipeline(ComputePipeline* computePipeline)
    {
        auto it = mCreatedComputePipeline.find(computePipeline);
        CHECK(it != mCreatedComputePipeline.end());

        mCreatedComputePipeline.erase(it);
    }
} // namespace cube
