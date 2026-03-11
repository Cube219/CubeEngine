#pragma once

#include "CoreHeader.h"

#include "GAPI_Pipeline.h"
#include "Renderer/RenderTypes.h"
#include "Renderer/ShaderParameter.h"
#include "Vector.h"

namespace cube
{
    class GraphicsPipeline;
    class Shader;
    class ShaderManager;
    class TextureResource;
    struct MeshMetadata;

    namespace gapi
    {
        class CommandList;
        class Sampler;
        class Buffer;
        class Texture;
    } // namespace gapi

    class MaterialShaderParameters : public ShaderParameters
    {
        CUBE_BEGIN_SHADER_PARAMETERS(MaterialShaderParameters)
            CUBE_SHADER_PARAMETER(Vector4, baseColor)
            CUBE_SHADER_PARAMETER(Vector4, diffuseColor)
            CUBE_SHADER_PARAMETER(Vector4, specularColor)
            CUBE_SHADER_PARAMETER(float, shininess)

            CUBE_SHADER_PARAMETER(BindlessCombinedTextureSampler, textureSlot0)
            CUBE_SHADER_PARAMETER(BindlessCombinedTextureSampler, textureSlot1)
            CUBE_SHADER_PARAMETER(BindlessCombinedTextureSampler, textureSlot2)
            CUBE_SHADER_PARAMETER(BindlessCombinedTextureSampler, textureSlot3)
            CUBE_SHADER_PARAMETER(BindlessCombinedTextureSampler, textureSlot4)
        CUBE_END_SHADER_PARAMETERS
    };

    class Material
    {
    public:
        Material(StringView debugName);
        ~Material();

        void SetChannelMappingCode(StringView channelMappingCode);

        void SetBaseColor(Vector4 color);
        void SetDiffuseColor(Vector4 color);
        void SetSpecularColor(Vector4 color);
        void SetShininess(float shininess);
        void SetIsPBR(bool isPBR);

        void SetTexture(int slotIndex, SharedPtr<TextureResource> texture);

        void SetSampler(Uint64 samplerId);

        SharedPtr<MaterialShaderParameters> GenerateShaderParameters(gapi::CommandList* commandList) const;

        StringView GetDebugName() const { return mDebugName; }

    private:
        friend class MaterialShaderManager;

        void CalculateMaterialHash();

        Uint64 mMaterialHash;
        String mChannelMappingCode;

        bool mIsPBR = true;
        Vector4 mConstantBaseColor;
        Vector4 mConstantDiffuseColor;
        Vector4 mConstantSpecularColor;
        float mConstantShininess = 0.0f;
        Array<SharedPtr<TextureResource>, 5> mTextures;
        Uint64 mSamplerId;

        String mDebugName;
    };

    class MaterialShaderManager
    {
    public:
        MaterialShaderManager(ShaderManager& shaderManager);
        ~MaterialShaderManager() = default;

        SharedPtr<GraphicsPipeline> GetOrCreateMaterialPipeline(SharedPtr<Material> material, const MeshMetadata& meshMeta, gapi::RasterizerState::FillMode fillMode = gapi::RasterizerState::FillMode::Solid);

        void ClearPipelineCache();

    private:
        friend class ShaderManager;

        // Initialize / Shutdown in ShaderManager
        void Initialize(GAPI* gapi);
        void Shutdown();

        ShaderManager& mShaderManager;

        HashMap<Uint64, SharedPtr<Shader>> mMaterialVertexShaders;
        HashMap<Uint64, SharedPtr<Shader>> mMaterialPixelShaders;
        HashMap<Uint64, SharedPtr<GraphicsPipeline>> mMaterialPipelines;

    };
} // namespace cube
