#include "Material.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_CommandList.h"
#include "GAPI_Shader.h"
#include "GAPI_Texture.h"
#include "Mesh.h"
#include "Renderer.h"
#include "RenderGraph.h"
#include "Shader.h"
#include "Texture.h"

namespace cube
{
    CUBE_REGISTER_SHADER_PARAMETERS(MaterialShaderParameters);

    Material::Material(StringView debugName)
        : mConstantBaseColor(1.0f, 0.0f, 0.80392f) // Magenta
        , mDebugName(std::move(debugName))
    {
        FrameString bufferDebugName = Format<FrameString>(CUBE_T("[{0}] MaterialBuffer"), mDebugName);

        mTextures.fill(nullptr);

        mSamplerId = Engine::GetRenderer()->GetSamplerManager().GetDefaultLinearSamplerId();

        static const Character* defaultChannelMappingCode = 
            CUBE_T("value.albedo = materialData.baseColor.rgb;\n")
        ;

        SetChannelMappingCode(defaultChannelMappingCode);
    }

    Material::~Material()
    {
    }

    void Material::SetChannelMappingCode(StringView channelMappingCode)
    {
        mChannelMappingCode = channelMappingCode;

        CalculateMaterialHash();
    }

    void Material::SetBaseColor(Vector4 color)
    {
        mConstantBaseColor = color;
    }

    void Material::SetDiffuseColor(Vector4 color)
    {
        mConstantDiffuseColor = color;
    }

    void Material::SetSpecularColor(Vector4 color)
    {
        mConstantSpecularColor = color;
    }

    void Material::SetShininess(float shininess)
    {
        mConstantShininess = shininess;
    }

    void Material::SetIsPBR(bool isPBR)
    {
        mIsPBR = isPBR;
        CalculateMaterialHash();
    }

    void Material::SetTexture(int slotIndex, SharedPtr<TextureResource> texture)
    {
        CHECK_FORMAT(0 <= slotIndex && slotIndex < 5, "Texture slot out of range! ({0})", slotIndex);

        mTextures[slotIndex] = texture;
    }

    void Material::SetSampler(Uint64 samplerId)
    {
        mSamplerId = samplerId;
    }

    RGShaderParametersHandle<MaterialShaderParameters> Material::GenerateShaderParameters(RGBuilder& builder) const
    {
        RGShaderParametersHandle<MaterialShaderParameters> parameters = builder.CreateShaderParameters<MaterialShaderParameters>();

        parameters->Get()->baseColor = mConstantBaseColor;
        parameters->Get()->diffuseColor = mConstantDiffuseColor;
        parameters->Get()->specularColor = mConstantSpecularColor;
        parameters->Get()->shininess = mConstantShininess;
        if (mTextures[0])
        {
            RGTextureHandle texture = builder.RegisterTexture(mTextures[0]->GetGAPITexture());
            RGTextureSRVHandle srv = builder.CreateSRV(texture);
            parameters->Get()->textureSlot0 = srv;
        }
        else
        {
            parameters->Get()->textureSlot0 = builder.GetDummyBlackTexture();
        }
        if (mTextures[1])
        {
            RGTextureHandle texture = builder.RegisterTexture(mTextures[1]->GetGAPITexture());
            RGTextureSRVHandle srv = builder.CreateSRV(texture);
            parameters->Get()->textureSlot1 = srv;
        }
        else
        {
            parameters->Get()->textureSlot1 = builder.GetDummyBlackTexture();
        }
        if (mTextures[2])
        {
            RGTextureHandle texture = builder.RegisterTexture(mTextures[2]->GetGAPITexture());
            RGTextureSRVHandle srv = builder.CreateSRV(texture);
            parameters->Get()->textureSlot2 = srv;
        }
        else
        {
            parameters->Get()->textureSlot2 = builder.GetDummyBlackTexture();
        }
        if (mTextures[3])
        {
            RGTextureHandle texture = builder.RegisterTexture(mTextures[3]->GetGAPITexture());
            RGTextureSRVHandle srv = builder.CreateSRV(texture);
            parameters->Get()->textureSlot3 = srv;
        }
        else
        {
            parameters->Get()->textureSlot3 = builder.GetDummyBlackTexture();
        }
        if (mTextures[4])
        {
            RGTextureHandle texture = builder.RegisterTexture(mTextures[4]->GetGAPITexture());
            RGTextureSRVHandle srv = builder.CreateSRV(texture);
            parameters->Get()->textureSlot4 = srv;
        }
        else
        {
            parameters->Get()->textureSlot4 = builder.GetDummyBlackTexture();
        }
        parameters->Get()->materialSampler.id = mSamplerId;

        parameters->Get()->WriteAllParametersToGPUBuffer();

        return parameters;
    }

