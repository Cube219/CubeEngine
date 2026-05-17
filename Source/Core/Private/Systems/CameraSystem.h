#pragma once

#include "CoreHeader.h"

#include "Event.h"
#include "Matrix.h"
#include "Platform.h"
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
        static void OnLoopImGUIContent();
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
        static Float2 mAxisAngles;
        static Vector3 mDirection;
        static Matrix mRotation;
        static float mAspectRatio;

        static EventFunction<void(KeyCode)> mKeyDownEventFunc;
        static EventFunction<void(KeyCode)> mKeyUpEventFunc;
        static EventFunction<void(MouseButton)> mMouseDownEventFunc;
        static EventFunction<void(MouseButton)> mMouseUpEventFunc;
        static EventFunction<void(Int32, Int32)> mMousePositionEventFunc;
        static EventFunction<void(int)> mMouseWheelEventFunc;
        static EventFunction<void(platform::WindowActivatedState)> mWindowActivateEventFunc;

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
        static Int2 mLastMousePos;
        static Int2 mCurrentMousePos;
        static bool mIsMouseLocked;
        static Int2 mLockedMousePos;
        static float mMouseSpeed;

        static bool mIsMouseWheelMoved;
        static int mMouseWheelDelta;
    };
} // namespace cube
