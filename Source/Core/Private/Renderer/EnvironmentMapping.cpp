#include "EnvironmentMapping.h"

#include "imgui.h"

#include "GAPI_Shader.h"
#include "RenderGraph.h"
#include "Shader.h"

namespace cube
{
    class GenerateIrradianceMapShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(GenerateIrradianceMapShaderParameterList)
            CUBE_SHADER_PARAMETER(int, numSlices)
            CUBE_SHADER_PARAMETER(Vector2, widthAndInvWidth)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, srcIBL)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstDiffuseIrradianceMap)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(GenerateIrradianceMapShaderParameterList);

    class SkyboxShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(SkyboxShaderParameterList)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, skyboxTexture)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(SkyboxShaderParameterList);

    CUBE_REGISTER_SHADER_PARAMETER_LIST(EnvironmentMapLightShaderParameterList);

    EnvironmentMapping::EnvironmentMapping(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    void EnvironmentMapping::Initialize(bool enable)
    {
        mIsEnabled = enable;

        mCurrentSkyboxType = SkyboxType::IBL;

        {
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
            CHECK(mSkyboxVS);
            mSkyboxPS = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Pixel,
                    .language = gapi::ShaderLanguage::Slang,
                    .entryPoint = "PSMain"
                },
                .filePaths = { &skyboxShaderFilePath, 1 },
                .debugName = CUBE_T("SkyboxPS")
            });
            CHECK(mSkyboxPS);
        }

        {
            platform::FilePath environmentMappingShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("EnvironmentMapping.slang");

            mGenerateIrradianceMapShader = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Compute,
                    .entryPoint = "GenerateIrradianceMapCS"
                },
                .filePaths = { &environmentMappingShaderFilePath, 1 },
                .debugName = CUBE_T("GenerateIrradianceMapCS")
            });
            CHECK(mGenerateIrradianceMapShader);

            mGenerateIrradianceMapPipeline = mRenderer.GetShaderManager().CreateComputePipeline({
                .pipelineInfo = {
                    .shader = mGenerateIrradianceMapShader
                },
                .debugName = CUBE_T("GenerateIrradianceMap Pipeline")
            });
        }

        mCommandList = mRenderer.GetGAPI().CreateCommandList({
            .debugName = CUBE_T("EnvironmentMappingCommandList")
        });
    }

    void EnvironmentMapping::Shutdown()
    {
        mCommandList = nullptr;

        mGenerateIrradianceMapPipeline = nullptr;
        mGenerateIrradianceMapShader = nullptr;

        mSkyboxPipeline = nullptr;
        mSkyboxPS = nullptr;
        mSkyboxVS = nullptr;
    }

    void EnvironmentMapping::OnLoopImGUI()
    {
        ImGui::SeparatorText("Environment Mapping");
        {
            ImGui::BeginDisabled(!IsSupported());

            bool isEnabled = IsEnabled();
            if (ImGui::Checkbox("Enable", &isEnabled))
            {
                SetEnable(isEnabled);
            }

            ImGui::BeginDisabled(!isEnabled);

            {
                ImGui::Text("Skybox");

                for (int i = 0; i < static_cast<int>(SkyboxType::Num); ++i)
                {
                    if (i > 0)
                    {
                        ImGui::SameLine();
                    }

                    const char* name = SkyboxTypeToString[i];
                    if (ImGui::RadioButton(name, (i == static_cast<int>(mCurrentSkyboxType))))
                    {
                        mCurrentSkyboxType = static_cast<SkyboxType>(i);
                        
                    }
                }
            }
            ImGui::EndDisabled();

            ImGui::EndDisabled();
        }
    }

    void EnvironmentMapping::LoadResources()
    {
        platform::FilePath IBLPath = Engine::GetRootDirectoryPath() / CUBE_T("Resources/Textures/IBL/NissiBeach2");
        if (platform::FileSystem::IsExist(IBLPath))
        {
            TextureRawData negXData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("negx.jpg"), TextureHelper::LoadElementType::Float);
            TextureRawData negYData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("negy.jpg"), TextureHelper::LoadElementType::Float);
            TextureRawData negZData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("negz.jpg"), TextureHelper::LoadElementType::Float);
            TextureRawData posXData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("posx.jpg"), TextureHelper::LoadElementType::Float);
            TextureRawData posYData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("posy.jpg"), TextureHelper::LoadElementType::Float);
            TextureRawData posZData = TextureHelper::LoadFromFile(IBLPath / CUBE_T("posz.jpg"), TextureHelper::LoadElementType::Float);

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

            GenerateIrradianceMap();
        }
    }

    void EnvironmentMapping::ClearReaources()
    {
        mDiffuseIrradianceMap = nullptr;
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
        if (!IsSupported() || !mIsEnabled || mCurrentSkyboxType == SkyboxType::None)
        {
            return;
        }

        // Use IBL texture as default.
        SharedPtr<gapi::Texture> skyboxGAPITexture = mIBLTexture->GetGAPITexture();
        if (mCurrentSkyboxType == SkyboxType::DiffuseIrradianceMap)
        {
            skyboxGAPITexture = mDiffuseIrradianceMap;
        }

        RGTextureHandle skyboxTexture = builder.RegisterTexture(skyboxGAPITexture);
        RGTextureSRVHandle skyboxSRV = builder.CreateSRV(skyboxTexture);

        RGShaderParameterListHandle<SkyboxShaderParameterList> skyboxParams = builder.CreateShaderParameterList<SkyboxShaderParameterList>();
        skyboxParams->Get()->skyboxTexture = skyboxSRV;
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
            return builder.GetDummyBlackTextureCube();
        }
    }

    void EnvironmentMapping::GenerateIrradianceMap()
    {
        CHECK(mIBLTexture);

        const gapi::ElementFormat format = gapi::ElementFormat::RGBA16_Float;
        const Uint32 width = 256;
        const Uint32 height = 256;

        mDiffuseIrradianceMap = mRenderer.GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = {
                .format = format,
                .type = gapi::TextureType::TextureCube,
                .flags = gapi::TextureFlag::UAV,
                .width = width,
                .height = height,
            },
            .debugName = CUBE_T("Irradiance Map")
        });

        RGBuilder builder(mRenderer);
        {
            RG_GPU_EVENT_SCOPE(builder, CUBE_T("GenerateIrradianceMap"));

            RGTextureHandle srcIBL = builder.RegisterTexture(mIBLTexture->GetGAPITexture());
            RGTextureSRVHandle srcIBLSRV = builder.CreateSRV(srcIBL);
            RGTextureHandle dstDiffuseEnvMap = builder.RegisterTexture(mDiffuseIrradianceMap);
            RGTextureUAVHandle dstDiffuseEnvMapUAV = builder.CreateUAV(dstDiffuseEnvMap);

            RGShaderParameterListHandle<GenerateIrradianceMapShaderParameterList> params = builder.CreateShaderParameterList<GenerateIrradianceMapShaderParameterList>();
            params->Get()->numSlices = 128;
            params->Get()->widthAndInvWidth = Vector2(static_cast<float>(width), 1.0f / static_cast<float>(width));
            params->Get()->srcIBL = srcIBLSRV;
            params->Get()->dstDiffuseIrradianceMap = dstDiffuseEnvMapUAV;
            params->Get()->WriteAllParametersToGPUBuffer();

            builder.AddPass(CUBE_T("GenerateIrradianceMap"),
                mGenerateIrradianceMapPipeline,
                params,
                [width, height](gapi::CommandList& commandList)
            {
                commandList.DispatchThreads(width, height, 6);
            });
        }
        builder.ExecuteAndSubmit(*mCommandList);
    }
} // namespace cube