    void Material::CalculateMaterialHash()
    {
        mMaterialHash = std::hash<String>{}(mChannelMappingCode);
        // Mix in isPBR flag so PBR and non-PBR materials get separate pipelines
        if (!mIsPBR)
        {
            mMaterialHash ^= 0x9e3779b97f4a7c15ULL;
        }
    }

    MaterialShaderManager::MaterialShaderManager(ShaderManager& shaderManager)
        : mShaderManager(shaderManager)
    {
    }

    void MaterialShaderManager::ClearPipelineCache()
    {
        mMaterialPipelines.clear();
    }

    SharedPtr<GraphicsPipeline> MaterialShaderManager::GetOrCreateMaterialPipeline(SharedPtr<Material> material, const MeshMetadata& meshMeta, gapi::RasterizerState::FillMode fillMode)
    {
        const Uint64 shaderHash = material->mMaterialHash;
        const Uint64 pipelineHash = shaderHash ^ (static_cast<Uint64>(fillMode) * 0x9e3779b97f4a7c15ULL);

        if (const auto it = mMaterialPipelines.find(pipelineHash); it != mMaterialPipelines.end())
        {
            return it->second;
        }

        // Generate material shader codes
        FrameString getMaterialShaderCode = Format<FrameString>(
            CUBE_T("MaterialValue GetMaterialValue(MaterialShaderParameters materialData, PSInput input)\n")
            CUBE_T("{{\n")
            CUBE_T("    MaterialValue value = {{}};\n")
            CUBE_T("\n")
            CUBE_T("    {0}")
            CUBE_T("\n")
            CUBE_T("    return value;\n")
            CUBE_T("}}\n"),
            material->mChannelMappingCode
        );

        FrameString materialShaderCode = Format<FrameString>(
            CUBE_T("import MainInterface;\n")
            CUBE_T("import Material;\n")
            CUBE_T("\n")
            CUBE_T("export struct Material : IMaterial\n")
            CUBE_T("{{\n")
            CUBE_T("\n")
            CUBE_T("{0}\n")
            CUBE_T("\n")
            CUBE_T("}}\n"),

            getMaterialShaderCode
        );

        // Create shaders
        gapi::PreprocessorDefine pbrDefine = { "MATERIAL_PBR", "1" };
        ArrayView<gapi::PreprocessorDefine> shaderDefines;
        if (material->mIsPBR)
        {
            shaderDefines = { &pbrDefine, 1 };
        }

        SharedPtr<Shader>& vertexShader = mMaterialVertexShaders[shaderHash];
        {
            platform::FilePath vertexShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("Main.slang");
            vertexShader = mShaderManager.CreateShader({
                .type = gapi::ShaderType::Vertex,
                .language = gapi::ShaderLanguage::Slang,
                .filePaths = { &vertexShaderFilePath, 1 },
                .entryPoint = "VSMain",
                .defines = shaderDefines,
                .debugName = Format<FrameString>(CUBE_T("MaterialVS ({0})"), shaderHash)
            });
        }
        SharedPtr<Shader>& pixelShader = mMaterialPixelShaders[shaderHash];
        if (!pixelShader)
        {
            // Currently dynamic linkage are used in pixel shader only
            platform::FilePath pixelShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("Main.slang");

            pixelShader = mShaderManager.CreateShader({
                .type = gapi::ShaderType::Pixel,
                .language = gapi::ShaderLanguage::Slang,
                .filePaths = { &pixelShaderFilePath, 1 },
                .materialShaderCode = materialShaderCode,
                .entryPoint = "PSMain",
                .defines = shaderDefines,
                .debugName = Format<FrameString>(CUBE_T("MaterialPS ({0})"), shaderHash)
            });
        }

        SharedPtr<GraphicsPipeline>& pipeline = mMaterialPipelines[pipelineHash];
        {
            pipeline = mShaderManager.CreateGraphicsPipeline({
                .vertexShader = vertexShader,
                .pixelShader = pixelShader,
                .inputLayouts = Mesh::GetInputElements(meshMeta),
                .rasterizerState = {
                    .fillMode = fillMode
                },
                .depthStencilState = {
                    .enableDepth = true,
                    .depthFunction = gapi::CompareFunction::Greater
                },
                .numRenderTargets = 1,
                .renderTargetFormats = { gapi::ElementFormat::RGBA8_UNorm },
                .debugName = Format<FrameString>(CUBE_T("MaterialPipeline ({0})"), pipelineHash)
            });
        }

        return pipeline;
    }

    void MaterialShaderManager::Initialize(GAPI* gapi)
    {
    }

    void MaterialShaderManager::Shutdown()
    {
        mMaterialPipelines.clear();
        mMaterialPixelShaders.clear();
        mMaterialVertexShaders.clear();
    }
} // namespace cube
