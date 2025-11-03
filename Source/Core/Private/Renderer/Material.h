#pragma once

#include "CoreHeader.h"

#include "Renderer/RenderTypes.h"
#include "ShaderParameter.h"
#include "Vector.h"

namespace cube
{
    class GraphicsPipeline;
    class Shader;
    class ShaderManager;
    class TextureResource;

    namespace gapi
    {
        class Sampler;
        class ShaderVariablesLayout;
        class Buffer;
        class Texture;
    } // namespace gapi

    class MaterialShaderParameters : public ShaderParameters
    {
        CUBE_BEGIN_SHADER_PARAMETERS(MaterialShaderParameters)
            CUBE_SHADER_PARAMETER(Vector4, baseColor)

            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot0)
            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot1)
            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot2)
            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot3)
            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot4)
        CUBE_END_SHADER_PARAMETERS
    };

    class Material
    {
    public:
        Material(StringView debugName);
        ~Material();

        void SetChannelMappingCode(StringView channelMappingCode);

        void SetBaseColor(Vector4 color);

        void SetTexture(int slotIndex, SharedPtr<TextureResource> texture);

        void SetSampler(int samplerIndex);

        SharedPtr<MaterialShaderParameters> GenerateShaderParameters() const;

        StringView GetDebugName() const { return mDebugName; }

    private:
        friend class MaterialShaderManager;

        void CalculateMaterialHash();

        Uint64 mMaterialHash;
        String mChannelMappingCode;

        Vector4 mConstantBaseColor;
        Array<SharedPtr<TextureResource>, 5> mTextures;
        int mSamplerIndex;

        String mDebugName;
    };

    class MaterialShaderManager
    {
    public:
        MaterialShaderManager(ShaderManager& shaderManager);
        ~MaterialShaderManager() = default;

        SharedPtr<GraphicsPipeline> GetOrCreateMaterialPipeline(SharedPtr<Material> material);

        SharedPtr<gapi::ShaderVariablesLayout> GetShaderVariablesLayout() const { return mShaderVariablesLayout; }

    private:
        friend class ShaderManager;

        // Initialize / Shutdown in ShaderManager
        void Initialize(GAPI* gapi);
        void Shutdown();

        ShaderManager& mShaderManager;

        SharedPtr<gapi::ShaderVariablesLayout> mShaderVariablesLayout;

        HashMap<Uint64, SharedPtr<Shader>> mMaterialVertexShaders;
        HashMap<Uint64, SharedPtr<Shader>> mMaterialPixelShaders;
        HashMap<Uint64, SharedPtr<GraphicsPipeline>> mMaterialPipelines;

    };
} // namespace cube
