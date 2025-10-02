#pragma once

#include "CoreHeader.h"

#include "Material.h"

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
        ShaderManager();
        ~ShaderManager() = default;

        void Initialize(GAPI* gapi, bool useDebugMode);
        void Shutdown();

        bool IsUsingDebugMode() const { return mUseDebugMode; }

        SharedPtr<Shader> CreateShader(const ShaderCreateInfo& createInfo);
        void FreeShader(Shader* shader);

        SharedPtr<GraphicsPipeline> CreateGraphicsPipeline(const GraphisPipelineCreateInfo& createInfo);
        void FreeGraphicsPipeline(GraphicsPipeline* graphicsPipeline);
        SharedPtr<ComputePipeline> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo);
        void FreeComputePipeline(ComputePipeline* computePipeline);

        void RecompileShaders(bool forceAll = false);

        MaterialShaderManager& GetMaterialShaderManager() { return mMaterialShaderManager; }

    private:
        friend class Renderer;

        GAPI* mGAPI;

        bool mUseDebugMode; // Modified in Renderer directly

        Set<Shader*> mCreatedShaders;
        Set<GraphicsPipeline*> mCreatedGraphicsPipelines;
        Set<ComputePipeline*> mCreatedComputePipelines;

        MaterialShaderManager mMaterialShaderManager;
    };
} // namespace cube
