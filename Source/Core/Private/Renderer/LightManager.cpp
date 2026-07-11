#include "LightManager.h"

#include "imgui.h"

#include "Allocator/FrameAllocator.h"
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

        mEnvironmentMapping.Initialize(true);
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

    void LightManager::Shutdown()
    {
        mPointLightInfoGPUBuffer = nullptr;
        mPointLights.clear();

        mEnvironmentMapping.Shutdown();
    }

    void LightManager::LoadResources()
    {
        mEnvironmentMapping.LoadResources();
    }

    void LightManager::ClearResources()
    {
        mEnvironmentMapping.ClearResources();
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
    }

    void LightManager::BindLightShaderParameterList(RGBuilder& builder)
    {
        RGBufferHandle pointLightInfoBuffer = builder.RegisterBuffer(mPointLightInfoGPUBuffer);
        RGBufferSRVHandle pointLightInfoBufferSRV = builder.CreateSRV(pointLightInfoBuffer);

        auto lightShaderParameterList = builder.CreateShaderParameterList<LightShaderParameterList>();
        lightShaderParameterList->isDirectionalLightEnabled = mDirectionalLight.IsEnabled();
        lightShaderParameterList->directionalLightDirection = mDirectionalLight.GetDirection();
        lightShaderParameterList->directionalLightIntensity = mDirectionalLight.GetIntensity();
        lightShaderParameterList->numPointLights = mNumActivePointLights;
        lightShaderParameterList->pointLightInfos = pointLightInfoBufferSRV;
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
} // namespace cube
