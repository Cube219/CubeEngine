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
        if (StringView warningMessage = mGAPIShader->GetWarningMessage(); !warningMessage.empty())
        {
            CUBE_LOG(Warning, Shader, "{0}", warningMessage);
        }
        if (StringView errorMessage = mGAPIShader->GetErrorMessage(); !errorMessage.empty())
        {
            CUBE_LOG(Warning, Shader, "{0}", errorMessage);
        }
        CHECK(mGAPIShader->IsValid());

        mMetaData.type = createInfo.type;
        mMetaData.language = createInfo.language;
        mMetaData.filePath = createInfo.filePath;
        mMetaData.lastModifiedTime = shaderFile->GetWriteTime();
        mMetaData.entryPoint = createInfo.entryPoint;
        mMetaData.debugName = createInfo.debugName;

        mRecompiledGAPIShader = nullptr;
        mLastModifiedTimeInRecompiled = 0;
        mRecompileCount = 0;
    }

    Shader::~Shader()
    {
        mRecompiledGAPIShader = nullptr;
        mGAPIShader = nullptr;

        mManager.FreeShader(this);
    }

    Shader::RecompileResult Shader::TryRecompileShader(String& outErrorMessage, bool force)
    {
        if (mRecompiledGAPIShader)
        {
            CUBE_LOG(Error, Shader, "Try to recompile shader but not applied recompiled shader exist. Overwrite it.");
        }

        SharedPtr<platform::File> shaderFile = platform::FileSystem::OpenFile(mMetaData.filePath, platform::FileAccessModeFlag::Read);
        if (!shaderFile)
        {
            CUBE_LOG(Warning, Shader, "Cannot open the shader file to recompile. Treat as unmodified.");
            return RecompileResult::Unmodified;
        }

        Time modifiedTime = shaderFile->GetWriteTime();
        if (!force && modifiedTime == mMetaData.lastModifiedTime)
        {
            return RecompileResult::Unmodified;
        }

        Uint64 shaderFileSize = shaderFile->GetFileSize();
        Blob shaderCode(shaderFileSize);
        Uint64 readSize = shaderFile->Read(shaderCode.GetData(), shaderFileSize);
        CHECK(readSize <= shaderFileSize);

        mRecompiledGAPIShader = Engine::GetRenderer()->GetGAPI().CreateShader(
        {
            .type = mMetaData.type,
            .language = mMetaData.language,
            .fileName = PathHelper::GetFileNameFromPath(mMetaData.filePath),
            .path = mMetaData.filePath,
            .code = shaderCode,
            .entryPoint = mMetaData.entryPoint,
            .withDebugSymbol = true, // TODO: Add option in render ui after implement shader recompilation
            .debugName = Format<String>(CUBE_T("{0}:{1}"), mMetaData.debugName, mRecompileCount + 1)
        });

        StringView warningMessage = mRecompiledGAPIShader->GetWarningMessage();
        StringView errorMessage = mRecompiledGAPIShader->GetErrorMessage();

        if (!mRecompiledGAPIShader->IsValid())
        {
            outErrorMessage = Format(CUBE_T("{0}\n{1}"), warningMessage, errorMessage);

            return RecompileResult::Failed;
        }

        mRecompileCount++;

        return RecompileResult::Success;
    }

    void Shader::ApplyRecompiledShader()
    {
        if (!mRecompiledGAPIShader)
        {
            CUBE_LOG(Error, Shader, "Try to apply recompiled shader but it does not existed. Ignore it.");
            return;
        }

        mMetaData.lastModifiedTime = mLastModifiedTimeInRecompiled;
        mGAPIShader = mRecompiledGAPIShader;

        mLastModifiedTimeInRecompiled = 0;
        mRecompiledGAPIShader = nullptr;
    }

    void Shader::DiscardRecompiledShader()
    {
        mRecompiledGAPIShader = nullptr;
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

        mRecreateInfo.CopyFromCreateInfo(createInfo);
        mRecreateCount = 0;
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        mGAPIGraphicsPipeline = nullptr;

        mManager.FreeGraphicsPipeline(this);
    }

    bool GraphicsPipeline::HasRecompiledShadersInPipeline() const
    {
        bool result = false;

        if (mRecreateInfo.vertexShader && mRecreateInfo.vertexShader->HasRecompiledShader())
        {
            result = true;
        }
        else if (mRecreateInfo.pixelShader && mRecreateInfo.pixelShader->HasRecompiledShader())
        {
            result = true;
        }

        return result;
    }

    void GraphicsPipeline::RecreateGraphicsPipeline()
    {
        mGAPIGraphicsPipeline = Engine::GetRenderer()->GetGAPI().CreateGraphicsPipeline({
            .vertexShader = mRecreateInfo.vertexShader ? mRecreateInfo.vertexShader->GetGAPIShader() : nullptr,
            .pixelShader = mRecreateInfo.pixelShader ? mRecreateInfo.pixelShader->GetGAPIShader() : nullptr,
            .inputLayouts = mRecreateInfo.inputLayouts,
            .rasterizerState = mRecreateInfo.rasterizerState,
            .blendStates = mRecreateInfo.blendStates,
            .depthStencilState = mRecreateInfo.depthStencilState,
            .primitiveTopologyType = mRecreateInfo.primitiveTopologyType,
            .numRenderTargets = mRecreateInfo.numRenderTargets,
            .renderTargetFormats = mRecreateInfo.renderTargetFormats,
            .depthStencilFormat = mRecreateInfo.depthStencilFormat,
            .shaderVariablesLayout = mRecreateInfo.shaderVariablesLayout,
            .debugName = Format<String>(CUBE_T("{0}:{1}"), mRecreateInfo.debugName, mRecreateCount + 1)
        });

        mRecreateCount++;
    }

    ComputePipeline::ComputePipeline(ShaderManager& manager, const ComputePipelineCreateInfo& createInfo) :
        mManager(manager)
    {
        mGAPIComputePipeline = Engine::GetRenderer()->GetGAPI().CreateComputePipeline({
            .shader = createInfo.shader ? createInfo.shader->GetGAPIShader() : nullptr,
            .shaderVariablesLayout = createInfo.shaderVariablesLayout,
            .debugName = createInfo.debugName
        });

        mRecreateInfo.CopyFromCreateInfo(createInfo);
        mRecreateCount = 0;
    }

    ComputePipeline::~ComputePipeline()
    {
        mGAPIComputePipeline = nullptr;

        mManager.FreeComputePipeline(this);
    }

    bool ComputePipeline::HasRecompiledShaderInPipeline() const
    {
        return mRecreateInfo.shader && mRecreateInfo.shader->HasRecompiledShader() ? true : false;
    }

    void ComputePipeline::RecreateComputePipeline()
    {
        mGAPIComputePipeline = Engine::GetRenderer()->GetGAPI().CreateComputePipeline({
            .shader = mRecreateInfo.shader ? mRecreateInfo.shader->GetGAPIShader() : nullptr,
            .shaderVariablesLayout = mRecreateInfo.shaderVariablesLayout,
            .debugName = Format<String>(CUBE_T("{0}:{1}"), mRecreateInfo.debugName, mRecreateCount + 1)
        });

        mRecreateCount++;
    }
} // namespace cube
