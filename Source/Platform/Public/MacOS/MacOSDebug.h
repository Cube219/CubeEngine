#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "PlatformDebug.h"

#include "MacOSString.h"

namespace cube
{
    namespace platform
    {
        class MacOSDebug : public PlatformDebug
        {
        public:
            static void PrintToDebugConsoleImpl(StringView str);

            static void ProcessFatalErrorImpl(StringView msg);

            static void ProcessFailedCheckImpl(const char* fileName, int lineNum, StringView formattedMsg);

            static String DumpStackTraceImpl(bool removeBeforeProjectFolderPath);

            static bool IsDebuggerAttachedImpl();
            static void BreakDebugImpl();
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
