#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "PlatformDebug.h"

#ifdef __OBJC__
#include <AppKit/AppKit.h>
#include <atomic>
#include <mutex>
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
            static bool IsAlive() { return mIsAlive.load(); }
            static void Spawn();
            static void Shutdown();
            static void Send(StringView str, PrintColorCategory colorCategory);

            // === Child side ===
            // Enters NSApp run loop and calls exit(0) on terminate. Never returns.
            static void Run();
            // Detach the reader thread and call [NSApp terminate:nil]. Use from Obj-C delegates so
            // exit() (called inside terminate:) does not invoke ~thread on a joinable thread.
            static void TerminateFromUI();

        private:
            static NSTask* mLoggerTask;
            static NSFileHandle* mLoggerWriteHandle;
            static std::mutex mLoggerWriteMutex;
            static std::atomic<bool> mIsAlive;
#endif // __OBJC__

            MacOSLoggerSubprocess() = delete;
            ~MacOSLoggerSubprocess() = delete;
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
