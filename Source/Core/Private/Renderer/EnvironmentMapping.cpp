#include "EnvironmentMapping.h"

#include "imgui.h"

#include "GAPI_Shader.h"
#include "RenderCore/Pipeline.h"
#include "RenderCore/Mesh.h"
#include "RenderCore/RenderGraph.h"
#include "RenderCore/Shader.h"
#include "Allocator/FrameAllocator.h"

namespace cube
{
    class GenerateIrradianceMapShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(GenerateIrradianceMapShaderParameterList)
            CUBE_SHADER_PARAMETER(Uint32, numSlices)
            CUBE_SHADER_PARAMETER(Float2, widthAndInvWidth)
            CUBE_SHADER_PARAMETER(Uint3, tileOffsetAndCubeFaceIndex)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, srcIBL)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstDiffuseIrradianceMap)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(GenerateIrradianceMapShaderParameterList);

    class GenerateIntegratedBRDFLUTShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(GenerateIntegratedBRDFLUTShaderParameterList)
            CUBE_SHADER_PARAMETER(Uint32, sampleCount)
            CUBE_SHADER_PARAMETER(Uint32, width)
            CUBE_SHADER_PARAMETER(Uint2, tileOffset)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstIntegratedBRDFLUT)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(GenerateIntegratedBRDFLUTShaderParameterList);

    class GeneratePrefilterMapShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(GeneratePrefilterMapShaderParameterList)
            CUBE_SHADER_PARAMETER(Uint32, numSamples)
            CUBE_SHADER_PARAMETER(Float2, widthAndInvWidth)
            CUBE_SHADER_PARAMETER(Uint3, tileOffsetAndCubeFaceIndex)
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
        mIBLDropdownExpandedLastFrame = false;

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
    }

    void EnvironmentMapping::Shutdown()
    {
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

            ImGui::SetNextItemWidth(160);
            if (ImGui::BeginCombo("IBL Texture", mCurrentSelectedIBLTextureName.c_str()))
            {
                if (!mIBLDropdownExpandedLastFrame)
                {
                    LoadIBLTextureList();
                }
                mIBLDropdownExpandedLastFrame = true;

                for (const AnsiString& IBLTextureName : mIBLTextureList)
                {
                    const bool selected = mCurrentSelectedIBLTextureName == IBLTextureName;
                    if (ImGui::Selectable(IBLTextureName.c_str(), selected))
                    {
                        mCurrentSelectedIBLTextureName = IBLTextureName;
                        LoadCurrentIBLTexture();
                    }

                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }
            else
            {
                mIBLDropdownExpandedLastFrame = false;
            }

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
        // Load the first IBL texture as default.
        LoadIBLTextureList();
        if (mIBLTextureList.size() > 0)
        {
            mCurrentSelectedIBLTextureName = mIBLTextureList[0];
            LoadCurrentIBLTexture();

            if (!mIBLTexture)
            {
                // Failed to load.
                mCurrentSelectedIBLTextureName = "";
            }
        }

        if (!mIntegratedBRDFLUT)
        {
            QueueGenerateIntegratedBRDFLUT();
        }
    }

    void EnvironmentMapping::ClearResources()
    {
        mIntegratedBRDFLUT = nullptr;

        ClearCurrentIBLTexture();
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
            skyboxSRV = builder.CreateSRV(skyboxTexture, { .subresourceRange = { .firstMipLevel = mCurrentSkyboxMipLevel, .mipLevels = 1 } });
            break;
        }
        }

        auto skyboxParams = builder.CreateShaderParameterList<SkyboxShaderParameterList>();
        skyboxParams->skyboxTexture = skyboxSRV;

        mSkyboxPipelineInfo.rasterizerState.fillMode = mRenderer.IsDrawInWireframe()
            ? gapi::RasterizerState::FillMode::Line
            : gapi::RasterizerState::FillMode::Solid;
        builder.ApplyRenderTargetFormatsFromCurrentRenderPass(mSkyboxPipelineInfo);

        SharedPtr<GraphicsPipeline> skyboxPipeline = mRenderer.GetPipelineManager().GetOrCreateGraphicsPipeline({
            .pipelineInfo = mSkyboxPipelineInfo,
            .debugName = CUBE_T("SkyboxPipeline")
        });

        builder.AddPass(CUBE_T("Skybox"), skyboxPipeline, skyboxParams,
        [](gapi::CommandList& commandList)
        {
            // 6 faces * 2 triangles * 3 vertices.
            commandList.Draw(6 * 2 * 3, 0);
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

    void EnvironmentMapping::LoadIBLTextureList()
    {
        mIBLTextureList.clear();

        const platform::FilePath IBLBasePath = Engine::GetRootDirectoryPath() / CUBE_T("Resources/Textures/IBL");
        static const char* nameListToLoad[] = {
            "Brudslojan",
            "NissiBeach2"
        };
        for (const char* nameToLoad : nameListToLoad)
        {
            if (platform::FileSystem::IsExist(IBLBasePath / nameToLoad))
            {
                mIBLTextureList.push_back(nameToLoad);
            }
        }
    }

    void EnvironmentMapping::ClearCurrentIBLTexture()
    {
        mPrefilterMap = nullptr;
        mDiffuseIrradianceMap = nullptr;
        mIBLTexture = nullptr;
    }

    void EnvironmentMapping::LoadCurrentIBLTexture()
    {
        ClearCurrentIBLTexture();

        const platform::FilePath IBLPath = Engine::GetRootDirectoryPath() / CUBE_T("Resources/Textures/IBL") / mCurrentSelectedIBLTextureName;
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
                    .height = negXData.height,
                },
                .data = BlobView(totalData),
                .bytesPerElement = negXData.bytesPerElement,
                .debugName = CUBE_T("IBLTexture")
            };
            mIBLTexture = TextureResource::Create(createInfo);

            QueueGenerateIrradianceMap();
            QueueGeneratePrefilterMap();
        }
    }

    void EnvironmentMapping::QueueGenerateIrradianceMap()
    {
        CHECK(mIBLTexture);

        mDiffuseIrradianceMap = mRenderer.GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = {
                .format = gapi::ElementFormat::RGBA16_Float,
                .type = gapi::TextureType::TextureCube,
                .flags = gapi::TextureFlag::UAV,
                .width = 256,
                .height = 256,
            },
            .debugName = CUBE_T("Irradiance Map")
        });

        mRenderer.GetResourceManager().QueuePreprocessTask([this](RGBuilder& builder)
        {
            GenerateIrradianceMap(builder);
        });
    }

    void EnvironmentMapping::GenerateIrradianceMap(RGBuilder& builder)
    {
        CHECK(mIBLTexture);
        CHECK(mDiffuseIrradianceMap);

        const Uint32 widthAndHeight = 256;

        const Uint32 numSlices = 128;
        // Split work into tile to avoid TDR.
        const Uint32 tileSize = 32;

        {
            RGTextureHandle srcIBL = builder.RegisterTexture(mIBLTexture->GetGAPITexture());
            RGTextureSRVHandle srcIBLSRV = builder.CreateSRV(srcIBL);
            RGTextureHandle dstDiffuseEnvMap = builder.RegisterTexture(mDiffuseIrradianceMap);
            RGTextureUAVHandle dstDiffuseEnvMapUAV = builder.CreateUAV(dstDiffuseEnvMap);

            SharedPtr<ComputePipeline> generateIrradianceMapPipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline({
                .pipelineInfo = mGenerateIrradianceMapPipelineInfo,
                .debugName = CUBE_T("GenerateIrradianceMap Pipeline")
            });

            for (Uint32 cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex)
            {
                for (Uint32 offsetX = 0; offsetX < widthAndHeight; offsetX += tileSize)
                {
                    for (Uint32 offsetY = 0; offsetY < widthAndHeight; offsetY += tileSize)
                    {
                        auto params = builder.CreateShaderParameterList<GenerateIrradianceMapShaderParameterList>();
                        params->numSlices = numSlices;
                        params->widthAndInvWidth = Float2(static_cast<float>(widthAndHeight), 1.0f / static_cast<float>(widthAndHeight));
                        params->tileOffsetAndCubeFaceIndex = Uint3(offsetX, offsetY, cubeFaceIndex);
                        params->srcIBL = srcIBLSRV;
                        params->dstDiffuseIrradianceMap = dstDiffuseEnvMapUAV;

                        const Uint2 dispatchTileSize = Uint2(
                            std::min(tileSize, widthAndHeight - offsetX),
                            std::min(tileSize, widthAndHeight - offsetY)
                        );

                        builder.AddPass(Format<FrameString>(CUBE_T("Generate IrradianceMap [({0},{1}), ({2},{3}), face: {4}]"),
                                            offsetX, offsetY, offsetX + dispatchTileSize.x, offsetY + dispatchTileSize.y, cubeFaceIndex),
                            generateIrradianceMapPipeline,
                            params,
                            [dispatchTileSize](gapi::CommandList& commandList)
                            {
                                commandList.DispatchThreads(dispatchTileSize.x, dispatchTileSize.y, 1);
                            }
                        );
                    }
                }
            }
        }
    }

    void EnvironmentMapping::QueueGenerateIntegratedBRDFLUT()
    {
        mIntegratedBRDFLUT = mRenderer.GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = {
                .format = gapi::ElementFormat::RG16_Float,
                .type = gapi::TextureType::Texture2D,
                .flags = gapi::TextureFlag::UAV,
                .width = 512,
                .height = 512,
            },
            .debugName = CUBE_T("IntegratedBRDF LUT")
        });

        mRenderer.GetResourceManager().QueuePreprocessTask([this](RGBuilder& builder)
        {
            GenerateIntegratedBRDFLUT(builder);
        });
    }

    void EnvironmentMapping::GenerateIntegratedBRDFLUT(RGBuilder& builder)
    {
        CHECK(mIntegratedBRDFLUT);

        const Uint32 widthAndHeight = mIntegratedBRDFLUT->GetInfo().width;

        // Split work into tile to avoid TDR.
        const Uint32 tileSize = 32;

        {
            RGTextureHandle dstIntegratedBRDFLUT = builder.RegisterTexture(mIntegratedBRDFLUT);
            RGTextureUAVHandle dstIntegratedBRDFLUTUAV = builder.CreateUAV(dstIntegratedBRDFLUT);

            SharedPtr<ComputePipeline> generateIntegratedBRDFLUTPipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline({
                .pipelineInfo = mGenerateIntegratedBRDFLUTPipelineInfo,
                .debugName = CUBE_T("GenerateIntegratedBRDFLUT Pipeline")
            });

            for (Uint32 offsetX = 0; offsetX < widthAndHeight; offsetX += tileSize)
            {
                for (Uint32 offsetY = 0; offsetY < widthAndHeight; offsetY += tileSize)
                {
                    auto params = builder.CreateShaderParameterList<GenerateIntegratedBRDFLUTShaderParameterList>();
                    params->sampleCount = 1024;
                    params->width = widthAndHeight;
                    params->tileOffset = Uint2(offsetX, offsetY);
                    params->dstIntegratedBRDFLUT = dstIntegratedBRDFLUTUAV;

                    const Uint2 dispatchTileSize = Uint2(
                        std::min(tileSize, widthAndHeight - offsetX),
                        std::min(tileSize, widthAndHeight - offsetY)
                    );

                    builder.AddPass(Format<FrameString>(CUBE_T("Generate IntegratedBRDFLUT [({0},{1}), ({2},{3})]"),
                                        offsetX, offsetY, offsetX + dispatchTileSize.x, offsetY + dispatchTileSize.y),
                        generateIntegratedBRDFLUTPipeline,
                        params,
                        [dispatchTileSize](gapi::CommandList& commandList)
                        {
                            commandList.DispatchThreads(dispatchTileSize.x, dispatchTileSize.y, 1);
                        }
                    );
                }
            }
        }
    }

    void EnvironmentMapping::QueueGeneratePrefilterMap()
    {
        CHECK(mIBLTexture);

        mPrefilterMap = mRenderer.GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = {
                .format = gapi::ElementFormat::RGBA16_Float,
                .type = gapi::TextureType::TextureCube,
                .flags = gapi::TextureFlag::UAV,
                .width = 256,
                .height = 256,
                .mipLevels = 5
            },
            .debugName = CUBE_T("Prefilter Map")
        });

        mRenderer.GetResourceManager().QueuePreprocessTask([this](RGBuilder& builder)
        {
            GeneratePrefilterMap(builder);
        });
    }

    void EnvironmentMapping::GeneratePrefilterMap(RGBuilder& builder)
    {
        CHECK(mIBLTexture);
        CHECK(mPrefilterMap);

        const Uint32 widthAndHeight = mPrefilterMap->GetInfo().width;
        const Uint32 mipLevels = mPrefilterMap->GetInfo().mipLevels;

        // Split work into tile to avoid TDR.
        const Uint32 tileSize = 32;

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
                const Uint32 mipWidthAndHeight = (widthAndHeight >> mipLevel);

                RGTextureUAVHandle dstPrefilterMapUAV = builder.CreateUAV(dstPrefilterMap, { .subresourceRange = { .firstMipLevel = mipLevel } });

                for (Uint32 cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex)
                {
                    for (Uint32 offsetX = 0; offsetX < mipWidthAndHeight; offsetX += tileSize)
                    {
                        for (Uint32 offsetY = 0; offsetY < mipWidthAndHeight; offsetY += tileSize)
                        {
                            auto params = builder.CreateShaderParameterList<GeneratePrefilterMapShaderParameterList>();
                            params->numSamples = 256;
                            params->widthAndInvWidth = Float2(static_cast<float>(mipWidthAndHeight), 1.0f / static_cast<float>(mipWidthAndHeight));
                            params->tileOffsetAndCubeFaceIndex = Uint3(offsetX, offsetY, cubeFaceIndex);
                            params->roughness = roughness;
                            params->srcIBL = srcIBLSRV;
                            params->dstPrefilterMap = dstPrefilterMapUAV;

                            const Uint2 dispatchTileSize = Uint2(
                                std::min(tileSize, mipWidthAndHeight - offsetX),
                                std::min(tileSize, mipWidthAndHeight - offsetY)
                            );

                            builder.AddPass(Format<FrameString>(CUBE_T("Generate PrefilterMap [({0},{1}), ({2},{3}), face: {4}, mipLevel: {5}]"),
                                                offsetX, offsetY, offsetX + dispatchTileSize.x, offsetY + dispatchTileSize.y, cubeFaceIndex, mipLevel),
                                generatePrefilterMapPipeline,
                                params,
                                [dispatchTileSize](gapi::CommandList& commandList)
                                {
                                    commandList.DispatchThreads(dispatchTileSize.x, dispatchTileSize.y, 1);
                                }
                            );
                        }
                    }
                }
            }
        }
    }
} // namespace cube
