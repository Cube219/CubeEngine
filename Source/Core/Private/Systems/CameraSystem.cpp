#include "CameraSystem.h"

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
    Matrix CameraSystem::mRotation;
    float CameraSystem::mAspectRatio;

    EventFunction<void(KeyCode)> CameraSystem::mKeyDownEventFunc;
    EventFunction<void(KeyCode)> CameraSystem::mKeyUpEventFunc;
    EventFunction<void(MouseButton)> CameraSystem::mMouseDownEventFunc;
    EventFunction<void(MouseButton)> CameraSystem::mMouseUpEventFunc;
    EventFunction<void(Int32, Int32)> CameraSystem::mMousePositionEventFunc;
    EventFunction<void(int)> CameraSystem::mMouseWheelEventFunc;
    EventFunction<void(platform::WindowActivatedState)> CameraSystem::mWindowActivateEventFunc;

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
    Int32 CameraSystem::mLastMouseX;
    Int32 CameraSystem::mLastMouseY;
    Int32 CameraSystem::mCurrentMouseX;
    Int32 CameraSystem::mCurrentMouseY;
    bool CameraSystem::mIsMouseLocked = false;
    Int32 CameraSystem::mLockedMouseX;
    Int32 CameraSystem::mLockedMouseY;
    float CameraSystem::mMouseSpeed = 3.0f;

    bool CameraSystem::mIsMouseWheelMoved;
    int CameraSystem::mMouseWheelDelta;

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
            if (ImGui::GetIO().WantCaptureMouse)
            {
                return;
            }
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
        mMousePositionEventFunc = platform::Platform::GetMousePositionEvent().AddListener([](Int32 x, Int32 y)
        {
            if (x == mCurrentMouseX && y == mCurrentMouseY)
            {
                return;
            }
            mIsMouseMoved = true;

            mLastMouseX = mCurrentMouseX;
            mLastMouseY = mCurrentMouseY;
            mCurrentMouseX = x;
            mCurrentMouseY = y;
        });
        mMouseWheelEventFunc = platform::Platform::GetMouseWheelEvent().AddListener([](int delta)
        {
            if (ImGui::GetIO().WantCaptureMouse)
            {
                return;
            }
            mIsMouseWheelMoved = true;

            mMouseWheelDelta = delta;
        });
        mWindowActivateEventFunc = platform::Platform::GetActivatedEvent().AddListener([](platform::WindowActivatedState state)
        {
            if (state == platform::WindowActivatedState::Inactive)
            {
                mIsForwardKeyPressed = false;
                mIsBackwardKeyPressed = false;
                mIsLeftKeyPressed = false;
                mIsRightKeyPressed = false;
                mIsUpKeyPressed = false;
                mIsDownKeyPressed = false;
                mIsLeftMousePressed = false;
                mIsRightMousePressed = false;
            }
        });

        mFOV = 45.0f;
        mAspectRatio = static_cast<float>(platform::Platform::GetWindowWidth()) / static_cast<float>(platform::Platform::GetWindowHeight());

        Reset();

        UpdateViewMatrix();
        UpdatePerspectiveMatrix();
    }

    void CameraSystem::Shutdown()
    {
        platform::Platform::GetActivatedEvent().RemoveListener(mWindowActivateEventFunc);
        platform::Platform::GetMouseWheelEvent().RemoveListener(mMouseWheelEventFunc);
        platform::Platform::GetMousePositionEvent().RemoveListener(mMousePositionEventFunc);
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
            if (mIsRightMousePressed && !mIsMouseLocked)
            {
                mLockedMouseX = mCurrentMouseX;
                mLockedMouseY = mCurrentMouseY;
                platform::Platform::HideCursor();
            }
            else if (!mIsRightMousePressed && mIsMouseLocked)
            {
                platform::Platform::ShowCursor();
            }
            mIsMouseLocked = mIsRightMousePressed;

            if (mIsMouseLocked && mIsMouseMoved)
            {
                Int32 deltaX = mCurrentMouseX - mLockedMouseX;
                Int32 deltaY = mCurrentMouseY - mLockedMouseY;

                float scaledMouseSpeed = mMouseSpeed * 0.02f;
                mAxisXAngle = Math::Max(-89.9f, Math::Min(89.9f, mAxisXAngle - static_cast<float>(deltaY) * scaledMouseSpeed));
                mAxisYAngle = mAxisYAngle - static_cast<float>(deltaX) * scaledMouseSpeed;
                UpdateDirection();

                platform::Platform::MoveCursor(mLockedMouseX, mLockedMouseY);
                mIsMouseMoved = false;
            }

            if (mIsMouseWheelMoved)
            {
                mPosition += mDirection * static_cast<double>(mMouseWheelDelta) * deltaTime;

                mIsMouseWheelMoved = false;
            }
        }

        UpdateViewMatrix();
    }

    void CameraSystem::OnLoopImGUI()
    {
        ImGui::Begin("Camera");

        // Position
        float position[3] = { mPosition.GetFloat3().x, mPosition.GetFloat3().y, mPosition.GetFloat3().z };

        ImGui::DragFloat3("Position", position, 0.1f);

        mPosition = { position[0], position[1], position[2] };

        // Rotation
        float direction[3] = { mDirection.GetFloat3().x, mDirection.GetFloat3().y, mDirection.GetFloat3().z };
        ImGui::PushItemFlag(ImGuiItemFlags_ReadOnly, true);

        vec3 directionVec3 = { -direction[0], -direction[1], -direction[2] };
        imguiGizmo::resizeAxesOf({ 0.7f, 0.8f, 0.8f });
        imguiGizmo::setDirectionColor(ImVec4(0.0f,1.0f,1.0f,1.0f));
        ImGui::gizmo3D("##Direction Vector", directionVec3);
        imguiGizmo::restoreAxesSize();
        imguiGizmo::restoreDirectionColor();

        Matrix rotationMatrixT = mRotation.Transposed();
        Float3 row0 = Vector3(rotationMatrixT.GetRow(0)).GetFloat3();
        Float3 row1 = Vector3(rotationMatrixT.GetRow(1)).GetFloat3();
        Float3 row2 = Vector3(rotationMatrixT.GetRow(2)).GetFloat3();
        quat qRotation = quat(mat3(
            row0.x, row0.y, row0.z,
            row1.x, row1.y, row1.z,
            row2.x, row2.y, row2.z
        ));
        imguiGizmo::resizeAxesOf({ 1.0f, 1.8f, 1.5f });
        imguiGizmo::resizeSolidOf(1.5f);
        ImGui::SameLine();
        ImGui::gizmo3D("##gizmo1", qRotation);
        imguiGizmo::restoreAxesSize();
        imguiGizmo::restoreSolidSize();

        ImGui::DragFloat3("Direction", direction, 0.1f);

        ImGui::PopItemFlag();

        UpdateViewMatrix();

        // Speed and reset
        ImGui::NewLine();
        ImGui::SliderFloat("Speed", &mMovementSpeed, 0.5f, 100.0f);
        ImGui::SliderFloat("Mouse Speed", &mMouseSpeed, 0.5f, 10.0f);

        ImGui::NewLine();
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
        Engine::GetRenderer()->SetPerspectiveMatrix(Math::Deg2Rad(mFOV), mAspectRatio, 10000.0f, 0.1f);
    }

    void CameraSystem::UpdateDirection()
    {
        mRotation = MatrixUtility::GetRotationX(Math::Deg2Rad(mAxisXAngle)) * MatrixUtility::GetRotationY(Math::Deg2Rad(mAxisYAngle));
        mDirection = Vector3(Vector4(0.0f, 0.0f, -1.0f, 0.0f) * mRotation);
    }

    void CameraSystem::Reset()
    {
        mPosition = { 10.0f, 10.0f, 10.0f };
        mAxisXAngle = -30.0f;
        mAxisYAngle = 45.0f;

        UpdateDirection();
        UpdateViewMatrix();
    }
} // namespace cube
