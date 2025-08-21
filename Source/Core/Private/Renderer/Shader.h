#pragma once

#include "CoreHeader.h"

#include "GAPI_Pipeline.h"
#include "GAPI_Shader.h"

namespace cube
{
    // ===== Shader =====

    struct ShaderCreateInfo
    {
        gapi::ShaderType type;
        gapi::ShaderLanguage language;

        StringView filePath;

        AnsiStringView entryPoint;

        StringView debugName;
    };

    class ShaderManager;

    class Shader
    {
    public:
        Shader(ShaderManager& manager, const ShaderCreateInfo& createInfo);
        ~Shader();

        SharedPtr<gapi::Shader> GetGAPIShader() const { return mGAPIShader; }

    private:
        friend class ShaderManager;

        ShaderManager& mManager;

        SharedPtr<gapi::Shader> mGAPIShader;
    };

    // ===== GraphicsPipeline =====

    struct GraphisPipelineCreateInfo
    {
        SharedPtr<Shader> vertexShader = nullptr;
        SharedPtr<Shader> pixelShader = nullptr;

        ArrayView<gapi::InputElement> inputLayouts;

        gapi::RasterizerState rasterizerState;

        Array<gapi::BlendState, gapi::MAX_NUM_RENDER_TARGETS> blendStates;

        gapi::DepthStencilState depthStencilState;

        gapi::PrimitiveTopologyType primitiveTopologyType = gapi::PrimitiveTopologyType::Triangle;

        Uint32 numRenderTargets;
        Array<gapi::ElementFormat, gapi::MAX_NUM_RENDER_TARGETS> renderTargetFormats;
        gapi::ElementFormat depthStencilFormat = gapi::ElementFormat::D32_Float;

        SharedPtr<gapi::ShaderVariablesLayout> shaderVariablesLayout;

        StringView debugName;
    };

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(ShaderManager& manager, const GraphisPipelineCreateInfo& createInfo);
        ~GraphicsPipeline();

        SharedPtr<gapi::GraphicsPipeline> GetGAPIGraphicsPipeline() const { return mGAPIGraphicsPipeline; }

    private:
        ShaderManager& mManager;

        SharedPtr<gapi::GraphicsPipeline> mGAPIGraphicsPipeline;
    };

    // ===== ComputePipeline =====

    struct ComputePipelineCreateInfo
    {
        SharedPtr<Shader> shader;
        SharedPtr<gapi::ShaderVariablesLayout> shaderVariablesLayout;

        StringView debugName;
    };

    class ComputePipeline
    {
    public:
        ComputePipeline(ShaderManager& manager, const ComputePipelineCreateInfo& createInfo);
        ~ComputePipeline();

        SharedPtr<gapi::ComputePipeline> GetGAPIComputePipeline() const { return mGAPIComputePipeline; }

    private:
        ShaderManager& mManager;

        SharedPtr<gapi::ComputePipeline> mGAPIComputePipeline;
    };
} // namespace cube
