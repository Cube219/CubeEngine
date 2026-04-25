#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "PlatformDebug.h"

#ifndef CUBE_MACOS_USE_LOGGER_WINDOW
#define CUBE_MACOS_USE_LOGGER_WINDOW CUBE_DEBUG
#endif

#if CUBE_MACOS_USE_LOGGER_WINDOW

#ifdef __OBJC__
#include <AppKit/AppKit.h>
#include <atomic>
#include <mutex>
#include <thread>

@class CubeLoggerSubprocessWindow;
@class CubeLoggerSubprocessTextView;
@class CubeLoggerSubprocessWindowDelegate;
@class CubeLoggerSubprocessAppDelegate;
#endif // __OBJC__

namespace cube
{
    namespace platform
    {
        // Owns the logger window subprocess: parent-side spawn/IPC, and child-side entry point.
        // The same CE-Main binary serves both roles; the child branch is taken when the process
        // is launched with --logger-subprocess.
        class MacOSLoggerSubprocess
        {
        public:
#ifdef __OBJC__
            // === Parent side ===
            static void Spawn();
            static void Shutdown();

            static bool IsAlive() { return mIsAlive.load(); }
            static void Send(StringView str, PrintColorCategory colorCategory);

            // === Child side ===
            static void Run();
            static bool IsPressAnyKeyActivated() { return mIsPressAnyKeyActivated.load(); }

            static void OnAppWillTerminate();

        private:
            // === Parent-side state ===
            static NSTask* mLoggerTask;
            static NSFileHandle* mLoggerWriteHandle;
            static std::mutex mLoggerWriteMutex;
            static std::atomic<bool> mIsAlive;

            // === Child-side state and helpers ===
            static void AppendLogString(NSString* text, PrintColorCategory colorCategory);
            static bool ReadExact(int fd, void* buf, Uint64 len);
            static void ReaderThreadMain();
            static void CreateWindow();

            static CubeLoggerSubprocessWindow* mWindow;
            static CubeLoggerSubprocessTextView* mTextView;
            static CubeLoggerSubprocessWindowDelegate* mWindowDelegate;
            static CubeLoggerSubprocessAppDelegate* mAppDelegate;
            static std::atomic<bool> mIsPressAnyKeyActivated;
            static std::thread mReaderThread;
#endif // __OBJC__

            MacOSLoggerSubprocess() = delete;
            ~MacOSLoggerSubprocess() = delete;
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_MACOS_USE_LOGGER_WINDOW

#endif // CUBE_PLATFORM_MACOS
