#include "Material.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_CommandList.h"
#include "GAPI_Shader.h"
#include "GAPI_Texture.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"

namespace cube
{
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

    void Material::SetTexture(int slotIndex, SharedPtr<TextureResource> texture)
    {
        CHECK_FORMAT(0 <= slotIndex && slotIndex < 5, "Texture slot out of range! ({0})", slotIndex);

        mTextures[slotIndex] = texture;
    }

    void Material::SetSampler(Uint64 samplerId)
    {
        mSamplerId = samplerId;
    }

    SharedPtr<MaterialShaderParameters> Material::GenerateShaderParameters(gapi::CommandList* commandList) const
    {
        ShaderParametersManager& shaderParametersManager = Engine::GetRenderer()->GetShaderParametersManager();

        SharedPtr<MaterialShaderParameters> parameters = shaderParametersManager.CreateShaderParameters<MaterialShaderParameters>();

        parameters->baseColor = mConstantBaseColor;
        if (mTextures[0])
        {
            parameters->textureSlot0.textureId = mTextures[0]->GetDefaultSRV()->GetBindlessId();
            parameters->textureSlot0.samplerId = mSamplerId;
            commandList->UseResource(mTextures[0]->GetDefaultSRV());
        }
        if (mTextures[1])
        {
            parameters->textureSlot1.textureId = mTextures[1]->GetDefaultSRV()->GetBindlessId();
            parameters->textureSlot1.samplerId = mSamplerId;
            commandList->UseResource(mTextures[1]->GetDefaultSRV());
        }
        if (mTextures[2])
        {
            parameters->textureSlot2.textureId = mTextures[2]->GetDefaultSRV()->GetBindlessId();
            parameters->textureSlot2.samplerId = mSamplerId;
            commandList->UseResource(mTextures[2]->GetDefaultSRV());
        }
        if (mTextures[3])
        {
            parameters->textureSlot3.textureId = mTextures[3]->GetDefaultSRV()->GetBindlessId();
            parameters->textureSlot3.samplerId = mSamplerId;
            commandList->UseResource(mTextures[3]->GetDefaultSRV());
        }
        if (mTextures[4])
        {
            parameters->textureSlot4.textureId = mTextures[4]->GetDefaultSRV()->GetBindlessId();
            parameters->textureSlot4.samplerId = mSamplerId;
            commandList->UseResource(mTextures[4]->GetDefaultSRV());
        }

        parameters->WriteAllParametersToBuffer();

        return parameters;
    }

    void Material::CalculateMaterialHash()
    {
        mMaterialHash = std::hash<String>{}(mChannelMappingCode);
    }

    MaterialShaderManager::MaterialShaderManager(ShaderManager& shaderManager)
        : mShaderManager(shaderManager)
    {
    }

    void MaterialShaderManager::ClearPipelineCache()
    {
        mMaterialPipelines.clear();
    }

    SharedPtr<GraphicsPipeline> MaterialShaderManager::GetOrCreateMaterialPipeline(SharedPtr<Material> material, const MeshMetadata& meshMeta)
    {
        const Uint64 materialHash = material->mMaterialHash;
         
        if (const auto it = mMaterialPipelines.find(materialHash); it != mMaterialPipelines.end())
        {
            return it->second;
        }

        // Generate material shader codes
        FrameString getMaterialShaderCode = Format<FrameString>(
            CUBE_T("MaterialValue GetMaterialValue(MaterialData_CB materialData, PSInput input)\n")
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
        SharedPtr<Shader>& vertexShader = mMaterialVertexShaders[materialHash];
        {
            String vertexShaderFilePath = String(Engine::GetShaderDirectoryPath()) + CUBE_T("/Main.slang");
            vertexShader = mShaderManager.CreateShader({
                .type = gapi::ShaderType::Vertex,
                .language = gapi::ShaderLanguage::Slang,
                .filePaths = { &vertexShaderFilePath, 1 },
                .entryPoint = "VSMain",
                .debugName = Format<FrameString>(CUBE_T("MaterialVS ({0})"), materialHash)
            });
        }
        SharedPtr<Shader>& pixelShader = mMaterialPixelShaders[materialHash];
        {
            // Currently dynamic linkage are used in pixel shader only
            String pixelShaderFilePath = String(Engine::GetShaderDirectoryPath()) + CUBE_T("/Main.slang");

            pixelShader = mShaderManager.CreateShader({
                .type = gapi::ShaderType::Pixel,
                .language = gapi::ShaderLanguage::Slang,
                .filePaths = { &pixelShaderFilePath, 1 },
                .materialShaderCode = materialShaderCode,
                .entryPoint = "PSMain",
                .debugName = Format<FrameString>(CUBE_T("MaterialPS ({0})"), materialHash)
            });
        }

        SharedPtr<GraphicsPipeline>& pipeline = mMaterialPipelines[materialHash];
        {
            pipeline = mShaderManager.CreateGraphicsPipeline({
                .vertexShader = vertexShader,
                .pixelShader = pixelShader,
                .inputLayouts = Mesh::GetInputElements(meshMeta),
                .depthStencilState = {
                    .enableDepth = true,
                    .depthFunction = gapi::CompareFunction::Greater
                },
                .numRenderTargets = 1,
                .renderTargetFormats = { gapi::ElementFormat::RGBA8_UNorm },
                .debugName = Format<FrameString>(CUBE_T("MaterialPipeline ({0})"), materialHash)
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
