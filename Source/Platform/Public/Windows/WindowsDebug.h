#pragma once
#ifdef CUBE_PLATFORM_WINDOWS

#include "PlatformHeader.h"

#include "PlatformDebug.h"

#include "WindowsString.h"

namespace cube
{
    namespace platform
    {
        class WindowsDebug : public PlatformDebug
        {
        public:
            static void PrintToDebugConsoleImpl(StringView str);

            static void ProcessFatalErrorImpl(StringView msg);

            static void ProcessFailedCheckImpl(const char* fileName, int lineNum, StringView formattedMsg);

            static String DumpStackTraceImpl();

            static bool IsDebuggerAttachedImpl();
            static void BreakDebugImpl();

        private:
            static void ShowDebugMessageBox(const WindowsString& title, const WindowsString& msg);
        };
        PLATFORM_DEBUG_CLASS_DEFINITIONS(WindowsDebug)
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
