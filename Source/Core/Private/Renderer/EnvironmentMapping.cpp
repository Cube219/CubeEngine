#include "EnvironmentMapping.h"

#include "GAPI_Shader.h"
#include "RenderGraph.h"
#include "Shader.h"

namespace cube
{
    EnvironmentMapping::EnvironmentMapping(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    void EnvironmentMapping::Initialize(bool enable)
    {
        mIsEnabled = enable;
    }

    void EnvironmentMapping::Shutdown()
    {
    }

    void EnvironmentMapping::LoadResources()
    {
        platform::FilePath IBLPath = Engine::GetRootDirectoryPath() / CUBE_T("Resources/Textures/IBL/NissiBeach2");
        if (platform::FileSystem::IsExist(IBLPath))
        {
            TextureRawData negXData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("negx.jpg"));
            TextureRawData negYData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("negy.jpg"));
            TextureRawData negZData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("negz.jpg"));
            TextureRawData posXData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("posx.jpg"));
            TextureRawData posYData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("posy.jpg"));
            TextureRawData posZData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("posz.jpg"));

            const Uint64 totalSize = negXData.data.GetSize() + negYData.data.GetSize() + negZData.data.GetSize()
                + posXData.data.GetSize() + posYData.data.GetSize() + posZData.data.GetSize();
            Blob totalData = Blob(totalSize);
            {
                Byte* pData = (Byte*)totalData.GetData();
#define CUBE_APPEND_DATA(v) \
                memcpy(pData, (v).data.GetData(), (v).data.GetSize()); \
                pData += (v).data.GetSize();

                CUBE_APPEND_DATA(posXData);
                CUBE_APPEND_DATA(negXData);
                CUBE_APPEND_DATA(posYData);
                CUBE_APPEND_DATA(negYData);
                CUBE_APPEND_DATA(posZData);
                CUBE_APPEND_DATA(negZData);
#undef CUBE_APPEND_DATA
            }

            TextureResourceCreateInfo createInfo = {
                .textureInfo = {
                    .format = negXData.format,
                    .type = gapi::TextureType::TextureCube,
                    .width = negXData.width,
                    .height = negYData.height,
                },
                .data = BlobView(totalData),
                .bytesPerElement = negXData.bytesPerElement,
                .debugName = CUBE_T("IBLTexture")
            };
            mIBLTexture = std::make_shared<TextureResource>(createInfo);

            platform::FilePath skyboxShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("Skybox.slang");

            mSkyboxVS = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Vertex,
                    .language = gapi::ShaderLanguage::Slang,
                    .entryPoint = "VSMain"
                },
                .filePaths = { &skyboxShaderFilePath, 1 },
                .debugName = CUBE_T("SkyboxVS")
            });
            mSkyboxPS = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Pixel,
                    .language = gapi::ShaderLanguage::Slang,
                    .entryPoint = "PSMain"
                },
                .filePaths = { &skyboxShaderFilePath, 1 },
                .debugName = CUBE_T("SkyboxPS")
            });
        }
    }

    void EnvironmentMapping::ClearReaources()
    {
        mSkyboxPipeline = nullptr;
        mSkyboxPS = nullptr;
        mSkyboxVS = nullptr;
        mIBLTexture = nullptr;
    }

    void EnvironmentMapping::SetEnable(bool newEnable)
    {
        mIsEnabled = newEnable;
    }

    void EnvironmentMapping::RecreateGraphicsPipelines()
    {
        mSkyboxPipeline = mRenderer.GetShaderManager().CreateGraphicsPipeline({
            .pipelineInfo = {
                .vertexShader = mSkyboxVS,
                .pixelShader = mSkyboxPS,
                .inputLayouts = Mesh::GetInputElements(mRenderer.GetMeshMetadata()),
                .rasterizerState = {
                    .fillMode = mRenderer.IsDrawInWireframe() ? gapi::RasterizerState::FillMode::Line : gapi::RasterizerState::FillMode::Solid,
                    .cullMode = gapi::RasterizerState::CullMode::Front
                },
                .depthStencilState = {
                    .enableDepth = true,
                    .depthFunction = gapi::CompareFunction::GreaterEqual
                },
                .numRenderTargets = 1,
                .renderTargetFormats = { gapi::ElementFormat::RGBA8_UNorm }
            },
            .debugName = CUBE_T("SkyboxPipeline")
        });
    }

    void EnvironmentMapping::DrawSkybox(RGBuilder& builder)
    {
        if (!IsSupported() || !mIsEnabled)
        {
            return;
        }

        RGTextureHandle IBLTexture = builder.RegisterTexture(mIBLTexture->GetGAPITexture());
        RGTextureSRVHandle IBLSRV = builder.CreateSRV(IBLTexture);

        RGShaderParameterListHandle<SkyboxShaderParameterList> skyboxParams = builder.CreateShaderParameterList<SkyboxShaderParameterList>();
        skyboxParams->Get()->skyboxTexture = IBLSRV;
        skyboxParams->Get()->skyboxSampler.id = mRenderer.GetSamplerManager().GetDefaultLinearSamplerId();
        skyboxParams->Get()->WriteAllParametersToGPUBuffer();

        builder.AddPass(CUBE_T("Skybox"), mSkyboxPipeline, skyboxParams,
        [boxMesh = mRenderer.GetBoxMesh()](gapi::CommandList& commandList)
        {
            const SubMesh& boxSubMesh = boxMesh->GetSubMeshes()[0];
            Uint32 vbOffset = 0;
            SharedPtr<gapi::Buffer> vb = boxMesh->GetVertexBuffer();
            commandList.BindVertexBuffers(0, { &vb, 1 }, { &vbOffset, 1 });
            commandList.BindIndexBuffer(boxMesh->GetIndexBuffer(), 0);

            commandList.DrawIndexed(boxSubMesh.numIndices, 0, 0);
        });
    }

    RGTextureSRVHandle EnvironmentMapping::GetDiffuseIrradianceMap(RGBuilder& builder) const
    {
        if (mIsEnabled && mDiffuseIrradianceMap)
        {
            RGTextureHandle rgTexture = builder.RegisterTexture(mDiffuseIrradianceMap);
            return builder.CreateSRV(rgTexture);
        }
        else
        {
            return builder.GetDummyBlackTexture();
        }
    }
} // namespace cube
