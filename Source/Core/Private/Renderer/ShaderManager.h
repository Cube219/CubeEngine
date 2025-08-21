#pragma once

#include "CoreHeader.h"

namespace cube
{
    struct ComputePipelineCreateInfo;
    class ComputePipeline;
    class GAPI;
    struct GraphisPipelineCreateInfo;
    class GraphicsPipeline;
    struct ShaderCreateInfo;
    class Shader;

    class ShaderManager
    {
    public:
        ShaderManager() = default;
        ~ShaderManager() = default;

        void Initialize(GAPI* gapi);
        void Shutdown();

        SharedPtr<Shader> CreateShader(const ShaderCreateInfo& createInfo);
        void FreeShader(Shader* shader);

        SharedPtr<GraphicsPipeline> CreateGraphicsPipeline(const GraphisPipelineCreateInfo& createInfo);
        void FreeGraphicsPipeline(GraphicsPipeline* graphicsPipeline);
        SharedPtr<ComputePipeline> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo);
        void FreeComputePipeline(ComputePipeline* computePipeline);

    private:
        GAPI* mGAPI;

        Set<Shader*> mCreatedShaders;
        Set<GraphicsPipeline*> mCreatedGraphicsPipeline;
        Set<ComputePipeline*> mCreatedComputePipeline;
    };
} // namespace cube
