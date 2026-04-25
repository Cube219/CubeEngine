#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "PlatformDebug.h"

#ifdef __OBJC__
#include <AppKit/AppKit.h>

#include "MacOSString.h"
#endif // __OBJC__

namespace cube
{
    namespace platform
    {
        class MacOSDebug : public BaseDebug
        {
            // === Base member functions ===
        public:
            static void PrintToDebugConsole(StringView str, PrintColorCategory colorCategory = PrintColorCategory::Default);

            static void ProcessFatalError(StringView msg);

            static void ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg);

            static String DumpStackTrace(bool removeBeforeProjectFolderPath = true);

            static bool IsDebuggerAttached();

            static void SetTestMode(bool enable);
            static bool IsTestMode();
            // === Base member functions ===

#ifdef __OBJC__
        private:
            static void ShowDebugMessageAlert(StringView title, StringView msg);
            static void ShowDebugMessageAlert_MainThread(StringView title, StringView msg);

            static bool mIsDebugBreakSetFromDebugMessageAlert;
            static bool mIsForceTerminationSetFromDebugMessageAlert;

            static bool mIsTestMode;
#endif // __OBJC__
        };
        using Debug = MacOSDebug;
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
