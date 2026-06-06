#pragma once

#include "CoreHeader.h"

#include "GAPI_Pipeline.h"
#include "Renderer/RenderGraphTypes.h"
#include "Renderer/RenderTypes.h"
#include "Renderer/ShaderParameter.h"
#include "Vector.h"

namespace cube
{
    class PipelineManager;
    class GraphicsPipeline;
    class Renderer;
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

    class MaterialShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(MaterialShaderParameterList)
            CUBE_SHADER_PARAMETER(Vector4, baseColor)
            CUBE_SHADER_PARAMETER(Vector4, diffuseColor)
            CUBE_SHADER_PARAMETER(Vector4, specularColor)
            CUBE_SHADER_PARAMETER(float, shininess)

            CUBE_SHADER_PARAMETER(Uint32, materialMode)
            CUBE_SHADER_PARAMETER(float, alphaCutoff)

            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, textureSlot0)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, textureSlot1)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, textureSlot2)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, textureSlot3)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, textureSlot4)
        CUBE_END_SHADER_PARAMETER_LIST
    };

    // Must match in Material.slang.
    enum class MaterialMode
    {
        Opaque,
        Mask
    };

    class Material
    {
    public:
        Material(StringView debugName);
        ~Material();

        void SetChannelMappingCode(StringView channelMappingCode);
        void AddAdditionalModule(StringView moduleName);

        void SetBaseColor(Vector4 color);
        void SetDiffuseColor(Vector4 color);
        void SetSpecularColor(Vector4 color);
        void SetShininess(float shininess);
        void SetIsPBR(bool isPBR);

        void SetMode(MaterialMode mode);
        void SetAlphaCutoff(float alphaCutoff);

        void SetTexture(int slotIndex, SharedPtr<TextureResource> texture);

        RGShaderParameterListHandle<MaterialShaderParameterList> GenerateShaderParameterList(RGBuilder& builder) const;

        StringView GetDebugName() const { return mDebugName; }

    private:
        friend class MaterialShaderManager;

        Uint64 GetMaterialHash();

        bool mIsMaterialHashDirty = true;
        Uint64 mMaterialHash;
        Set<String> mAdditionalModules;
        String mChannelMappingCode;

        bool mIsPBR = true;
        Vector4 mConstantBaseColor;
        Vector4 mConstantDiffuseColor;
        Vector4 mConstantSpecularColor;
        float mConstantShininess = 0.0f;

        MaterialMode mMode = MaterialMode::Opaque;
        float mAlphaCutoff = 0.0f;

        Array<SharedPtr<TextureResource>, 5> mTextures;

        String mDebugName;
    };

    struct MaterialPipelineStateInfo
    {
        gapi::RasterizerState rasterizerState;
        gapi::DepthStencilState depthStencilState;
        Uint32 numRenderTargets = 0;
        Array<gapi::ElementFormat, gapi::MAX_NUM_RENDER_TARGETS> renderTargetFormats;
        gapi::ElementFormat depthStencilFormat = gapi::ElementFormat::Unknown;
    };

    class MaterialShaderManager
    {
    public:
        MaterialShaderManager(Renderer& renderer, ShaderManager& shaderManager, PipelineManager& pipelineManager);
        ~MaterialShaderManager() = default;

        SharedPtr<GraphicsPipeline> GetOrCreateMaterialPipeline(
            SharedPtr<Material> material,
            const MaterialPipelineStateInfo& stateInfo
        );

        void ClearMaterialShaderCaches();

    private:
        friend class ShaderManager;

        // Initialize / Shutdown will be called in ShaderManager
        void Initialize();
        void Shutdown();

        Renderer& mRenderer;
        ShaderManager& mShaderManager;
        PipelineManager& mPipelineManager;

        HashMap<Uint64, SharedPtr<Shader>> mMaterialVertexShaders;
        HashMap<Uint64, SharedPtr<Shader>> mMaterialPixelShaders;
    };
} // namespace cube
