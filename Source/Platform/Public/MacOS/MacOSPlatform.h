#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "Platform.h"

#include <AppKit/AppKit.h>
#include <thread>

#include "Async.h"
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
        namespace internal
        {
            struct MacOSPlatformPrivateAccessor;
        } // namespace internal

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

            static void SetEngineInitializeFunctionImpl(std::function<void()> function);
            static void SetEngineShutdownFunctionImpl(std::function<void()> function);
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

            static CubeWindow* GetWindow();
            static bool IsMainWindowCreated();
            static void CloseMainWindow();

            static bool IsApplicationClosed() { return mIsApplicationClosed; }

            static void LastCleanup();

            static void ForceTerminateMainLoopThread();

            static void QueueEvent(std::function<void()> eventFunction);

        private:
            friend class CubeWindowDelegate;
            friend struct internal::MacOSPlatformPrivateAccessor;

            static void InitializeKeyCodeMapping();

            static void ReceiveModifierKeyEvent(NSEventModifierFlags flags);

            static void CreateMainMenu();

            static void MainLoop();

            static Array<KeyCode, MaxKeyCode> mKeyCodeMapping;

            static std::atomic<bool> mIsApplicationClosed;
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

            static std::function<void()> mEngineInitializeFunction;
            static std::function<void()> mEngineShutdownFunction;
            static Signal mRunMainLoopSignal;
            static std::thread mMainLoopThread;
            static bool mIsLoopStarted;
            static bool mIsLoopFinished;

            static std::mutex mEventQueueMutex;
            static Vector<std::function<void()>> mEventQueue;

            MacOSPlatform() = delete;
            ~MacOSPlatform() = delete;
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
