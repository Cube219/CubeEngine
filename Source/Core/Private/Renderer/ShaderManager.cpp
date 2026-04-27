#include "ShaderManager.h"

#include "Checker.h"
#include "PipelineManager.h"
#include "Renderer.h"
#include "Shader.h"

namespace cube
{
    ShaderManager::ShaderManager(Renderer& renderer)
        : mMaterialShaderManager(*this)
        , mRenderer(renderer)
    {
    }

    void ShaderManager::Initialize(GAPI* gapi, bool useDebugMode)
    {
        mGAPI = gapi;
        mUseDebugMode = useDebugMode;

        mMaterialShaderManager.Initialize(gapi);
    }

    void ShaderManager::Shutdown()
    {
        mMaterialShaderManager.Shutdown();

        CHECK_FORMAT(mCreatedShaders.size() == 0, "Not all shaders are freeed!");
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
