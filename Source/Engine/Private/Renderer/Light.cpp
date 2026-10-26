#include "Light.h"

#include "imguizmo_quat/imGuIZMOquat.h"
#include "imgui.h"

#include "Engine.h"
#include "Renderer.h"

namespace cube
{
    void Light::SetEnable(bool enable)
    {
        mEnabled = enable;
    }

    void Light::SetPosition(const Float3& newPosition)
    {
        mPosition = newPosition;
    }

    void Light::SetDirection(const Float3& newDirection)
    {
        mDirection = newDirection;
    }

    void Light::SetIntensity(const Float3& newIntensity)
    {
        mIntensity = newIntensity;
    }

    void DirectionalLight::OnLoopImGUIContent()
    {
        Renderer* renderer = Engine::GetRenderer();

        ImGui::BeginDisabled(!mEnabled);
        {
            Float3 direction = GetDirection();
            ImGui::Text("Direction: %.3f %.3f %.3f", mDirection.x, mDirection.y, mDirection.z);

            Vector4 directionInView = Vector4(direction.x, direction.y, direction.z) * renderer->GetViewMatrix();
            vec3 directionInViewVec3 = { directionInView.GetFloat3().x, directionInView.GetFloat3().y, directionInView.GetFloat3().z };

            imguiGizmo::resizeAxesOf({ 0.7f, 0.8f, 0.8f });
            ImGui::gizmo3D("##Directional Light - Direction", directionInViewVec3);
            imguiGizmo::restoreAxesSize();

            directionInView = Vector4(directionInViewVec3.x, directionInViewVec3.y, directionInViewVec3.z, 0);
            Vector3 afterDirection = Vector3(directionInView * renderer->GetInverseViewMatrix());

            SetDirection(afterDirection.GetFloat3());
        }
        {
            Float3 intensity = GetIntensity();
            ImGui::DragFloat3("Intensity", &intensity.x, 0.1f);

            SetIntensity(intensity);
        }
        ImGui::EndDisabled();
    }

    void PointLight::OnLoopImGUIContent()
    {
        ImGui::BeginDisabled(!mEnabled);
        {
            Float3 position = GetPosition();
            ImGui::DragFloat3("Position", &position.x, 0.1f);
            SetPosition(position);
        }
        {
            Float3 intensity = GetIntensity();
            ImGui::DragFloat3("Intensity", &intensity.x, 0.1f, 0.0f, FLT_MAX);
            SetIntensity(intensity);
        }
        ImGui::EndDisabled();
    }
} // namespace cube
