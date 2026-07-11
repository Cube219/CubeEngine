#include "LightManager.h"

#include "imguizmo_quat/imGuIZMOquat.h"
#include "imgui.h"

#include "RenderGraph.h"
#include "Vector.h"

namespace cube
{
    class LightShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(LightShaderParameterList)
            CUBE_SHADER_PARAMETER(bool, isDirectionalLightEnabled)
            CUBE_SHADER_PARAMETER(Float3, directionalLightDirection)
            CUBE_SHADER_PARAMETER(Float3, directionalLightIntensity)
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
        mIsDirectionalLightEnabled = true;
        mDirectionalLight.SetDirection({ 1.0f, 1.0f, 1.0f });
        mDirectionalLight.SetIntensity({ 1.0f, 1.0f, 1.0f });

        mEnvironmentMapping.Initialize(true);
    }

    void LightManager::Shutdown()
    {
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
        ImGui::SeparatorText("Directional Light");
        ImGui::Checkbox("Enable##Directional Light", &mIsDirectionalLightEnabled);
        ImGui::BeginDisabled(!mIsDirectionalLightEnabled);
        {
            Float3 direction = mDirectionalLight.GetDirection();
            ImGui::Text("Direction: %.3f %.3f %.3f", direction.x, direction.y, direction.z);

            Vector4 directionInView = Vector4(direction.x, direction.y, direction.z) * mRenderer.GetViewMatrix();
            vec3 directionInViewVec3 = { directionInView.GetFloat3().x, directionInView.GetFloat3().y, directionInView.GetFloat3().z };

            imguiGizmo::resizeAxesOf({ 0.7f, 0.8f, 0.8f });
            ImGui::gizmo3D("##Directional Light - Direction", directionInViewVec3);
            imguiGizmo::restoreAxesSize();

            directionInView = Vector4(directionInViewVec3.x, directionInViewVec3.y, directionInViewVec3.z, 0);
            Vector3 afterDirection = Vector3(directionInView * mRenderer.GetInverseViewMatrix());

            mDirectionalLight.SetDirection(afterDirection.GetFloat3());
        }
        {
            Float3 intensity = mDirectionalLight.GetIntensity();
            ImGui::DragFloat3("Intensity", &intensity.x, 0.1f);

            mDirectionalLight.SetIntensity(intensity);
        }
        ImGui::EndDisabled();

        mEnvironmentMapping.OnLoopImGUI();
    }

    void LightManager::BindLightShaderParameterList(RGBuilder& builder)
    {
        auto lightShaderParameterList = builder.CreateShaderParameterList<LightShaderParameterList>();
        lightShaderParameterList->Get()->isDirectionalLightEnabled = mIsDirectionalLightEnabled;
        lightShaderParameterList->Get()->directionalLightDirection = mDirectionalLight.GetDirection();
        lightShaderParameterList->Get()->directionalLightIntensity = mDirectionalLight.GetIntensity();
        builder.BindGlobalShaderParameterList(lightShaderParameterList);

        auto envMapShaderParameterList = builder.CreateShaderParameterList<EnvironmentMapLightShaderParameterList>();
        envMapShaderParameterList->Get()->diffuseIrradianceMap = mEnvironmentMapping.GetDiffuseIrradianceMap(builder);
        envMapShaderParameterList->Get()->integratedBRDFLUT = mEnvironmentMapping.GetIntegratedBRDFLUT(builder);
        envMapShaderParameterList->Get()->prefilterMap = mEnvironmentMapping.GetPrefilterMap(builder);
        envMapShaderParameterList->Get()->prefilterSampler = mEnvironmentMapping.GetPrefilterMapSampler();
        envMapShaderParameterList->Get()->prefilterMapMipLevels = mEnvironmentMapping.GetPrefilterMapMipLevels();
        builder.BindGlobalShaderParameterList(envMapShaderParameterList);
    }

    void LightManager::UnbindLightShaderParameterList(RGBuilder& builder)
    {
        builder.UnbindGlobalShaderParameterList<EnvironmentMapLightShaderParameterList>();
        builder.UnbindGlobalShaderParameterList<LightShaderParameterList>();
    }
} // namespace cube
