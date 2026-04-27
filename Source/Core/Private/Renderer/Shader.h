#pragma once

#include "CoreHeader.h"

#include "GAPI_Pipeline.h"
#include "GAPI_Shader.h"
#include "GAPI_ShaderReflection.h"

namespace cube
{
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
        Shader(ShaderManager& manager, const ShaderCreateInfo& createInfo);
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

    // ===== GraphicsPipeline =====

    struct GraphicsPipelineInfo
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

        Uint64 GetHashValue() const;
    };

    struct GraphicsPipelineCreateInfo
    {
        GraphicsPipelineInfo pipelineInfo;

        StringView debugName;
    };

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);
        ~GraphicsPipeline() = default;

        SharedPtr<gapi::GraphicsPipeline> GetGAPIGraphicsPipeline() const { return mGAPIGraphicsPipeline; }
        const gapi::ShaderReflection& GetMergedShaderReflection() const { return mMergedShaderReflection; }

        bool HasRecompiledShadersInPipeline() const;

    private:
        void MergeShaderReflection(const gapi::ShaderReflection& reflection);
        void CacheShaderReflection(SharedPtr<Shader> vertexShader, SharedPtr<Shader> pixelShader);

        SharedPtr<gapi::GraphicsPipeline> mGAPIGraphicsPipeline;
        gapi::ShaderReflection mMergedShaderReflection;

        SharedPtr<Shader> mVertexShader;
        SharedPtr<Shader> mPixelShader;

        String mDebugName;
    };

    // ===== ComputePipeline =====

    struct ComputePipelineInfo
    {
        SharedPtr<Shader> shader;

        Uint64 GetHashValue() const;
    };

    struct ComputePipelineCreateInfo
    {
        ComputePipelineInfo pipelineInfo;

        StringView debugName;
    };

    class ComputePipeline
    {
    public:
        ComputePipeline(const ComputePipelineCreateInfo& createInfo);
        ~ComputePipeline() = default;

        SharedPtr<gapi::ComputePipeline> GetGAPIComputePipeline() const { return mGAPIComputePipeline; }
        const gapi::ShaderReflection& GetShaderReflection() const { return mShaderReflection; }

        bool HasRecompiledShaderInPipeline() const;

    private:
        void CacheShaderReflection(SharedPtr<Shader> shader);

        SharedPtr<gapi::ComputePipeline> mGAPIComputePipeline;
        gapi::ShaderReflection mShaderReflection;

        SharedPtr<Shader> mShader;

        String mDebugName;
    };
} // namespace cube
