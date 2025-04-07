#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "Platform.h"

#import <Cocoa/Cocoa.h>
#include <thread>

#include "MacOSString.h"

@interface CubeAppDelegate : NSObject <NSApplicationDelegate>

@end

namespace cube
{
    namespace platform
    {
        class MacOSPlatform : public Platform
        {
        public:
            static void InitializeImpl();
            static void ShutdownImpl();

            static void InitWindowImpl(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY);
            static void ShowWindowImpl();

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
            static Uint32 GetWindowPositionXImpl();
            static Uint32 GetWindowPositionYImpl();

            static SharedPtr<DLib> LoadDLibImpl(StringView path);

        private:
            static void CreateMainMenu();
            static void MainLoop();

            static NSWindow* mWindow;
            static CubeAppDelegate* mDelegate;
#if CUBE_DEBUG
            static NSTask* mDebugConsoleTerminalTask;
#endif

            static std::thread mMainLoopThread;
            static bool mIsFinished;

            MacOSPlatform() = delete;
            ~MacOSPlatform() = delete;
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
