#pragma once

#include "CoreHeader.h"

#include "GAPI_Shader.h"
#include "Material.h"

namespace cube
{
    class GAPI;
    class Renderer;
    class ShaderManager;

    // ===== Shader =====

    struct ShaderInfo
    {
        gapi::ShaderType type;
        gapi::ShaderLanguage language = gapi::ShaderLanguage::Slang;

        AnsiString entryPoint;

        Vector<gapi::PreprocessorDefine> defines;
    };

    struct ShaderCreateInfo
    {
        ShaderInfo shaderInfo;

        // Only slang can support multiple shader code and material shader code.
        ArrayView<platform::FilePath> filePaths;
        StringView materialShaderCode;

        StringView debugName;
    };

    struct ShaderFileInfo
    {
        bool isGeneratedShader = false;
        platform::FilePath path;
        Time lastModifiedTimes = 0;
    };

    class Shader
    {
    public:
        Shader(ShaderManager& manager, GAPI& gapi, const ShaderCreateInfo& createInfo);
        ~Shader();

        SharedPtr<gapi::Shader> GetGAPIShader() const { return mGAPIShader; }

        String GetFilePathsString() const;
        AnsiStringView GetEntryPoint() const { return mShaderInfo.entryPoint; }
        StringView GetDebugName() const { return mDebugName; }

        bool HasRecompiledShader() const { return mRecompiledGAPIShader != nullptr; }

    private:
        friend class ShaderManager;

        enum class RecompileResult
        {
            Success,
            Failed,
            Unmodified
        };
        RecompileResult TryRecompileShader(String& outErrorMessage, bool force = false);
        void ApplyRecompiledShader();
        void DiscardRecompiledShader();

        ShaderManager& mManager;
        GAPI& mGAPI;

        SharedPtr<gapi::Shader> mGAPIShader;

        ShaderInfo mShaderInfo;
        Vector<ShaderFileInfo> mFileInfos;
        Vector<ShaderFileInfo> mDependencyFileInfos;
        String mMaterialShaderCode;

        String mDebugName;

        SharedPtr<gapi::Shader> mRecompiledGAPIShader;
        Vector<ShaderFileInfo> mRecompiledShaderFileInfos;
        Vector<ShaderFileInfo> mRecompiledDependencyFileInfos;
        int mRecompileCount;
    };

    class ShaderManager
    {
    public:
        ShaderManager(Renderer& renderer);
        ~ShaderManager() = default;

        void Initialize(bool useDebugMode);
        void Shutdown();

        bool IsUsingDebugMode() const { return mUseDebugMode; }

        SharedPtr<Shader> CreateShader(const ShaderCreateInfo& createInfo);
        void FreeShader(Shader* shader);

        void RecompileShaders(bool forceAll = false);

        MaterialShaderManager& GetMaterialShaderManager() { return mMaterialShaderManager; }

    private:
        friend class Renderer;

        Renderer& mRenderer;

        bool mUseDebugMode; // Modified in Renderer directly

        Set<Shader*> mCreatedShaders;

        MaterialShaderManager mMaterialShaderManager;
    };
} // namespace cube
