#include "Material.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_Buffer.h"
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
        : mUseConstantBaseColor(false)
        , mConstantBaseColor(1.0f, 1.0f, 1.0f, 1.0f)
        , mImplType(MaterialImplType::None)
    {
        FrameString bufferDebugName = Format<FrameString>(CUBE_T("[{0}] MaterialBuffer"), debugName);

        mTextures.fill(nullptr);

        mSamplerIndex = Engine::GetRenderer()->GetSamplerManager().GetDefaultLinearSamplerIndex();
    }

    Material::~Material()
    {
    }

    void Material::SetImplType(MaterialImplType implType)
    {
        mImplType = implType;
    }

    void Material::SetConstantBaseColor(bool use, Vector4 color)
    {
        mUseConstantBaseColor = use;
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

        parameters->useConstantBaseColor = mUseConstantBaseColor ? 1 : 0;
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
        parameters->sampler.index = mSamplerIndex;

        parameters->WriteAllParametersToBuffer();

        return parameters;
    }

    MaterialShaderManager::MaterialShaderManager(ShaderManager& shaderManager)
        : mShaderManager(shaderManager)
    {
    }

    SharedPtr<GraphicsPipeline> MaterialShaderManager::GetOrCreateMaterialPipeline(SharedPtr<Material> material)
    {
        MaterialImplType type = material->mImplType;
        int typeIndex = static_cast<int>(type);
        StringView typeStr;
        switch (type)
        {
        case MaterialImplType::None:
            typeStr = CUBE_T("None");
            break;
        case MaterialImplType::PBR_glTF:
            typeStr = CUBE_T("PBR_glTF");
            break;
        default:
            typeStr = CUBE_T("Unknown");
        }
         
        if (mMaterialPipelinesPerTypes[typeIndex] != nullptr)
        {
            return mMaterialPipelinesPerTypes[typeIndex];
        }

        // Create shaders
        FrameVector<String> additionalShaderPaths;
        switch (type)
        {
        case MaterialImplType::None:
            additionalShaderPaths.push_back(String(Engine::GetRootDirectoryPath()) + CUBE_T("/Resources/Shaders/Material/Material_none.slang"));
            break;
        case MaterialImplType::PBR_glTF:
            additionalShaderPaths.push_back(String(Engine::GetRootDirectoryPath()) + CUBE_T("/Resources/Shaders/Material/Material_glTF.slang"));
            break;
        default: ;
        }

        SharedPtr<Shader>& vertexShader = mMaterialVertexShadersPerTypes[typeIndex];
        {
            String vertexShaderFilePath = String(Engine::GetRootDirectoryPath()) + CUBE_T("/Resources/Shaders/Main.slang");
            vertexShader = mShaderManager.CreateShader({
                .type = gapi::ShaderType::Vertex,
                .language = gapi::ShaderLanguage::Slang,
                .filePaths = { &vertexShaderFilePath, 1 },
                .entryPoint = "VSMain",
                .debugName = Format<FrameString>(CUBE_T("MaterialVS ({0})"), typeStr)
            });
        }
        SharedPtr<Shader>& pixelShader = mMaterialPixelShadersPerTypes[typeIndex];
        {
            // Currently dynamic linkage are used in pixel shader only
            FrameVector<String> pixelShaderFilePaths = additionalShaderPaths;
            pixelShaderFilePaths.push_back(String(Engine::GetRootDirectoryPath()) + CUBE_T("/Resources/Shaders/Main.slang"));

            pixelShader = mShaderManager.CreateShader({
                .type = gapi::ShaderType::Pixel,
                .language = gapi::ShaderLanguage::Slang,
                .filePaths = pixelShaderFilePaths,
                .entryPoint = "PSMain",
                .debugName = Format<FrameString>(CUBE_T("MaterialPS ({0})"), typeStr)
            });
        }

        SharedPtr<GraphicsPipeline>& pipeline = mMaterialPipelinesPerTypes[typeIndex];
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
                .debugName = Format<FrameString>(CUBE_T("MaterialPipeline ({0})"), typeStr)
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

        mMaterialVertexShadersPerTypes.fill(nullptr);
        mMaterialPixelShadersPerTypes.fill(nullptr);
        mMaterialPipelinesPerTypes.fill(nullptr);
    }

    void MaterialShaderManager::Shutdown()
    {
        mMaterialPipelinesPerTypes.fill(nullptr);
        mMaterialPixelShadersPerTypes.fill(nullptr);
        mMaterialVertexShadersPerTypes.fill(nullptr);

        mShaderVariablesLayout = nullptr;
    }
} // namespace cube
