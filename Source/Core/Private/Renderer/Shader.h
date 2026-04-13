#pragma once

#include "CoreHeader.h"

#include "GAPI_Pipeline.h"
#include "GAPI_Shader.h"
#include "GAPI_ShaderReflection.h"

namespace cube
{
    // ===== Shader =====

    struct ShaderCreateInfo
    {
        gapi::ShaderType type;
        gapi::ShaderLanguage language = gapi::ShaderLanguage::Slang;

        // Only slang can support multiple shader code and material shader code.
        ArrayView<platform::FilePath> filePaths;
        StringView materialShaderCode;
        AnsiStringView entryPoint;

        ArrayView<gapi::PreprocessorDefine> defines;

        StringView debugName;
    };

    class ShaderManager;

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
        AnsiStringView GetEntryPoint() const { return mMetaData.entryPoint; }
        StringView GetDebugName() const { return mMetaData.debugName; }

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

        struct StoredPreprocessorDefine
        {
            AnsiString name;
            AnsiString value;
        };

        struct MetaData
        {
            gapi::ShaderType type;
            gapi::ShaderLanguage language;

            Vector<ShaderFileInfo> fileInfos;
            Vector<ShaderFileInfo> dependencyFileInfos;
            String materialShaderCode;
            AnsiString entryPoint;

            Vector<StoredPreprocessorDefine> defines;

            String debugName;
        };
        MetaData mMetaData;

        SharedPtr<gapi::Shader> mRecompiledGAPIShader;
        Vector<ShaderFileInfo> mRecompiledShaderFileInfos;
        Vector<ShaderFileInfo> mRecompiledDependencyFileInfos;
        int mRecompileCount;
    };

    // ===== GraphicsPipeline =====

    // TODO: Separate info data and replace RecreateInfo
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

        StringView debugName;
    };

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(ShaderManager& manager, const GraphisPipelineCreateInfo& createInfo);
        ~GraphicsPipeline();

        SharedPtr<gapi::GraphicsPipeline> GetGAPIGraphicsPipeline() const { return mGAPIGraphicsPipeline; }
        const gapi::ShaderReflection& GetMergedShaderReflection() const { return mMergedShaderReflection; }

        bool HasRecompiledShadersInPipeline() const;

    private:
        friend class ShaderManager;

        void RecreateGraphicsPipeline();
        void MergeShaderReflection(const gapi::ShaderReflection& reflection);
        void CacheShaderReflection(SharedPtr<Shader> vertexShader, SharedPtr<Shader> pixelShader);

        ShaderManager& mManager;

        SharedPtr<gapi::GraphicsPipeline> mGAPIGraphicsPipeline;
        gapi::ShaderReflection mMergedShaderReflection;

        struct RecreateInfo
        {
            SharedPtr<Shader> vertexShader = nullptr;
            SharedPtr<Shader> pixelShader = nullptr;

            Vector<gapi::InputElement> inputLayouts;

            gapi::RasterizerState rasterizerState;

            Array<gapi::BlendState, gapi::MAX_NUM_RENDER_TARGETS> blendStates;

            gapi::DepthStencilState depthStencilState;

            gapi::PrimitiveTopologyType primitiveTopologyType = gapi::PrimitiveTopologyType::Triangle;

            Uint32 numRenderTargets;
            Array<gapi::ElementFormat, gapi::MAX_NUM_RENDER_TARGETS> renderTargetFormats;
            gapi::ElementFormat depthStencilFormat = gapi::ElementFormat::D32_Float;

            String debugName;

            void CopyFromCreateInfo(const GraphisPipelineCreateInfo& createInfo)
            {
                vertexShader = createInfo.vertexShader;
                pixelShader = createInfo.pixelShader;

                inputLayouts = Vector<gapi::InputElement>(createInfo.inputLayouts.begin(), createInfo.inputLayouts.end());

                rasterizerState = createInfo.rasterizerState;

                std::copy(createInfo.blendStates.begin(), createInfo.blendStates.end(), blendStates.begin());

                depthStencilState = createInfo.depthStencilState;

                primitiveTopologyType = createInfo.primitiveTopologyType;

                numRenderTargets = createInfo.numRenderTargets;
                std::copy(createInfo.renderTargetFormats.begin(), createInfo.renderTargetFormats.end(), renderTargetFormats.begin());
                depthStencilFormat = createInfo.depthStencilFormat;

                debugName = createInfo.debugName;
            }
        };
        RecreateInfo mRecreateInfo;
        int mRecreateCount;
    };

    // ===== ComputePipeline =====

    struct ComputePipelineCreateInfo
    {
        SharedPtr<Shader> shader;

        StringView debugName;
    };

    class ComputePipeline
    {
    public:
        ComputePipeline(ShaderManager& manager, const ComputePipelineCreateInfo& createInfo);
        ~ComputePipeline();

        SharedPtr<gapi::ComputePipeline> GetGAPIComputePipeline() const { return mGAPIComputePipeline; }
        const gapi::ShaderReflection& GetShaderReflection() const { return mShaderReflection; }

        bool HasRecompiledShaderInPipeline() const;

    private:
        friend class ShaderManager;

        void RecreateComputePipeline();
        void CacheShaderReflection(SharedPtr<Shader> shader);

        ShaderManager& mManager;

        SharedPtr<gapi::ComputePipeline> mGAPIComputePipeline;
        gapi::ShaderReflection mShaderReflection;

        struct RecreateInfo
        {
            SharedPtr<Shader> shader;

            StringView debugName;

            void CopyFromCreateInfo(const ComputePipelineCreateInfo& createInfo)
            {
                shader = createInfo.shader;

                debugName = createInfo.debugName;
            }
        };
        RecreateInfo mRecreateInfo;
        int mRecreateCount;
    };
} // namespace cube
