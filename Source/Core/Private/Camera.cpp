#include "Camera.h"

#include "imguizmo_quat/imGuIZMOquat.h"
#include "imgui.h"

#include "Engine.h"
#include "Logger.h"
#include "MatrixUtility.h"
#include "Platform.h"

#include "Renderer/Renderer.h"

namespace cube
{
    float CameraSystem::mFOV;
    Vector3 CameraSystem::mPosition;
    float CameraSystem::mAxisXAngle;
    float CameraSystem::mAxisYAngle;
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
    float CameraSystem::mMovementSpeed = 5.0f;

    bool CameraSystem::mIsLeftMousePressed;
    bool CameraSystem::mIsRightMousePressed;
    bool CameraSystem::mIsMouseMoved;
    Uint32 CameraSystem::mLastMouseX;
    Uint32 CameraSystem::mLastMouseY;
    Uint32 CameraSystem::mCurrentMouseX;
    Uint32 CameraSystem::mCurrentMouseY;
    float CameraSystem::mMouseSpeed = 3.0f;

    void CameraSystem::Initialize()
    {
        mKeyDownEventFunc = platform::Platform::GetKeyDownEvent().AddListener([](KeyCode keyCode)
        {
            switch (keyCode)
            {
            case KeyCode::W:
                mIsForwardKeyPressed = true;
                break;
            case KeyCode::S:
                mIsBackwardKeyPressed = true;
                break;
            case KeyCode::A:
                mIsLeftKeyPressed = true;
                break;
            case KeyCode::D:
                mIsRightKeyPressed = true;
                break;
            case KeyCode::Q:
                mIsDownKeyPressed = true;
                break;
            case KeyCode::E:
                mIsUpKeyPressed = true;
                break;
            default:
                break;
            }
        });
        mKeyUpEventFunc = platform::Platform::GetKeyUpEvent().AddListener([](KeyCode keyCode)
        {
            switch (keyCode)
            {
            case KeyCode::W:
                mIsForwardKeyPressed = false;
                break;
            case KeyCode::S:
                mIsBackwardKeyPressed = false;
                break;
            case KeyCode::A:
                mIsLeftKeyPressed = false;
                break;
            case KeyCode::D:
                mIsRightKeyPressed = false;
                break;
            case KeyCode::Q:
                mIsDownKeyPressed = false;
                break;
            case KeyCode::E:
                mIsUpKeyPressed = false;
                break;
            default:
                break;
            }
        });
        mMouseDownEventFunc = platform::Platform::GetMouseDownEvent().AddListener([](MouseButton mouseButton)
        {
            switch (mouseButton)
            {
            case MouseButton::Left:
                mIsLeftMousePressed = true;
                break;
            case MouseButton::Right:
                mIsRightMousePressed = true;
                break;
            default:
                break;
            }
        });
        mMouseUpEventFunc = platform::Platform::GetMouseUpEvent().AddListener([](MouseButton mouseButton)
        {
            switch (mouseButton)
            {
            case MouseButton::Left:
                mIsLeftMousePressed = false;
                break;
            case MouseButton::Right:
                mIsRightMousePressed = false;
                break;
            default:
                break;
            }
        });
        mMousePositionFunc = platform::Platform::GetMousePositionEvent().AddListener([](Uint32 x, Uint32 y)
        {
            mIsMouseMoved = true;

            mLastMouseX = mCurrentMouseX;
            mLastMouseY = mCurrentMouseY;
            mCurrentMouseX = x;
            mCurrentMouseY = y;
        });

        mFOV = 45.0f;
        mAspectRatio = static_cast<float>(platform::Platform::GetWindowWidth()) / static_cast<float>(platform::Platform::GetWindowHeight());

        Reset();

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

    void CameraSystem::OnLoop(double deltaTime)
    {
        { // Keyboard
            float forwardDelta = (mIsForwardKeyPressed ? 1.0f : 0.0f) + (mIsBackwardKeyPressed ? -1.0f : 0.0f);
            float rightDelta = (mIsLeftKeyPressed ? -1.0f : 0.0f) + (mIsRightKeyPressed ? 1.0f : 0.0f);
            float upDelta = (mIsUpKeyPressed ? 1.0f : 0.0f) + (mIsDownKeyPressed ? -1.0f : 0.0f);

            Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
            Vector3 right = Vector3::Cross(mDirection, up);
            Vector3 forward = mDirection;

            Vector3 delta = (forward * forwardDelta) + (right * rightDelta) + (up * upDelta);
            if (Vector3(delta.Length()).GetFloat3().x > 0)
            {
                delta.Normalize();
            }
            delta *= static_cast<double>(mMovementSpeed) * deltaTime;

            mPosition += delta;
        }
        { // Mouse
            if (mIsRightMousePressed && mIsMouseMoved)
            {
                Int32 deltaX = static_cast<Int32>(mCurrentMouseX) - static_cast<Int32>(mLastMouseX);
                Int32 deltaY = static_cast<Int32>(mCurrentMouseY) - static_cast<Int32>(mLastMouseY);

                float scaledMouseSpeed = mMouseSpeed * 0.02f;
                mAxisXAngle = Math::Max(-89.9f, Math::Min(89.9f, mAxisXAngle - static_cast<float>(deltaY) * scaledMouseSpeed));
                mAxisYAngle = mAxisYAngle - static_cast<float>(deltaX) * scaledMouseSpeed;
                UpdateDirection();
            }
            mIsMouseMoved = false;
        }

        UpdateViewMatrix();
    }

    void CameraSystem::OnLoopImGUI()
    {
        ImGui::Begin("Camera");

        float position[3] = { mPosition.GetFloat3().x, mPosition.GetFloat3().y, mPosition.GetFloat3().z };
        ImGui::DragFloat3("Position", position, 0.2f);
        mPosition = { position[0], position[1], position[2] };

        vec3 dir = { mDirection.GetFloat3().x, mDirection.GetFloat3().y, mDirection.GetFloat3().z };
        ImGui::gizmo3D("##Dir1", dir, 100, imguiGizmo::mode3Axes|imguiGizmo::cubeAtOrigin);
        // mDirection = { dir.x, dir.y, dir.z };

        float direction[3] = { mDirection.GetFloat3().x, mDirection.GetFloat3().y, mDirection.GetFloat3().z };
        ImGui::DragFloat3("Direction", direction, 0.1f);

        UpdateViewMatrix();

        vec3 forward = { 0, 0, -1 };
        vec3 axis = normalize(cross(forward, dir));
        float angle = acos(dot(forward, dir));

        quat qRot = angleAxis(angle, axis);
        static vec3 PanDolly(0.f);
        ImGui::gizmo3D("##gizmo1", PanDolly, qRot /*, size,  mode */);

        ImGui::SliderFloat("Speed", &mMovementSpeed, 0.5f, 30.0f);
        ImGui::SliderFloat("Mouse Speed", &mMouseSpeed, 0.5f, 10.0f);

        if (ImGui::Button("Reset"))
        {
            Reset();
        }

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

    void CameraSystem::UpdateDirection()
    {
        mDirection = Vector3(0.0f, 0.0f, -1.0f) * MatrixUtility::GetRotationX(Math::Deg2Rad(mAxisXAngle)) * MatrixUtility::GetRotationY(Math::Deg2Rad(mAxisYAngle));
    }

    void CameraSystem::Reset()
    {
        mPosition = { 5.0f, 5.0f, 5.0f };
        mAxisXAngle = -30.0f;
        mAxisYAngle = 45.0f;

        UpdateDirection();
        UpdateViewMatrix();
    }
} // namespace cube
