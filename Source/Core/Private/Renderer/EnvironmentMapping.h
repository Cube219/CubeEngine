#pragma once

#include "CoreHeader.h"

#include "Renderer/RenderGraphTypes.h"
#include "Shader.h"

namespace cube
{
    class ComputePipeline;
    class GraphicsPipeline;
    class Renderer;
    class RGBuilder;
    class TextureResource;

    namespace gapi
    {
        class CommandList;
    } // namespace gapi

    class EnvironmentMapLightShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(EnvironmentMapLightShaderParameterList)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, diffuseIrradianceMap)
        CUBE_END_SHADER_PARAMETER_LIST
    };

    class EnvironmentMapping
    {
    public:
        EnvironmentMapping(Renderer& renderer);
        ~EnvironmentMapping() = default;

        void Initialize(bool enable);
        void Shutdown();

        void OnLoopImGUI();

        void LoadResources();
        void ClearReaources();

        bool IsSupported() const { return mIBLTexture != nullptr; }
        bool IsEnabled() const { return mIsEnabled; }

        void SetEnable(bool newEnable);

        void DrawSkybox(RGBuilder& builder);

        // Return black cube texture if disabled.
        RGTextureSRVHandle GetDiffuseIrradianceMap(RGBuilder& builder) const;

    private:
        void GenerateIrradianceMap();

        Renderer& mRenderer;

        bool mIsEnabled;

        enum class SkyboxType
        {
            None,
            IBL,
            DiffuseIrradianceMap,

            Num
        };
        static constexpr Array<const char*, 3> SkyboxTypeToString = {
            "None",
            "IBL",
            "DiffuseIrradianceMap",
        };
        SkyboxType mCurrentSkyboxType;

        SharedPtr<TextureResource> mIBLTexture;
        SharedPtr<Shader> mSkyboxVS;
        SharedPtr<Shader> mSkyboxPS;
        GraphicsPipelineInfo mSkyboxPipelineInfo;

        SharedPtr<Shader> mGenerateIrradianceMapShader;
        ComputePipelineInfo mGenerateIrradianceMapPipelineInfo;
        SharedPtr<gapi::CommandList> mCommandList;

        SharedPtr<gapi::Texture> mDiffuseIrradianceMap;
    };
} // namespace cube
