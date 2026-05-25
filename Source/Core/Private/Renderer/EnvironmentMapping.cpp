#include "EnvironmentMapping.h"

#include "imgui.h"

#include "GAPI_Shader.h"
#include "Pipeline.h"
#include "Renderer/Mesh.h"
#include "RenderGraph.h"
#include "Shader.h"
#include "Allocator/FrameAllocator.h"

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

    class GenerateIntegratedBRDFLUTShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(GenerateIntegratedBRDFLUTShaderParameterList)
            CUBE_SHADER_PARAMETER(Uint32, sampleCount)
            CUBE_SHADER_PARAMETER(float, width)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstIntegratedBRDFLUT)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(GenerateIntegratedBRDFLUTShaderParameterList);

    class GeneratePrefilterMapShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(GeneratePrefilterMapShaderParameterList)
            CUBE_SHADER_PARAMETER(Uint32, numSamples)
            CUBE_SHADER_PARAMETER(Vector2, widthAndInvWidth)
            CUBE_SHADER_PARAMETER(float, roughness)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, srcIBL)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstPrefilterMap)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(GeneratePrefilterMapShaderParameterList);

    class SkyboxShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(SkyboxShaderParameterList)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, skyboxTexture)
            CUBE_SHADER_PARAMETER(RGBufferSRVHandle, vertexBuffer)
            CUBE_SHADER_PARAMETER(bool, useFP16)
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

            mSkyboxPipelineInfo = {
                .vertexShader = mSkyboxVS,
                .pixelShader = mSkyboxPS,
                .rasterizerState = {
                    .cullMode = gapi::RasterizerState::CullMode::Front
                },
                .depthStencilState = {
                    .enableDepth = true,
                    .depthFunction = gapi::CompareFunction::GreaterEqual
                },
                .numRenderTargets = 1,
                .renderTargetFormats = { mRenderer.GetBackbufferFormat() },
                .depthStencilFormat = mRenderer.GetDepthStencilFormat()
            };
        }

        {
            platform::FilePath environmentMappingShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("EnvironmentMapping.slang");

            mGenerateIrradianceMapShader = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Compute,
                    .entryPoint = "GenerateIrradianceMapCS"
                },
                .filePaths = { &environmentMappingShaderFilePath, 1 },
                .debugName = CUBE_T("GenerateIrradianceMap CS")
            });
            CHECK(mGenerateIrradianceMapShader);

            mGenerateIrradianceMapPipelineInfo = {
                .shader = mGenerateIrradianceMapShader
            };

            mGenerateIntegratedBRDFLUTShader = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Compute,
                    .entryPoint = "GenerateIntegratedBRDFLUTCS"
                },
                .filePaths = { &environmentMappingShaderFilePath, 1 },
                .debugName = CUBE_T("GenerateIntegratedBRDFLUT CS")
            });
            CHECK(mGenerateIntegratedBRDFLUTShader);

            mGenerateIntegratedBRDFLUTPipelineInfo = {
                .shader = mGenerateIntegratedBRDFLUTShader
            };

            mGeneratePrefilterMapShader = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Compute,
                    .entryPoint = "GeneratePrefilterMapCS"
                },
                .filePaths = { &environmentMappingShaderFilePath, 1 },
                .debugName = CUBE_T("GeneratePrefilterMap CS")
            });
            CHECK(mGeneratePrefilterMapShader);
            
            mGeneratePrefilterMapPipelineInfo = {
                .shader = mGeneratePrefilterMapShader
            };

            mPrefilterMapSampler = mRenderer.GetSamplerManager().GetSampler({
                .minFilter = gapi::SamplerFilterType::Linear,
                .magFilter = gapi::SamplerFilterType::Linear,
                .mipFilter = gapi::SamplerFilterType::Linear,
                .addressU = gapi::SamplerAddressMode::Wrap,
                .addressV = gapi::SamplerAddressMode::Wrap,
                .addressW = gapi::SamplerAddressMode::Wrap,
                .debugName = CUBE_T("PrefilterMap Sampler")
            });
        }

        mCommandList = mRenderer.GetGAPI().CreateCommandList({
            .debugName = CUBE_T("EnvironmentMappingCommandList")
        });
    }

    void EnvironmentMapping::Shutdown()
    {
        mCommandList = nullptr;

        mGeneratePrefilterMapPipelineInfo = {};
        mGeneratePrefilterMapShader = nullptr;
        mGenerateIntegratedBRDFLUTPipelineInfo = {};
        mGenerateIntegratedBRDFLUTShader = nullptr;
        mGenerateIrradianceMapPipelineInfo = {};
        mGenerateIrradianceMapShader = nullptr;

        mSkyboxPipelineInfo = {};
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
                FrameAnsiString prefilterMapStr = Format<FrameAnsiString>("PrefilterMap {0}", mCurrentSkyboxMipLevel);
                auto GetSkyboxTypeStr = [this, &prefilterMapStr]() -> const char*
                {
                    switch (mCurrentSkyboxType)
                    {
                    case SkyboxType::None: return "None";
                    case SkyboxType::IBL: return "IBL";
                    case SkyboxType::DiffuseIrradianceMap: return "DiffuseIrradianceMap";
                    case SkyboxType::PrefilterMap: return prefilterMapStr.c_str();
                    case SkyboxType::Num: return "Num";
                    }
                    return "";
                };
                ImGui::SetNextItemWidth(160);
                if (ImGui::BeginCombo("Skybox", GetSkyboxTypeStr()))
                {
                    if (ImGui::Selectable("None", mCurrentSkyboxType == SkyboxType::None))
                    {
                        mCurrentSkyboxType = SkyboxType::None;
                    }
                    if (mIBLTexture && ImGui::Selectable("IBL", mCurrentSkyboxType == SkyboxType::IBL))
                    {
                        mCurrentSkyboxType = SkyboxType::IBL;
                    }
                    if (mDiffuseIrradianceMap && ImGui::Selectable("DiffuseIrradianceMap", mCurrentSkyboxType == SkyboxType::DiffuseIrradianceMap))
                    {
                        mCurrentSkyboxType = SkyboxType::DiffuseIrradianceMap;
                    }
                    if (mPrefilterMap)
                    {
                        const int mipLevels = mPrefilterMap->GetMipLevels();
                        for (int i = 0; i < mipLevels; ++i)
                        {
                            FrameAnsiString label = Format<FrameAnsiString>("PrefilterMap {0}", i);
                            if (ImGui::Selectable(label.c_str(), mCurrentSkyboxType == SkyboxType::PrefilterMap && mCurrentSkyboxMipLevel == i))
                            {
                                mCurrentSkyboxType = SkyboxType::PrefilterMap;
                                mCurrentSkyboxMipLevel = i;
                            }
                        }
                    }
                    
                    ImGui::EndCombo();
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
            GeneratePrefilterMap();
        }

        if (!mIntegratedBRDFLUT)
        {
            GenerateIntegratedBRDFLUT();
        }
    }

    void EnvironmentMapping::ClearResources()
    {
        mPrefilterMap = nullptr;
        mIntegratedBRDFLUT = nullptr;
        mDiffuseIrradianceMap = nullptr;
        mIBLTexture = nullptr;
    }

    void EnvironmentMapping::SetEnable(bool newEnable)
    {
        mIsEnabled = newEnable;
    }

    void EnvironmentMapping::DrawSkybox(RGBuilder& builder)
    {
        if (!IsSupported() || !mIsEnabled || mCurrentSkyboxType == SkyboxType::None)
        {
            return;
        }

        RGTextureHandle skyboxTexture;
        RGTextureSRVHandle skyboxSRV;
        switch (mCurrentSkyboxType)
        {
        default:
        case SkyboxType::IBL:
        {
            skyboxTexture = builder.RegisterTexture(mIBLTexture->GetGAPITexture());
            skyboxSRV = builder.CreateSRV(skyboxTexture);
            break;
        }
        case SkyboxType::DiffuseIrradianceMap:
        {
            skyboxTexture = builder.RegisterTexture(mDiffuseIrradianceMap);
            skyboxSRV = builder.CreateSRV(skyboxTexture);
            break;
        }
        case SkyboxType::PrefilterMap:
        {
            skyboxTexture = builder.RegisterTexture(mPrefilterMap);
            skyboxSRV = builder.CreateSRV(skyboxTexture, mCurrentSkyboxMipLevel, 1);
            break;
        }
        }

        SharedPtr<Mesh> boxMesh = mRenderer.GetBoxMesh();
        RGBufferHandle rgBoxVertexBuffer = builder.RegisterBuffer(boxMesh->GetVertexBuffer());
        RGBufferSRVHandle rgBoxVertexBufferSRV = builder.CreateSRV(rgBoxVertexBuffer);

        auto skyboxParams = builder.CreateShaderParameterList<SkyboxShaderParameterList>();
        skyboxParams->Get()->skyboxTexture = skyboxSRV;
        skyboxParams->Get()->vertexBuffer = rgBoxVertexBufferSRV;
        skyboxParams->Get()->useFP16 = boxMesh->GetMeta().useFloat16;

        mSkyboxPipelineInfo.rasterizerState.fillMode = mRenderer.IsDrawInWireframe()
            ? gapi::RasterizerState::FillMode::Line
            : gapi::RasterizerState::FillMode::Solid;
        SharedPtr<GraphicsPipeline> skyboxPipeline = mRenderer.GetPipelineManager().GetOrCreateGraphicsPipeline({
            .pipelineInfo = mSkyboxPipelineInfo,
            .debugName = CUBE_T("SkyboxPipeline")
        });

        builder.AddPass(CUBE_T("Skybox"), skyboxPipeline, skyboxParams,
        [boxMesh](gapi::CommandList& commandList)
        {
            const SubMesh& boxSubMesh = boxMesh->GetSubMeshes()[0];
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

    RGTextureSRVHandle EnvironmentMapping::GetIntegratedBRDFLUT(RGBuilder& builder) const
    {
        if (mIsEnabled && mIntegratedBRDFLUT)
        {
            RGTextureHandle rgTexture = builder.RegisterTexture(mIntegratedBRDFLUT);
            return builder.CreateSRV(rgTexture);
        }
        else
        {
            return builder.GetDummyBlackTexture2D();
        }
    }

    RGTextureSRVHandle EnvironmentMapping::GetPrefilterMap(RGBuilder& builder) const
    {
        if (mIsEnabled && mPrefilterMap)
        {
            RGTextureHandle rgTexture = builder.RegisterTexture(mPrefilterMap);
            return builder.CreateSRV(rgTexture);
        }
        else
        {
            return builder.GetDummyBlackTextureCube();
        }
    }

    BindlessSampler EnvironmentMapping::GetPrefilterMapSampler() const
    {
        return mPrefilterMapSampler;
    }

    Uint32 EnvironmentMapping::GetPrefilterMapMipLevels() const
    {
        if (mIsEnabled && mPrefilterMap)
        {
            return mPrefilterMap->GetInfo().mipLevels;
        }
        else
        {
            return 0;
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
            RGTextureHandle srcIBL = builder.RegisterTexture(mIBLTexture->GetGAPITexture());
            RGTextureSRVHandle srcIBLSRV = builder.CreateSRV(srcIBL);
            RGTextureHandle dstDiffuseEnvMap = builder.RegisterTexture(mDiffuseIrradianceMap);
            RGTextureUAVHandle dstDiffuseEnvMapUAV = builder.CreateUAV(dstDiffuseEnvMap);

            RGShaderParameterListHandle<GenerateIrradianceMapShaderParameterList> params = builder.CreateShaderParameterList<GenerateIrradianceMapShaderParameterList>();
            params->Get()->numSlices = 128;
            params->Get()->widthAndInvWidth = Vector2(static_cast<float>(width), 1.0f / static_cast<float>(width));
            params->Get()->srcIBL = srcIBLSRV;
            params->Get()->dstDiffuseIrradianceMap = dstDiffuseEnvMapUAV;

            SharedPtr<ComputePipeline> generateIrradianceMapPipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline({
                .pipelineInfo = mGenerateIrradianceMapPipelineInfo,
                .debugName = CUBE_T("GenerateIrradianceMap Pipeline")
            });

            builder.AddPass(CUBE_T("GenerateIrradianceMap"),
                generateIrradianceMapPipeline,
                params,
                [width, height](gapi::CommandList& commandList)
            {
                commandList.DispatchThreads(width, height, 6);
            });
        }
        builder.ExecuteAndSubmit(*mCommandList);
    }

    void EnvironmentMapping::GenerateIntegratedBRDFLUT()
    {
        const gapi::ElementFormat format = gapi::ElementFormat::RG16_Float;
        const Uint32 width = 512;

        mIntegratedBRDFLUT = mRenderer.GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = {
                .format = format,
                .type = gapi::TextureType::Texture2D,
                .flags = gapi::TextureFlag::UAV,
                .width = width,
                .height = width,
            },
            .debugName = CUBE_T("IntegratedBRDF LUT")
        });

        RGBuilder builder(mRenderer);
        {
            RGTextureHandle dstIntegratedBRDFLUT = builder.RegisterTexture(mIntegratedBRDFLUT);
            RGTextureUAVHandle dstIntegratedBRDFLUTUAV = builder.CreateUAV(dstIntegratedBRDFLUT);

            auto params = builder.CreateShaderParameterList<GenerateIntegratedBRDFLUTShaderParameterList>();
            params->Get()->sampleCount = 1024;
            params->Get()->width = width;
            params->Get()->dstIntegratedBRDFLUT = dstIntegratedBRDFLUTUAV;

            SharedPtr<ComputePipeline> generateIntegratedBRDFLUTPipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline({
                .pipelineInfo = mGenerateIntegratedBRDFLUTPipelineInfo,
                .debugName = CUBE_T("GenerateIntegratedBRDFLUT Pipeline")
            });
            builder.AddPass(CUBE_T("Generate IntegratedBRDFLUT"),
                generateIntegratedBRDFLUTPipeline,
                params,
                [width](gapi::CommandList& commandList)
            {
                commandList.DispatchThreads(width, width, 1);
            });
        }
        builder.ExecuteAndSubmit(*mCommandList);
    }

    void EnvironmentMapping::GeneratePrefilterMap()
    {
        CHECK(mIBLTexture);

        const gapi::ElementFormat format = gapi::ElementFormat::RGBA16_Float;
        const Uint32 width = 256;
        const Uint32 mipLevels = 5;

        mPrefilterMap = mRenderer.GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = {
                .format = format,
                .type = gapi::TextureType::TextureCube,
                .flags = gapi::TextureFlag::UAV,
                .width = width,
                .height = width,
                .mipLevels = mipLevels
            },
            .debugName = CUBE_T("Prefilter Map")
        });

        RGBuilder builder(mRenderer);
        {
            RGTextureHandle srcIBL = builder.RegisterTexture(mIBLTexture->GetGAPITexture());
            RGTextureSRVHandle srcIBLSRV = builder.CreateSRV(srcIBL);
            RGTextureHandle dstPrefilterMap = builder.RegisterTexture(mPrefilterMap);

            SharedPtr<ComputePipeline> generatePrefilterMapPipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline({
                .pipelineInfo = mGeneratePrefilterMapPipelineInfo,
                .debugName = CUBE_T("GeneratePrefilterMap Pipeline")
            });

            for (Uint32 mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
            {
                const float roughness = static_cast<float>(mipLevel) / (mipLevels - 1);
                const Uint32 mipWidth = (width >> mipLevel);

                RGTextureUAVHandle dstPrefilterMapUAV = builder.CreateUAV(dstPrefilterMap, mipLevel);

                auto params = builder.CreateShaderParameterList<GeneratePrefilterMapShaderParameterList>();
                params->Get()->numSamples = 256;
                params->Get()->widthAndInvWidth = Vector2(static_cast<float>(mipWidth), 1.0f / static_cast<float>(mipWidth));
                params->Get()->roughness = roughness;
                params->Get()->srcIBL = srcIBLSRV;
                params->Get()->dstPrefilterMap = dstPrefilterMapUAV;

                builder.AddPass(Format<FrameString>(CUBE_T("GeneratePrefilterMap [{0}]"), mipLevel),
                    generatePrefilterMapPipeline,
                    params,
                    [mipWidth](gapi::CommandList& commandList)
                    {
                        commandList.DispatchThreads(mipWidth, mipWidth, 6);
                    }
                );
            }
        }
        builder.ExecuteAndSubmit(*mCommandList);
    }
} // namespace cube
