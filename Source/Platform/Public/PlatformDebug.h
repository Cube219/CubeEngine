#pragma once

#include "PlatformHeader.h"

#include "Checker.h"

namespace cube
{
    namespace platform
    {
        enum class PrintColorCategory
        {
            Default,
            Warning,
            Error
        };

        class CUBE_PLATFORM_EXPORT BaseDebug
        {
        public:
            static void PrintToDebugConsole(StringView str, PrintColorCategory colorCategory = PrintColorCategory::Default) { NOT_IMPLEMENTED() }

            static void ProcessFatalError(StringView msg) { NOT_IMPLEMENTED() }

            static void ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg) { NOT_IMPLEMENTED() }

            static String DumpStackTrace(bool removeBeforeProjectFolderPath = true) { NOT_IMPLEMENTED() return {}; }

            static bool IsDebuggerAttached() { NOT_IMPLEMENTED() return false; }
        };
    } // namespace platform
} // namespace cube

#if defined(CUBE_PLATFORM_MACOS)
#include "MacOS/MacOSDebug.h"
#elif defined(CUBE_PLATFORM_WINDOWS)
#include "Windows/WindowsDebug.h"
#endif
