#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "Platform.h"

#ifdef __OBJC__
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
#endif // __OBJC__

namespace cube
{
    namespace platform
    {
#ifdef __OBJC__
        namespace internal
        {
            struct MacOSPlatformPrivateAccessor;
        } // namespace internal
#endif // __OBJC__

        class MacOSPlatform : public BasePlatform
        {
            // === Base member functions ===
        public:
            static void Initialize();
            static void Shutdown();

            static void InitWindow(StringView title, Uint32 width, Uint32 height, Int32 posX, Int32 posY);
            static void ShowWindow();
            static void ChangeWindowTitle(StringView title);

            static void* Allocate(Uint64 size);
            static void Free(void* ptr);
            static void* AllocateAligned(Uint64 size, Uint64 alignment);
            static void FreeAligned(void* ptr);

            static void SetEngineInitializeFunction(std::function<void()> function);
            static void SetEngineShutdownFunction(std::function<void()> function);
            static void SetPostLoopMainThreadFunction(std::function<void()> function);
            static void StartLoop();
            static void FinishLoop();
            static void Sleep(float timeSec);

            static void ShowCursor();
            static void HideCursor();
            static void MoveCursor(int x, int y);
            static void GetCursorPos(int& x, int& y);

            static Uint32 GetWindowWidth();
            static Uint32 GetWindowHeight();
            static Int32 GetWindowPositionX();
            static Int32 GetWindowPositionY();

            static SharedPtr<MacOSDLib> LoadDLib(const FilePath& path);
            // === Base member functions ===

#ifdef __OBJC__
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
            static std::function<void()> mPostLoopMainThreadFunction;
            static Signal mRunMainLoopSignal;
            static std::thread mMainLoopThread;
            static bool mIsLoopStarted;
            static bool mIsLoopFinished;

            static std::mutex mEventQueueMutex;
            static Vector<std::function<void()>> mEventQueue;

            MacOSPlatform() = delete;
            ~MacOSPlatform() = delete;
#endif // __OBJC__
        };
        using Platform = MacOSPlatform;
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
