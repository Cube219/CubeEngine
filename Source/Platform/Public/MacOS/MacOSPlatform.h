#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "Platform.h"

#include <AppKit/AppKit.h>
#include <thread>

#include "KeyCode.h"
#include "MacOSString.h"

@interface CubeWindow : NSWindow

@end

@interface CubeAppDelegate : NSObject <NSApplicationDelegate>

@end

@interface CubeWindowDelegate : NSObject <NSWindowDelegate>

@end

namespace cube
{
    namespace platform
    {
        enum class MacOSEventType
        {
            ResizeWindow,
            MoveWindow,
            KeyDown,
            KeyUp,
            MouseDown,
            MouseUp,
            MouseWheel,
            MousePosition
        };
        struct MacOSKeyDownEvent
        {
            Uint32 keyCode;
        };
        struct MacOSKeyUpEvent
        {
            Uint32 keyCode;
        };
        struct MacOSMouseDownEvent
        {
            MouseButton button;
        };
        struct MacOSMouseUpEvent
        {
            MouseButton button;
        };
        struct MacOSMouseWheelEvent
        {
            int delta;
        };
        struct MacOSResizeWindowEvent
        {
            Uint32 newWidth;
            Uint32 newHeight;
        };
        struct MacOSMoveWindowEvent
        {
            Int32 newX;
            Int32 newY;
        };
        struct MacOSMousePositionEvent
        {
            Int32 x;
            Int32 y;
        };

        class MacOSPlatform : public Platform
        {
        public:
            static void InitializeImpl();
            static void ShutdownImpl();

            static void InitWindowImpl(StringView title, Uint32 width, Uint32 height, Int32 posX, Int32 posY);
            static void ShowWindowImpl();
            static void ChangeWindowTitleImpl(StringView title);

            static void* AllocateImpl(Uint64 size);
            static void FreeImpl(void* ptr);
            static void* AllocateAlignedImpl(Uint64 size, Uint64 alignment);
            static void FreeAlignedImpl(void* ptr);

            static void StartLoopImpl();
            static void FinishLoopImpl();
            static void SleepImpl(float timeSec);

            static void ShowCursorImpl();
            static void HideCursorImpl();
            static void MoveCursorImpl(int x, int y);
            static void GetCursorPosImpl(int& x, int& y);

            static Uint32 GetWindowWidthImpl();
            static Uint32 GetWindowHeightImpl();
            static Int32 GetWindowPositionXImpl();
            static Int32 GetWindowPositionYImpl();

            static SharedPtr<DLib> LoadDLibImpl(StringView path);

            static CubeWindow* GetWindow() { return mWindow; }
            static bool IsMainWindowCreated();
            static void CloseMainWindow();

            static bool IsApplicationClosed() { return mIsApplicationClosed; }

            static void Cleanup();

            static void ForceTerminateMainLoopThread();

            static void DispatchEvent(MacOSEventType type, void* pData);

        private:
            friend class CubeWindowDelegate;

            static void InitializeKeyCodeMapping();

            static void ReceiveModifierKeyEvent(NSEventModifierFlags flags);

            static void CreateMainMenu();

            static void MainLoop();

            static Array<KeyCode, MaxKeyCode> mKeyCodeMapping;

            static bool mIsApplicationClosed;
            static CubeWindow* mWindow;
            static CubeAppDelegate* mAppDelegate;
            static CubeWindowDelegate* mWindowDelegate;
            static Uint32 mWindowWidth;
            static Uint32 mWindowHeight;
            static Int32 mWindowPositionX;
            static Int32 mWindowPositionY;

            static bool mIsMouseHidden;
            static Int32 mMousePositionX;
            static Int32 mMousePositionY;

            static id mModifierEventHandler;
            static bool mIsCapsLockKeyPressed;
            static bool mIsShiftKeyPressed;
            static bool mIsControlKeyPressed;
            static bool mIsOptionKeyPressed;
            static bool mIsCommandKeyPressed;
            static bool mIsFunctionKeyPressed;


            static std::thread mMainLoopThread;
            static bool mIsLoopStarted;
            static bool mIsLoopFinished;

            MacOSPlatform() = delete;
            ~MacOSPlatform() = delete;
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
