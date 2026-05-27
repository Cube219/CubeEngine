#pragma once

#include "CoreHeader.h"

#include "Renderer/RenderGraphTypes.h"
#include "Pipeline.h"

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
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, integratedBRDFLUT)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, prefilterMap)
            CUBE_SHADER_PARAMETER(BindlessSampler, prefilterSampler)
            CUBE_SHADER_PARAMETER(Uint32, prefilterMapMipLevels)
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
        void ClearResources();

        bool IsSupported() const { return mIBLTexture != nullptr; }
        bool IsEnabled() const { return mIsEnabled; }

        void SetEnable(bool newEnable);

        void DrawSkybox(RGBuilder& builder);

        // Return black cube texture if disabled.
        RGTextureSRVHandle GetDiffuseIrradianceMap(RGBuilder& builder) const;
        // Return black 2d texture if disabled.
        RGTextureSRVHandle GetIntegratedBRDFLUT(RGBuilder& builder) const;
        // Return black cube texture if disabled.
        RGTextureSRVHandle GetPrefilterMap(RGBuilder& builder) const;
        BindlessSampler GetPrefilterMapSampler() const;
        Uint32 GetPrefilterMapMipLevels() const;

    private:
        void LoadIBLTextureList();

        void ClearCurrentIBLTexture();
        void LoadCurrentIBLTexture();

        void GenerateIrradianceMap();
        void GenerateIntegratedBRDFLUT();
        void GeneratePrefilterMap();

        Renderer& mRenderer;

        bool mIsEnabled;

        enum class SkyboxType
        {
            None,
            IBL,
            DiffuseIrradianceMap,
            PrefilterMap,

            Num
        };
        SkyboxType mCurrentSkyboxType;
        Uint32 mCurrentSkyboxMipLevel = 0;

        bool mIBLDropdownExpandedLastFrame = false;
        Vector<AnsiString> mIBLTextureList;
        AnsiString mCurrentSelectedIBLTextureName;

        SharedPtr<TextureResource> mIBLTexture;
        SharedPtr<Shader> mSkyboxVS;
        SharedPtr<Shader> mSkyboxPS;
        GraphicsPipelineInfo mSkyboxPipelineInfo;

        SharedPtr<Shader> mGenerateIrradianceMapShader;
        ComputePipelineInfo mGenerateIrradianceMapPipelineInfo;
        SharedPtr<Shader> mGenerateIntegratedBRDFLUTShader;
        ComputePipelineInfo mGenerateIntegratedBRDFLUTPipelineInfo;
        SharedPtr<Shader> mGeneratePrefilterMapShader;
        ComputePipelineInfo mGeneratePrefilterMapPipelineInfo;
        SharedPtr<gapi::CommandList> mCommandList;

        SharedPtr<gapi::Texture> mDiffuseIrradianceMap;
        SharedPtr<gapi::Texture> mIntegratedBRDFLUT;
        SharedPtr<gapi::Texture> mPrefilterMap;
        BindlessSampler mPrefilterMapSampler;
    };
} // namespace cube
