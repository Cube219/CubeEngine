#include "Shader.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Engine.h"
#include "FileSystem.h"
#include "Logger.h"
#include "Renderer.h"

namespace cube
{
    static void GetShaderFileInfosAndCodes(ArrayView<platform::FilePath> filePaths, StringView materialShaderCode, FrameVector<ShaderFileInfo>& outFileInfos, FrameVector<Blob>& outCodes)
    {
        outFileInfos.clear();
        outCodes.clear();

        for (const platform::FilePath& filePath : filePaths)
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
            fileInfo.isGeneratedShader = true;
            fileInfo.path = Engine::GetShaderDirectoryPath() / CUBE_T("MaterialShaderCode_gen.slang");
            fileInfo.lastModifiedTimes = 0;

            FrameAnsiString ansiCode = String_Convert<FrameAnsiString>(materialShaderCode);
            outCodes.push_back(Blob(ansiCode.data(), ansiCode.size()));
        }
    }

    static FrameVector<ShaderFileInfo> GetDependencyFileInfos(ArrayView<const platform::FilePath> dependencyFilePaths, ArrayView<const ShaderFileInfo> excludeFileInfos)
    {
        FrameVector<ShaderFileInfo> dependencyFileInfos;

        for (const platform::FilePath& dependencyFilePath : dependencyFilePaths)
        {
            bool isExcluded = false;
            for (const ShaderFileInfo& excludeFileInfo : excludeFileInfos)
            {
                if (dependencyFilePath == excludeFileInfo.path)
                {
                    isExcluded = true;
                    break;
                }
            }
            if (isExcluded)
            {
                continue;
            }

            ShaderFileInfo fileInfo;
            fileInfo.path = dependencyFilePath;
            if (SharedPtr<platform::File> depFile = platform::FileSystem::OpenFile(dependencyFilePath, platform::FileAccessModeFlag::Read))
            {
                fileInfo.lastModifiedTimes = depFile->GetWriteTime();
            }
            dependencyFileInfos.push_back(std::move(fileInfo));
        }

        return dependencyFileInfos;
    }

    String Shader::GetFilePathsString() const
    {
        String result;
        if (mFileInfos.size() > 0)
        {
            result = mFileInfos[0].path.ToString();
        }
        for (int i = 1; i < mFileInfos.size(); ++i)
        {
            result += Format<FrameString>(CUBE_T(";{0}"), mFileInfos[i].path.ToString());
        }

        return result;
    }

    Shader::Shader(ShaderManager& manager, GAPI& gapi, const ShaderCreateInfo& createInfo)
        : mManager(manager)
        , mGAPI(gapi)
    {
        mShaderInfo = createInfo.shaderInfo;
        mDebugName = createInfo.debugName;
        mMaterialShaderCode = createInfo.materialShaderCode;

        FrameVector<Blob> shaderCodes;
        FrameVector<ShaderFileInfo> shaderFileInfos;
        GetShaderFileInfosAndCodes(createInfo.filePaths, createInfo.materialShaderCode, shaderFileInfos, shaderCodes);
        mFileInfos = { shaderFileInfos.begin(), shaderFileInfos.end() };

        FrameVector<gapi::ShaderCreateInfo::ShaderCodeInfo> shaderCodeInfos;
        for (int i = 0; i < shaderFileInfos.size(); ++i)
        {
            CHECK(shaderCodes[i].GetData() != nullptr);
            shaderCodeInfos.push_back({
                .path = shaderFileInfos[i].path,
                .code = shaderCodes[i]
            });
        }

        mGAPIShader = mGAPI.CreateShader(
        {
            .type = mShaderInfo.type,
            .language = mShaderInfo.language,
            .shaderCodeInfos = shaderCodeInfos,
            .entryPoint = mShaderInfo.entryPoint,
            .preprocessorDefines = mShaderInfo.defines,
            .withDebugSymbol = manager.IsUsingDebugMode(),
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

        FrameVector<ShaderFileInfo> dependencyFileInfos = GetDependencyFileInfos(mGAPIShader->GetDependencyFilePaths(), mFileInfos);
        mDependencyFileInfos = { dependencyFileInfos.begin(), dependencyFileInfos.end() };

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

        FrameVector<platform::FilePath> shaderFilePaths;
        for (const ShaderFileInfo& shaderFileInfo : mFileInfos)
        {
            if (!shaderFileInfo.isGeneratedShader)
            {
                shaderFilePaths.push_back(shaderFileInfo.path);
            }
        }
        FrameVector<Blob> shaderCodes;
        FrameVector<ShaderFileInfo> shaderFileInfos;
        GetShaderFileInfosAndCodes(shaderFilePaths, mMaterialShaderCode, shaderFileInfos, shaderCodes);

        FrameVector<platform::FilePath> dependencyFilePaths;
        for (const ShaderFileInfo& dependencyFileInfo : mDependencyFileInfos)
        {
            dependencyFilePaths.push_back(dependencyFileInfo.path);
        }
        FrameVector<ShaderFileInfo> dependencyFileInfos = GetDependencyFileInfos(dependencyFilePaths, shaderFileInfos);

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
                    failedFilePaths += shaderFileInfo.path.ToString();
                }
            }

            CUBE_LOG(Warning, Shader, "Cannot open the shader file to recompile. Treat as unmodified.\nFiles:\n{0}", failedFilePaths);

            return RecompileResult::Unmodified;
        }
        CHECK(shaderFileInfos.size() == mFileInfos.size());
        CHECK(dependencyFileInfos.size() == mDependencyFileInfos.size());

        bool hasModified = force ? true : false;
        if (!hasModified)
        {
            for (int i = 0; i < shaderFileInfos.size(); ++i)
            {
                const ShaderFileInfo& shaderFileInfo = shaderFileInfos[i];

                if (shaderFileInfo.lastModifiedTimes != mFileInfos[i].lastModifiedTimes)
                {
                    hasModified = true;
                    break;
                }
            }
        }
        // Also check dependency files.
        if (!hasModified)
        {
            for (int i = 0; i < dependencyFileInfos.size(); ++i)
            {
                const ShaderFileInfo& dependencyFileInfo = dependencyFileInfos[i];

                if (dependencyFileInfo.lastModifiedTimes != mDependencyFileInfos[i].lastModifiedTimes)
                {
                    hasModified = true;
                    break;
                }
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

        mRecompiledGAPIShader = mGAPI.CreateShader(
        {
            .type = mShaderInfo.type,
            .language = mShaderInfo.language,
            .shaderCodeInfos = shaderCodeInfos,
            .entryPoint = mShaderInfo.entryPoint,
            .preprocessorDefines = mShaderInfo.defines,
            .withDebugSymbol = mManager.IsUsingDebugMode(),
            .debugName = Format<String>(CUBE_T("{0}:{1}"), mDebugName, mRecompileCount + 1)
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
        mRecompiledDependencyFileInfos = { dependencyFileInfos.begin(), dependencyFileInfos.end() };

        return RecompileResult::Success;
    }

    void Shader::ApplyRecompiledShader()
    {
        if (!mRecompiledGAPIShader)
        {
            CUBE_LOG(Error, Shader, "Try to apply recompiled shader but it does not existed. Ignore it.");
            return;
        }

        CHECK(mFileInfos.size() == mRecompiledShaderFileInfos.size());
        mFileInfos = mRecompiledShaderFileInfos;
        mDependencyFileInfos = mRecompiledDependencyFileInfos;
        mGAPIShader = mRecompiledGAPIShader;

        mRecompiledShaderFileInfos.clear();
        mRecompiledDependencyFileInfos.clear();
        mRecompiledGAPIShader = nullptr;
    }

    void Shader::DiscardRecompiledShader()
    {
        mRecompiledShaderFileInfos.clear();
        mRecompiledDependencyFileInfos.clear();
        mRecompiledGAPIShader = nullptr;
    }

    ShaderManager::ShaderManager(Renderer& renderer)
        : mMaterialShaderManager(*this, renderer.GetPipelineManager())
        , mRenderer(renderer)
    {
    }

    void ShaderManager::Initialize(bool useDebugMode)
    {
        mUseDebugMode = useDebugMode;

        mMaterialShaderManager.Initialize(&mRenderer.GetGAPI());
    }

    void ShaderManager::Shutdown()
    {
        mMaterialShaderManager.Shutdown();

        CHECK_FORMAT(mCreatedShaders.size() == 0, "Not all shaders are freeed!");
    }

    SharedPtr<Shader> ShaderManager::CreateShader(const ShaderCreateInfo& createInfo)
    {
        SharedPtr<Shader> shader = std::make_shared<Shader>(*this, mRenderer.GetGAPI(), createInfo);
        mCreatedShaders.insert(shader.get());

        return shader;
    }

    void ShaderManager::FreeShader(Shader* shader)
    {
        auto it = mCreatedShaders.find(shader);
        CHECK(it != mCreatedShaders.end());

        mCreatedShaders.erase(it);
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
            // Evict cached pipelines that reference any recompiled shader BEFORE applying,
            // because EvictStalePipelines reads the HasRecompiledShader flag.
            mRenderer.GetPipelineManager().EvictStalePipelines();

            CUBE_LOG(Info, Shader, "===== Succeeded recompile shaders =====");

            for (Shader* shader : succeededShaders)
            {
                shader->ApplyRecompiledShader();

                CUBE_LOG(Info, Shader, "\nDebugName: {0}\nPath: {1}\nEntryPoint: {2}\n",
                    shader->GetDebugName(), shader->GetFilePathsString(), shader->GetEntryPoint());
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
