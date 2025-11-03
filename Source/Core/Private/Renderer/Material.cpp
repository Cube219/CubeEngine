#include "Material.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_Shader.h"
#include "GAPI_ShaderVariable.h"
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

        mSamplerIndex = Engine::GetRenderer()->GetSamplerManager().GetDefaultLinearSamplerIndex();

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

    void Material::SetSampler(int samplerIndex)
    {
        mSamplerIndex = samplerIndex;
    }

    SharedPtr<MaterialShaderParameters> Material::GenerateShaderParameters() const
    {
        ShaderParametersManager& shaderParametersManager = Engine::GetRenderer()->GetShaderParametersManager();

        SharedPtr<MaterialShaderParameters> parameters = shaderParametersManager.CreateShaderParameters<MaterialShaderParameters>();

        parameters->baseColor = mConstantBaseColor;
        if (mTextures[0])
        {
            parameters->textureSlot0.index = mTextures[0]->GetDefaultSRV()->GetBindlessIndex();
            parameters->textureSlot0.samplerIndex = mSamplerIndex;
        }
        if (mTextures[1])
        {
            parameters->textureSlot1.index = mTextures[1]->GetDefaultSRV()->GetBindlessIndex();
            parameters->textureSlot1.samplerIndex = mSamplerIndex;
        }
        if (mTextures[2])
        {
            parameters->textureSlot2.index = mTextures[2]->GetDefaultSRV()->GetBindlessIndex();
            parameters->textureSlot2.samplerIndex = mSamplerIndex;
        }
        if (mTextures[3])
        {
            parameters->textureSlot3.index = mTextures[3]->GetDefaultSRV()->GetBindlessIndex();
            parameters->textureSlot3.samplerIndex = mSamplerIndex;
        }
        if (mTextures[4])
        {
            parameters->textureSlot4.index = mTextures[4]->GetDefaultSRV()->GetBindlessIndex();
            parameters->textureSlot4.samplerIndex = mSamplerIndex;
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

    SharedPtr<GraphicsPipeline> MaterialShaderManager::GetOrCreateMaterialPipeline(SharedPtr<Material> material)
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
                .inputLayouts = Mesh::GetInputElements(),
                .depthStencilState = {
                    .enableDepth = true,
                    .depthFunction = gapi::CompareFunction::Greater
                },
                .numRenderTargets = 1,
                .renderTargetFormats = { gapi::ElementFormat::RGBA8_UNorm },
                .shaderVariablesLayout = mShaderVariablesLayout,
                .debugName = Format<FrameString>(CUBE_T("MaterialPipeline ({0})"), materialHash)
            });
        }

        return pipeline;
    }

    void MaterialShaderManager::Initialize(GAPI* gapi)
    {
        mShaderVariablesLayout = gapi->CreateShaderVariablesLayout({
            .numShaderVariablesConstantBuffer = 3,
            .shaderVariablesConstantBuffer = nullptr,
            .debugName = CUBE_T("MaterialShaderVariablesLayout")
        });
    }

    void MaterialShaderManager::Shutdown()
    {
        mMaterialPipelines.clear();
        mMaterialPixelShaders.clear();
        mMaterialVertexShaders.clear();

        mShaderVariablesLayout = nullptr;
    }
} // namespace cube
