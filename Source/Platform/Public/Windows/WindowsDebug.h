#pragma once
#ifdef CUBE_PLATFORM_WINDOWS

#include "PlatformHeader.h"
#include "PlatformDebug.h"

#include "WindowsString.h"

namespace cube
{
    namespace platform
    {
        class CUBE_PLATFORM_EXPORT WindowsDebug : public BaseDebug
        {
            // === Base member functions ===
        public:
            static void PrintToDebugConsole(StringView str, PrintColorCategory colorCategory = PrintColorCategory::Default);

            static void ProcessFatalError(StringView msg);

            static void ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg);

            static String DumpStackTrace(bool removeBeforeProjectFolderPath = true);

            static bool IsDebuggerAttached();
            // === Base member functions ===

        public:
            static void CreateAndShowLoggerWindow();

        private:
            static void ShowDebugMessageBox(const WindowsString& title, const WindowsString& msg);
        };
        using Debug = WindowsDebug;
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
