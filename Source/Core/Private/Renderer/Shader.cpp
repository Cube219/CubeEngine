#include "Shader.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "FileSystem.h"
#include "PathHelper.h"
#include "Renderer.h"

namespace cube
{
    static void GetShaderFileInfosAndCodes(ArrayView<String> filePaths, StringView materialShaderCode, FrameVector<ShaderFileInfo>& outFileInfos, FrameVector<Blob>& outCodes)
    {
        outFileInfos.clear();
        outCodes.clear();

        for (const String& filePath : filePaths)
        {
            outFileInfos.push_back({});
            ShaderFileInfo& fileInfo = outFileInfos.back();
            fileInfo.path = filePath;

            outCodes.push_back({});
            Blob& code = outCodes.back();

            if (SharedPtr<platform::File> shaderFile = platform::FileSystem::OpenFile(filePath, platform::FileAccessModeFlag::Read))
            {
                Uint64 shaderFileSize = shaderFile->GetFileSize();
                Blob shaderCode(shaderFileSize);
                Uint64 readSize = shaderFile->Read(shaderCode.GetData(), shaderFileSize);
                CHECK(readSize <= shaderFileSize);

                fileInfo.lastModifiedTimes = shaderFile->GetWriteTime();
                code = std::move(shaderCode);
            }
        }

        if (!materialShaderCode.empty())
        {
            outFileInfos.push_back({});
            ShaderFileInfo& fileInfo = outFileInfos.back();
            fileInfo.path = Engine::GetShaderDirectoryPath() + CUBE_T("/MaterialShaderCode_gen.slang");
            fileInfo.lastModifiedTimes = 0;

            FrameAnsiString ansiCode = String_Convert<FrameAnsiString>(materialShaderCode);
            outCodes.push_back(Blob(ansiCode.data(), ansiCode.size()));
        }
    }

    String Shader::GetFilePathsString() const
    {
        String result;
        if (mMetaData.fileInfos.size() > 0)
        {
            result = mMetaData.fileInfos[0].path;
        }
        for (int i = 1; i < mMetaData.fileInfos.size(); ++i)
        {
            result += Format<FrameString>(CUBE_T(";{0}"), mMetaData.fileInfos[i].path);
        }

        return result;
    }

    Shader::Shader(ShaderManager& manager, const ShaderCreateInfo& createInfo) :
        mManager(manager)
    {
        FrameVector<Blob> shaderCodes;
        FrameVector<ShaderFileInfo> shaderFileInfos;
        GetShaderFileInfosAndCodes(createInfo.filePaths, createInfo.materialShaderCode, shaderFileInfos, shaderCodes);

        FrameVector<gapi::ShaderCreateInfo::ShaderCodeInfo> shaderCodeInfos;
        for (int i = 0; i < shaderFileInfos.size(); ++i)
        {
            CHECK(shaderCodes[i].GetData() != nullptr);
            shaderCodeInfos.push_back({
                .path = shaderFileInfos[i].path,
                .code = shaderCodes[i]
            });
        }

        mGAPIShader = Engine::GetRenderer()->GetGAPI().CreateShader(
        {
            .type = createInfo.type,
            .language = createInfo.language,
            .shaderCodeInfos = shaderCodeInfos,
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
            CUBE_LOG(Error, Shader, "{0}", errorMessage);
        }
        CHECK(mGAPIShader->IsValid());

        mMetaData.type = createInfo.type;
        mMetaData.language = createInfo.language;
        mMetaData.fileInfos = Vector<ShaderFileInfo>(shaderFileInfos.begin(), shaderFileInfos.end());
        mMetaData.materialShaderCode = createInfo.materialShaderCode;
        mMetaData.entryPoint = createInfo.entryPoint;
        mMetaData.debugName = createInfo.debugName;

        mRecompiledGAPIShader = nullptr;
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

        FrameVector<String> shaderFilePaths(mMetaData.fileInfos.size());
        for (int i = 0; i < shaderFilePaths.size(); ++i)
        {
            shaderFilePaths[i] = mMetaData.fileInfos[i].path;
        }
        FrameVector<Blob> shaderCodes;
        FrameVector<ShaderFileInfo> shaderFileInfos;
        GetShaderFileInfosAndCodes(shaderFilePaths, mMetaData.materialShaderCode, shaderFileInfos, shaderCodes);

        bool hasNotOpen = false;
        for (const Blob& shaderCode : shaderCodes)
        {
            if (shaderCode.GetData() == nullptr)
            {
                hasNotOpen = true;
                break;
                
            }
        }
        if (hasNotOpen)
        {
            FrameString failedFilePaths;
            for (int i = 0; i < shaderCodes.size(); ++i)
            {
                const Blob& shaderCode = shaderCodes[i];
                if (shaderCode.GetData() == nullptr)
                {
                    const ShaderFileInfo& shaderFileInfo = shaderFileInfos[i];
                    //                         Files:
                    failedFilePaths += CUBE_T("       ");
                    failedFilePaths += shaderFileInfo.path;
                }
            }

            CUBE_LOG(Warning, Shader, "Cannot open the shader file to recompile. Treat as unmodified.\nFiles:\n{0}", failedFilePaths);

            return RecompileResult::Unmodified;
        }

        bool hasModified = force ? true : false;
        CHECK(shaderFileInfos.size() == mMetaData.fileInfos.size());
        for (int i = 0; i < shaderFileInfos.size(); ++i)
        {
            const ShaderFileInfo& shaderFileInfo = shaderFileInfos[i];

            if (shaderFileInfo.lastModifiedTimes != mMetaData.fileInfos[i].lastModifiedTimes)
            {
                hasModified = true;
            }

            if (hasModified)
            {
                break;
            }
        }
        if (!hasModified)
        {
            return RecompileResult::Unmodified;
        }

        FrameVector<gapi::ShaderCreateInfo::ShaderCodeInfo> shaderCodeInfos;
        for (int i = 0; i < shaderFileInfos.size(); ++i)
        {
            CHECK(shaderCodes[i].GetData() != nullptr);
            shaderCodeInfos.push_back({
                .path = shaderFileInfos[i].path,
                .code = shaderCodes[i]
            });
        }

        mRecompiledGAPIShader = Engine::GetRenderer()->GetGAPI().CreateShader(
        {
            .type = mMetaData.type,
            .language = mMetaData.language,
            .shaderCodeInfos = shaderCodeInfos,
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
        mRecompiledShaderFileInfos = { shaderFileInfos.begin(), shaderFileInfos.end() };

        return RecompileResult::Success;
    }

    void Shader::ApplyRecompiledShader()
    {
        if (!mRecompiledGAPIShader)
        {
            CUBE_LOG(Error, Shader, "Try to apply recompiled shader but it does not existed. Ignore it.");
            return;
        }

        CHECK(mMetaData.fileInfos.size() == mRecompiledShaderFileInfos.size());
        mMetaData.fileInfos = mRecompiledShaderFileInfos;
        mGAPIShader = mRecompiledGAPIShader;

        mRecompiledShaderFileInfos.clear();
        mRecompiledGAPIShader = nullptr;
    }

    void Shader::DiscardRecompiledShader()
    {
        mRecompiledShaderFileInfos.clear();
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
