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
            CUBE_SHADER_PARAMETER(int, useConstantBaseColor)
            CUBE_SHADER_PARAMETER(Vector4, baseColor)

            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot0)
            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot1)
            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot2)
            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot3)
            CUBE_SHADER_PARAMETER(BindlessResource, textureSlot4)
            CUBE_SHADER_PARAMETER(BindlessResource, sampler)
        CUBE_END_SHADER_PARAMETERS
    };

    enum class MaterialImplType
    {
        None,
        PBR_glTF,

        Num
    };

    class Material
    {
    public:
        Material(StringView debugName);
        ~Material();

        void SetImplType(MaterialImplType implType);

        void SetConstantBaseColor(bool use, Vector4 color);

        void SetTexture(int slotIndex, SharedPtr<TextureResource> texture);

        void SetSampler(int samplerIndex);

        SharedPtr<MaterialShaderParameters> GenerateShaderParameters() const;

    private:
        friend class MaterialShaderManager;

        bool mUseConstantBaseColor;
        Vector4 mConstantBaseColor;
        Array<SharedPtr<TextureResource>, 5> mTextures;
        int mSamplerIndex;

        MaterialImplType mImplType;
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

        Array<SharedPtr<Shader>, (int)MaterialImplType::Num> mMaterialVertexShadersPerTypes;
        Array<SharedPtr<Shader>, (int)MaterialImplType::Num> mMaterialPixelShadersPerTypes;
        Array<SharedPtr<GraphicsPipeline>, (int)MaterialImplType::Num> mMaterialPipelinesPerTypes;

    };
} // namespace cube
