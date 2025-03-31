#pragma once

#include "CoreHeader.h"

#include "Event.h"
#include "Vector.h"

namespace cube
{
    enum class MouseButton;
    enum class KeyCode;

    enum class CameraMovementMode
    {
        Preview,
    };

    class CameraSystem
    {
    public:
        CameraSystem() = delete;
        ~CameraSystem() = delete;

        static void Initialize();
        static void Shutdown();

        static void OnLoop(double deltaTime);
        static void OnLoopImGUI();
        static void OnResize(Uint32 width, Uint32 height);


        static float GetFOV() { return mFOV; }
        static Vector3 GetPosition() { return mPosition; }
        static Vector3 GetDirection() { return mDirection; }

    private:
        static void UpdateViewMatrix();
        static void UpdatePerspectiveMatrix();
        static void UpdateDirection();
        static void Reset();

        static float mFOV;
        static Vector3 mPosition;
        static float mAxisXAngle;
        static float mAxisYAngle;
        static Vector3 mDirection;
        static float mAspectRatio;

        static EventFunction<void(KeyCode)> mKeyDownEventFunc;
        static EventFunction<void(KeyCode)> mKeyUpEventFunc;
        static EventFunction<void(MouseButton)> mMouseDownEventFunc;
        static EventFunction<void(MouseButton)> mMouseUpEventFunc;
        static EventFunction<void(Uint32, Uint32)> mMousePositionFunc;

        static bool mIsForwardKeyPressed;
        static bool mIsBackwardKeyPressed;
        static bool mIsLeftKeyPressed;
        static bool mIsRightKeyPressed;
        static bool mIsUpKeyPressed;
        static bool mIsDownKeyPressed;
        static float mMovementSpeed;

        static bool mIsLeftMousePressed;
        static bool mIsRightMousePressed;
        static bool mIsMouseMoved;
        static Uint32 mLastMouseX;
        static Uint32 mLastMouseY;
        static Uint32 mCurrentMouseX;
        static Uint32 mCurrentMouseY;
        static float mMouseSpeed;
    };
} // namespace cube
