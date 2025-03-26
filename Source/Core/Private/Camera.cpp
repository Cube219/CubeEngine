#include "Camera.h"

#include "imguizmo_quat/imGuIZMOquat.h"
#include "imgui.h"

#include "Engine.h"
#include "MatrixUtility.h"
#include "Platform.h"

#include "Renderer/Renderer.h"

namespace cube
{
    float CameraSystem::mFOV;
    Vector3 CameraSystem::mPosition;
    Vector3 CameraSystem::mDirection;
    float CameraSystem::mAspectRatio;

    EventFunction<void(KeyCode)> CameraSystem::mKeyDownEventFunc;
    EventFunction<void(KeyCode)> CameraSystem::mKeyUpEventFunc;
    EventFunction<void(MouseButton)> CameraSystem::mMouseDownEventFunc;
    EventFunction<void(MouseButton)> CameraSystem::mMouseUpEventFunc;
    EventFunction<void(Uint32, Uint32)> CameraSystem::mMousePositionFunc;

    bool CameraSystem::mIsForwardKeyPressed = false;
    bool CameraSystem::mIsBackwardKeyPressed = false;
    bool CameraSystem::mIsLeftKeyPressed = false;
    bool CameraSystem::mIsRightKeyPressed = false;
    bool CameraSystem::mIsUpKeyPressed = false;
    bool CameraSystem::mIsDownKeyPressed = false;

    Uint32 CameraSystem::mLastMouseX;
    Uint32 CameraSystem::mLastMouseY;

    void CameraSystem::Initialize()
    {
        mKeyDownEventFunc = platform::Platform::GetKeyDownEvent().AddListener([](KeyCode keyCode)
        {
            
        });
        mKeyUpEventFunc = platform::Platform::GetKeyUpEvent().AddListener([](KeyCode keyCode)
        {
            
        });
        mMouseDownEventFunc = platform::Platform::GetMouseDownEvent().AddListener([](MouseButton mouseButton)
        {
            
        });
        mMouseUpEventFunc = platform::Platform::GetMouseUpEvent().AddListener([](MouseButton mouseButton)
        {
            
        });
        mMousePositionFunc = platform::Platform::GetMousePositionEvent().AddListener([](Uint32 x, Uint32 y)
        {
            
        });

        mFOV = 45.0f;
        mPosition = { 0.0f, 0.0f, 10.0f };
        mDirection = -mPosition.Normalized();

        mAspectRatio = static_cast<float>(platform::Platform::GetWindowWidth()) / static_cast<float>(platform::Platform::GetWindowHeight());

        UpdateViewMatrix();
        UpdatePerspectiveMatrix();
    }

    void CameraSystem::Shutdown()
    {
        platform::Platform::GetMousePositionEvent().RemoveListener(mMousePositionFunc);
        platform::Platform::GetMouseUpEvent().RemoveListener(mMouseUpEventFunc);
        platform::Platform::GetMouseDownEvent().RemoveListener(mMouseDownEventFunc);
        platform::Platform::GetKeyUpEvent().RemoveListener(mKeyUpEventFunc);
        platform::Platform::GetKeyDownEvent().RemoveListener(mKeyDownEventFunc);
    }

    void CameraSystem::OnLoop()
    {
    }

    void CameraSystem::OnLoopImGUI()
    {
        ImGui::Begin("Camera");

        static float position[3] = { mPosition.GetFloat3().x, mPosition.GetFloat3().y, mPosition.GetFloat3().z };
        ImGui::DragFloat3("Position", position, 0.2f);
        mPosition = { position[0], position[1], position[2] };

        static vec3 dir = { mDirection.GetFloat3().x, mDirection.GetFloat3().y, mDirection.GetFloat3().z };
        ImGui::gizmo3D("##Dir1", dir, 100, imguiGizmo::mode3Axes|imguiGizmo::cubeAtOrigin);
        mDirection = { dir.x, dir.y, dir.z };

        float direction[3] = { mDirection.GetFloat3().x, mDirection.GetFloat3().y, mDirection.GetFloat3().z };
        ImGui::DragFloat3("Direction", direction, 0.1f);

        UpdateViewMatrix();

        vec3 forward = { 0, 0, -1 };
        vec3 axis = normalize(cross(forward, dir));
        float angle = acos(dot(forward, dir));

        quat qRot = angleAxis(angle, axis);
        static vec3 PanDolly(0.f);
        ImGui::gizmo3D("##gizmo1", PanDolly, qRot /*, size,  mode */);


        ImGui::End();
    }

    void CameraSystem::OnResize(Uint32 width, Uint32 height)
    {
        mAspectRatio = static_cast<float>(width) / static_cast<float>(height);
        UpdatePerspectiveMatrix();
    }

    void CameraSystem::UpdateViewMatrix()
    {
        Engine::GetRenderer()->SetViewMatrix(mPosition, mPosition + mDirection, { 0.0f, 1.0f, 0.0f });
    }

    void CameraSystem::UpdatePerspectiveMatrix()
    {
        // Reverse Z
        Engine::GetRenderer()->SetPerspectiveMatrix(Math::Deg2Rad(mFOV), mAspectRatio, 100.0f, 0.1f);
    }
} // namespace cube
