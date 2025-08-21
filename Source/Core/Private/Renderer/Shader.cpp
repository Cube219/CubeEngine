#include "Shader.h"

#include "Engine.h"
#include "FileSystem.h"
#include "PathHelper.h"
#include "Renderer.h"

namespace cube
{
    Shader::Shader(ShaderManager& manager, const ShaderCreateInfo& createInfo) :
        mManager(manager)
    {
        SharedPtr<platform::File> shaderFile = platform::FileSystem::OpenFile(createInfo.filePath, platform::FileAccessModeFlag::Read);
        CHECK(shaderFile);
        Uint64 shaderFileSize = shaderFile->GetFileSize();

        Blob shaderCode(shaderFileSize);
        Uint64 readSize = shaderFile->Read(shaderCode.GetData(), shaderFileSize);
        CHECK(readSize <= shaderFileSize);

        mGAPIShader = Engine::GetRenderer()->GetGAPI().CreateShader(
        {
            .type = createInfo.type,
            .language = createInfo.language,
            .fileName = PathHelper::GetFileNameFromPath(createInfo.filePath),
            .path = createInfo.filePath,
            .code = shaderCode,
            .entryPoint = createInfo.entryPoint,
            .withDebugSymbol = true, // TODO: Add option in render ui after implement shader recompilation
            .debugName = createInfo.debugName
        });
        CHECK(mGAPIShader);
    }

    Shader::~Shader()
    {
        mGAPIShader = nullptr;

        mManager.FreeShader(this);
    }

    GraphicsPipeline::GraphicsPipeline(ShaderManager& manager, const GraphisPipelineCreateInfo& createInfo) :
        mManager(manager)
    {
        mGAPIGraphicsPipeline = Engine::GetRenderer()->GetGAPI().CreateGraphicsPipeline({
            .vertexShader = createInfo.vertexShader ? createInfo.vertexShader->GetGAPIShader() : nullptr,
            .pixelShader = createInfo.pixelShader ? createInfo.pixelShader->GetGAPIShader() : nullptr,
            .inputLayouts = createInfo.inputLayouts,
            .rasterizerState = createInfo.rasterizerState,
            .blendStates = createInfo.blendStates,
            .depthStencilState = createInfo.depthStencilState,
            .primitiveTopologyType = createInfo.primitiveTopologyType,
            .numRenderTargets = createInfo.numRenderTargets,
            .renderTargetFormats = createInfo.renderTargetFormats,
            .depthStencilFormat = createInfo.depthStencilFormat,
            .shaderVariablesLayout = createInfo.shaderVariablesLayout,
            .debugName = createInfo.debugName
        });
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        mGAPIGraphicsPipeline = nullptr;

        mManager.FreeGraphicsPipeline(this);
    }

    ComputePipeline::ComputePipeline(ShaderManager& manager, const ComputePipelineCreateInfo& createInfo) :
        mManager(manager)
    {
        mGAPIComputePipeline = Engine::GetRenderer()->GetGAPI().CreateComputePipeline({
            .shader = createInfo.shader ? createInfo.shader->GetGAPIShader() : nullptr,
            .shaderVariablesLayout = createInfo.shaderVariablesLayout,
            .debugName = createInfo.debugName
        });
    }

    ComputePipeline::~ComputePipeline()
    {
        mGAPIComputePipeline = nullptr;

        mManager.FreeComputePipeline(this);
    }
} // namespace cube
