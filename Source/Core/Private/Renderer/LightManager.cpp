#include "LightManager.h"

#include "imgui.h"

#include "Allocator/FrameAllocator.h"
#include "LTC.h"
#include "RenderCore/RenderGraph.h"
#include "Vector.h"

namespace cube
{
    class LightShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(LightShaderParameterList)
            CUBE_SHADER_PARAMETER(bool, isDirectionalLightEnabled)
            CUBE_SHADER_PARAMETER(Float3, directionalLightDirection)
            CUBE_SHADER_PARAMETER(Float3, directionalLightIntensity)
            CUBE_SHADER_PARAMETER(Uint32, numPointLights)
            CUBE_SHADER_PARAMETER(RGBufferSRVHandle, pointLightInfos)
            CUBE_SHADER_PARAMETER(Uint32, numRectLights)
            CUBE_SHADER_PARAMETER(RGBufferSRVHandle, rectLightInfos)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, rectLightLTC1)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, rectLightLTC2)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(LightShaderParameterList);

    LightManager::LightManager(Renderer& renderer)
        : mRenderer(renderer)
        , mEnvironmentMapping(renderer)
    {
    }

    void LightManager::Initialize()
    {
        AddPointLight({ 4.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });
        AddPointLight({ 0.0f, 4.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
        AddPointLight({ 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f, 1.0f });

        AddRectLight({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f });

        mPointLightInfoGPUBuffer = mRenderer.GetGAPI().CreateBuffer({
            .usage = gapi::ResourceUsage::GPUOnly,
            .bufferInfo = {
                .type = gapi::BufferType::Structured,
                .size = sizeof(PointLightInfo) * MAX_POINT_LIGHTS,
                .stride = sizeof(PointLightInfo),
                .flags = gapi::BufferFlag::None,
            },
            .debugName = CUBE_T("PointLight Buffer"),
        });

        mRectLightInfoGPUBuffer = mRenderer.GetGAPI().CreateBuffer({
            .usage = gapi::ResourceUsage::GPUOnly,
            .bufferInfo = {
                .type = gapi::BufferType::Structured,
                .size = sizeof(RectLightInfo) * MAX_RECT_LIGHTS,
                .stride = sizeof(RectLightInfo),
                .flags = gapi::BufferFlag::None,
            },
            .debugName = CUBE_T("RectLight Buffer"),
        });

        mEnvironmentMapping.Initialize(true);
    }

    void LightManager::Shutdown()
    {
        mRectLightInfoGPUBuffer = nullptr;
        mRectLights.clear();
        mPointLightInfoGPUBuffer = nullptr;
        mPointLights.clear();

        mEnvironmentMapping.Shutdown();
    }

    void LightManager::LoadResources()
    {
        LoadLTCTexture();

        mEnvironmentMapping.LoadResources();
    }

    void LightManager::ClearResources()
    {
        mEnvironmentMapping.ClearResources();

        mLTCTexture2 = nullptr;
        mLTCTexture1 = nullptr;
    }

    void LightManager::OnLoopImGUIContent()
    {
        constexpr ImGuiTreeNodeFlags kTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;

        const bool isDirectionalLightOpened = ImGui::TreeNodeEx("Directional Light", kTreeNodeFlags);
        ImGui::SameLine();
        bool isDirectionalLightEnabled = mDirectionalLight.IsEnabled();
        if (ImGui::Checkbox("##DirectionalLightEnable", &isDirectionalLightEnabled))
        {
            mDirectionalLight.SetEnable(isDirectionalLightEnabled);
        }
        if (isDirectionalLightOpened)
        {
            mDirectionalLight.OnLoopImGUIContent();
            ImGui::TreePop();
        }

        const bool isPointLightsOpened = ImGui::TreeNodeEx("Point Lights", kTreeNodeFlags);
        ImGui::SameLine();
        ImGui::BeginDisabled(mPointLights.size() >= MAX_POINT_LIGHTS);
        if (ImGui::Button("+"))
        {
            AddPointLight({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
        }
        ImGui::EndDisabled();

        if (isPointLightsOpened)
        {
            int indexToRemove = -1;

            for (int i = 0; i < static_cast<int>(mPointLights.size()); ++i)
            {
                PointLight& pointLight = mPointLights[i];

                ImGui::PushID(i);

                FrameAnsiString label = Format<FrameAnsiString>("Point Light {0}", i);

                const bool isPointLightOpened = ImGui::TreeNodeEx(label.c_str(), kTreeNodeFlags);
                ImGui::SameLine();
                bool isPointLightEnabled = pointLight.IsEnabled();
                if (ImGui::Checkbox("##PointLightEnable", &isPointLightEnabled))
                {
                    pointLight.SetEnable(isPointLightEnabled);
                }
                ImGui::SameLine();
                if (ImGui::Button("X"))
                {
                    indexToRemove = i;
                }

                if (isPointLightOpened)
                {
                    mPointLights[i].OnLoopImGUIContent();
                    ImGui::TreePop();
                }

                ImGui::PopID();
            }

            if (indexToRemove >= 0)
            {
                mPointLights.erase(mPointLights.begin() + indexToRemove);
            }

            ImGui::TreePop();
        }

        const bool isRectLightsOpened = ImGui::TreeNodeEx("Rect Lights", kTreeNodeFlags);
        ImGui::SameLine();
        ImGui::BeginDisabled(mRectLights.size() >= MAX_RECT_LIGHTS);
        if (ImGui::Button("+"))
        {
            AddRectLight({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f });
        }
        ImGui::EndDisabled();

        if (isRectLightsOpened)
        {
            int indexToRemove = -1;

            for (int i = 0; i < static_cast<int>(mRectLights.size()); ++i)
            {
                RectLight& rectLight = mRectLights[i];

                ImGui::PushID(i);

                FrameAnsiString label = Format<FrameAnsiString>("Rect Light {0}", i);

                const bool isRectLightOpened = ImGui::TreeNodeEx(label.c_str(), kTreeNodeFlags);
                ImGui::SameLine();
                bool isRectLightEnabled = rectLight.IsEnabled();
                if (ImGui::Checkbox("##RectLightEnable", &isRectLightEnabled))
                {
                    rectLight.SetEnable(isRectLightEnabled);
                }
                ImGui::SameLine();
                if (ImGui::Button("X"))
                {
                    indexToRemove = i;
                }

                if (isRectLightOpened)
                {
                    mRectLights[i].OnLoopImGUIContent();
                    ImGui::TreePop();
                }

                ImGui::PopID();
            }

            if (indexToRemove >= 0)
            {
                mRectLights.erase(mRectLights.begin() + indexToRemove);
            }

            ImGui::TreePop();
        }

        const bool isEnvironmentMappingOpened = ImGui::TreeNodeEx("Environment Mapping", kTreeNodeFlags);
        ImGui::SameLine();
        bool isEnvironmentMappingEnabled = mEnvironmentMapping.IsEnabled();
        ImGui::BeginDisabled(!mEnvironmentMapping.IsSupported());
        if (ImGui::Checkbox("##EnvironmentMappingEnable", &isEnvironmentMappingEnabled))
        {
            mEnvironmentMapping.SetEnable(isEnvironmentMappingEnabled);
        }
        ImGui::EndDisabled();
        if (isEnvironmentMappingOpened)
        {
            mEnvironmentMapping.OnLoopImGUI();
            ImGui::TreePop();
        }
    }

    void LightManager::UpdateLightInfoBuffers(RGBuilder& builder)
    {
        FrameVector<int> activePointLightIndices;
        for (int i = 0; i < static_cast<int>(mPointLights.size()); ++i)
        {
            PointLight& pointLight = mPointLights[i];
            if (pointLight.IsEnabled())
            {
                activePointLightIndices.push_back(i);
            }
        }

        mNumActivePointLights = static_cast<Uint32>(activePointLightIndices.size());
        if (mNumActivePointLights > 0)
        {
            CHECK(mNumActivePointLights <= MAX_POINT_LIGHTS);

            UploadManager& uploadManager = mRenderer.GetUploadManager();

            UploadDesc uploadDesc = uploadManager.Allocate(mPointLightInfoGPUBuffer, {
                .offset = 0,
                .size = sizeof(PointLightInfo) * mNumActivePointLights,
            });

            int bufferIndex = 0;
            for (int index : activePointLightIndices)
            {
                PointLight& pointLight = mPointLights[index];

                PointLightInfo pointLightInfo = {
                    .position = pointLight.GetPosition(),
                    .intensity = pointLight.GetIntensity(),
                };
                Byte* dst = (Byte*)uploadDesc.pData + sizeof(PointLightInfo) * bufferIndex;
                memcpy(dst, &pointLightInfo, sizeof(PointLightInfo));
                bufferIndex++;
            }

            RGBufferHandle rgPointLightInfoGPUBuffer = builder.RegisterBuffer(mPointLightInfoGPUBuffer);

            builder.AddPass(CUBE_T("##UpdateLightInfoBuffers - Upload PointLights"),
                [&uploadManager, uploadDesc](gapi::CommandList& commandList) mutable {
                    uploadManager.Submit(uploadDesc, &commandList);
                },
                [rgPointLightInfoGPUBuffer](RGBuilder& builder) {
                    builder.UseResource(rgPointLightInfoGPUBuffer, gapi::ResourceAccessFlag::CopyDst, gapi::ResourceSyncFlag::Copy);
                }
            );
        }

        FrameVector<int> activeRectLightIndices;
        for (int i = 0; i < static_cast<int>(mRectLights.size()); ++i)
        {
            RectLight& rectLight = mRectLights[i];
            if (rectLight.IsEnabled())
            {
                activeRectLightIndices.push_back(i);
            }
        }

        mNumActiveRectLights = static_cast<Uint32>(activeRectLightIndices.size());
        if (mNumActiveRectLights > 0)
        {
            CHECK(mNumActiveRectLights <= MAX_RECT_LIGHTS);

            UploadManager& uploadManager = mRenderer.GetUploadManager();

            UploadDesc uploadDesc = uploadManager.Allocate(mRectLightInfoGPUBuffer, {
                .offset = 0,
                .size = sizeof(RectLightInfo) * mNumActiveRectLights,
            });

            int bufferIndex = 0;
            for (int index : activeRectLightIndices)
            {
                RectLight& rectLight = mRectLights[index];

                RectLightInfo rectLightInfo = {
                    .position = rectLight.GetPosition(),
                    .direction = rectLight.GetDirection(),
                    .rectSize = rectLight.GetRectSize(),
                    .intensity = rectLight.GetIntensity(),
                };
                Byte* dst = (Byte*)uploadDesc.pData + sizeof(RectLightInfo) * bufferIndex;
                memcpy(dst, &rectLightInfo, sizeof(RectLightInfo));
                bufferIndex++;
            }

            RGBufferHandle rgRectLightInfoGPUBuffer = builder.RegisterBuffer(mRectLightInfoGPUBuffer);

            builder.AddPass(CUBE_T("##UpdateLightInfoBuffers - Upload RectLights"),
                [&uploadManager, uploadDesc](gapi::CommandList& commandList) mutable {
                    uploadManager.Submit(uploadDesc, &commandList);
                },
                [rgRectLightInfoGPUBuffer](RGBuilder& builder) {
                    builder.UseResource(rgRectLightInfoGPUBuffer, gapi::ResourceAccessFlag::CopyDst, gapi::ResourceSyncFlag::Copy);
                }
            );
        }
    }

    void LightManager::BindLightShaderParameterList(RGBuilder& builder)
    {
        RGBufferHandle pointLightInfoBuffer = builder.RegisterBuffer(mPointLightInfoGPUBuffer);
        RGBufferSRVHandle pointLightInfoBufferSRV = builder.CreateSRV(pointLightInfoBuffer);
        RGBufferHandle rectLightInfoBuffer = builder.RegisterBuffer(mRectLightInfoGPUBuffer);
        RGBufferSRVHandle rectLightInfoBufferSRV = builder.CreateSRV(rectLightInfoBuffer);
        RGTextureHandle LTC1 = builder.RegisterTexture(mLTCTexture1->GetGAPITexture());
        RGTextureSRVHandle LTC1SRV = builder.CreateSRV(LTC1);
        RGTextureHandle LTC2 = builder.RegisterTexture(mLTCTexture2->GetGAPITexture());
        RGTextureSRVHandle LTC2SRV = builder.CreateSRV(LTC2);
        
        auto lightShaderParameterList = builder.CreateShaderParameterList<LightShaderParameterList>();
        lightShaderParameterList->isDirectionalLightEnabled = mDirectionalLight.IsEnabled();
        lightShaderParameterList->directionalLightDirection = mDirectionalLight.GetDirection();
        lightShaderParameterList->directionalLightIntensity = mDirectionalLight.GetIntensity();
        lightShaderParameterList->numPointLights = mNumActivePointLights;
        lightShaderParameterList->pointLightInfos = pointLightInfoBufferSRV;
        lightShaderParameterList->numRectLights = mNumActiveRectLights;
        lightShaderParameterList->rectLightInfos = rectLightInfoBufferSRV;
        lightShaderParameterList->rectLightLTC1 = LTC1SRV;
        lightShaderParameterList->rectLightLTC2 = LTC2SRV;
        builder.BindGlobalShaderParameterList(lightShaderParameterList);

        auto envMapShaderParameterList = builder.CreateShaderParameterList<EnvironmentMapLightShaderParameterList>();
        envMapShaderParameterList->diffuseIrradianceMap = mEnvironmentMapping.GetDiffuseIrradianceMap(builder);
        envMapShaderParameterList->integratedBRDFLUT = mEnvironmentMapping.GetIntegratedBRDFLUT(builder);
        envMapShaderParameterList->prefilterMap = mEnvironmentMapping.GetPrefilterMap(builder);
        envMapShaderParameterList->prefilterSampler = mEnvironmentMapping.GetPrefilterMapSampler();
        envMapShaderParameterList->prefilterMapMipLevels = mEnvironmentMapping.GetPrefilterMapMipLevels();
        builder.BindGlobalShaderParameterList(envMapShaderParameterList);
    }

    void LightManager::UnbindLightShaderParameterList(RGBuilder& builder)
    {
        builder.UnbindGlobalShaderParameterList<EnvironmentMapLightShaderParameterList>();
        builder.UnbindGlobalShaderParameterList<LightShaderParameterList>();
    }

    void LightManager::LoadLTCTexture()
    {
        Blob ltc1Data(LTC1, sizeof(LTC1));

        TextureResourceCreateInfo createInfo = {
            .textureInfo = {
                .format = gapi::ElementFormat::RGBA32_Float,
                .type = gapi::TextureType::Texture2D,
                .width = 64,
                .height = 64,
            },
            .data = ltc1Data,
            .bytesPerElement = sizeof(float) * 4,
            .debugName = CUBE_T("LTC1"),
        };
        mLTCTexture1 = TextureResource::Create(createInfo);

        Blob ltc2Data(LTC2, sizeof(LTC2));
        createInfo.data = ltc2Data;
        createInfo.debugName = CUBE_T("LTC2");
        mLTCTexture2 = TextureResource::Create(createInfo);
    }

    void LightManager::AddPointLight(const Float3& position, const Float3& intensity)
    {
        if (mPointLights.size() >= MAX_POINT_LIGHTS)
        {
            return;
        }

        PointLight newLight;
        newLight.SetPosition(position);
        newLight.SetIntensity(intensity);
        mPointLights.push_back(newLight);
    }

    void LightManager::AddRectLight(const Float3& position, const Float3& direction, const Float2& rectSize, const Float3& intensity)
    {
        if (mRectLights.size() >= MAX_RECT_LIGHTS)
        {
            return;
        }

        RectLight newRectLight;
        newRectLight.SetPosition(position);
        newRectLight.SetDirection(direction);
        newRectLight.SetRectSize(rectSize);
        newRectLight.SetIntensity(intensity);
        mRectLights.push_back(newRectLight);
    }
} // namespace cube
