#include "Material.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_CommandList.h"
#include "GAPI_Shader.h"
#include "GAPI_Texture.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "RenderGraph.h"
#include "Shader.h"
#include "Texture.h"

namespace cube
{
    CUBE_REGISTER_SHADER_PARAMETER_LIST(MaterialShaderParameterList);

    Material::Material(StringView debugName)
        : mConstantBaseColor(1.0f, 0.0f, 0.80392f) // Magenta
        , mDebugName(std::move(debugName))
    {
        FrameString bufferDebugName = Format<FrameString>(CUBE_T("[{0}] MaterialBuffer"), mDebugName);

        mTextures.fill(nullptr);

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

    RGShaderParameterListHandle<MaterialShaderParameterList> Material::GenerateShaderParameterList(RGBuilder& builder) const
    {
        RGShaderParameterListHandle<MaterialShaderParameterList> parameters = builder.CreateShaderParameterList<MaterialShaderParameterList>();

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
            parameters->Get()->textureSlot0 = builder.GetDummyBlackTexture2D();
        }
        if (mTextures[1])
        {
            RGTextureHandle texture = builder.RegisterTexture(mTextures[1]->GetGAPITexture());
            RGTextureSRVHandle srv = builder.CreateSRV(texture);
            parameters->Get()->textureSlot1 = srv;
        }
        else
        {
            parameters->Get()->textureSlot1 = builder.GetDummyBlackTexture2D();
        }
        if (mTextures[2])
        {
            RGTextureHandle texture = builder.RegisterTexture(mTextures[2]->GetGAPITexture());
            RGTextureSRVHandle srv = builder.CreateSRV(texture);
            parameters->Get()->textureSlot2 = srv;
        }
        else
        {
            parameters->Get()->textureSlot2 = builder.GetDummyBlackTexture2D();
        }
        if (mTextures[3])
        {
            RGTextureHandle texture = builder.RegisterTexture(mTextures[3]->GetGAPITexture());
            RGTextureSRVHandle srv = builder.CreateSRV(texture);
            parameters->Get()->textureSlot3 = srv;
        }
        else
        {
            parameters->Get()->textureSlot3 = builder.GetDummyBlackTexture2D();
        }
        if (mTextures[4])
        {
            RGTextureHandle texture = builder.RegisterTexture(mTextures[4]->GetGAPITexture());
            RGTextureSRVHandle srv = builder.CreateSRV(texture);
            parameters->Get()->textureSlot4 = srv;
        }
        else
        {
            parameters->Get()->textureSlot4 = builder.GetDummyBlackTexture2D();
        }

        return parameters;
    }

    void Material::CalculateMaterialHash()
    {
        mMaterialHash = std::hash<String>{}(mChannelMappingCode);
        // Mix in isPBR flag so PBR and non-PBR materials get separate shader/pipelines.
        if (!mIsPBR)
        {
            mMaterialHash = HashCombine(mMaterialHash, 1);
        }
    }

    MaterialShaderManager::MaterialShaderManager(ShaderManager& shaderManager, PipelineManager& pipelineManager)
        : mShaderManager(shaderManager)
        , mPipelineManager(pipelineManager)
    {
    }

    SharedPtr<GraphicsPipeline> MaterialShaderManager::GetOrCreateMaterialPipeline(SharedPtr<Material> material, const MeshMetadata& meshMeta, gapi::RasterizerState::FillMode fillMode)
    {
        const Uint64 shaderHash = material->mMaterialHash;

        // Generate material shader codes
        FrameString getMaterialShaderCode = Format<FrameString>(
            CUBE_T("MaterialValue GetMaterialValue(MaterialShaderParameterList materialData, PSInput input)\n")
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
            CUBE_T("import StaticSampler;\n")
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
        if (!vertexShader)
        {
            platform::FilePath vertexShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("Main.slang");
            vertexShader = mShaderManager.CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Vertex,
                    .language = gapi::ShaderLanguage::Slang,
                    .entryPoint = "VSMain",
                    .defines = { shaderDefines.begin(), shaderDefines.end() }
                },
                .filePaths = { &vertexShaderFilePath, 1 },
                .debugName = Format<FrameString>(CUBE_T("MaterialVS ({0})"), material->GetDebugName())
            });
        }
        SharedPtr<Shader>& pixelShader = mMaterialPixelShaders[shaderHash];
        if (!pixelShader)
        {
            // Currently dynamic linkage is used in pixel shader only
            platform::FilePath pixelShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("Main.slang");

            pixelShader = mShaderManager.CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Pixel,
                    .language = gapi::ShaderLanguage::Slang,
                    .entryPoint = "PSMain",
                    .defines = { shaderDefines.begin(), shaderDefines.end() }
                },
                .filePaths = { &pixelShaderFilePath, 1 },
                .materialShaderCode = materialShaderCode,
                .debugName = Format<FrameString>(CUBE_T("MaterialPS ({0})"), material->GetDebugName())
            });
        }

        ArrayView<gapi::InputElement> inputLayouts = Mesh::GetInputElements(meshMeta);
        GraphicsPipelineInfo pipelineInfo = {
            .vertexShader = vertexShader,
            .pixelShader = pixelShader,
            .inputLayouts = { inputLayouts.begin(), inputLayouts.end() },
            .rasterizerState = {
                .fillMode = fillMode
            },
            .depthStencilState = {
                .enableDepth = true,
                .depthFunction = gapi::CompareFunction::Greater
            },
            .numRenderTargets = 1,
            .renderTargetFormats = {
                gapi::ElementFormat::RGBA8_UNorm
            }
        };
        pipelineInfo.CalculateHashValue();
        return mPipelineManager.GetOrCreateGraphicsPipeline({
            .pipelineInfo = pipelineInfo,
            .debugName = Format<FrameString>(CUBE_T("MaterialPipeline ({0})"), material->GetDebugName())
        });
    }

    void MaterialShaderManager::ClearMaterialShaderCaches()
    {
        mMaterialPixelShaders.clear();
        mMaterialVertexShaders.clear();
    }

    void MaterialShaderManager::Initialize(GAPI* gapi)
    {
    }

    void MaterialShaderManager::Shutdown()
    {
        ClearMaterialShaderCaches();
    }
} // namespace cube
