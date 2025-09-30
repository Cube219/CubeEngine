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
            static void PrintToDebugConsoleImpl(StringView str, PrintColorCategory colorCategory);

            static void ProcessFatalErrorImpl(StringView msg);

            static void ProcessFailedCheckImpl(const char* fileName, int lineNum, StringView formattedMsg);

            static String DumpStackTraceImpl(bool removeBeforeProjectFolderPath);

            static bool IsDebuggerAttachedImpl();

            static void CreateAndShowLoggerWindow();

        private:
            static void ShowDebugMessageBox(const WindowsString& title, const WindowsString& msg);
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
