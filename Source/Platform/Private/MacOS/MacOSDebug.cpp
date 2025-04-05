#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSDebug.h"

#include "Checker.h"

namespace cube
{
    namespace platform
    {
        PLATFORM_DEBUG_CLASS_DEFINITIONS(MacOSDebug)

        void MacOSDebug::PrintToDebugConsoleImpl(StringView str)
        {
            NOT_IMPLEMENTED();
        }

        void MacOSDebug::ProcessFatalErrorImpl(StringView msg)
        {
            NOT_IMPLEMENTED();
        }

        void MacOSDebug::ProcessFailedCheckImpl(const char* fileName, int lineNum, StringView formattedMsg)
        {
            NOT_IMPLEMENTED();
        }

        constexpr int MAX_NUM_FRAMES = 128;
        constexpr int MAX_NAME_LENGTH = 1024;

        String MacOSDebug::DumpStackTraceImpl(bool removeBeforeProjectFolderPath)
        {
            NOT_IMPLEMENTED();
            return {};
        }

        bool MacOSDebug::IsDebuggerAttachedImpl()
        {
            NOT_IMPLEMENTED();
            return false;
        }

        void MacOSDebug::BreakDebugImpl()
        {
            NOT_IMPLEMENTED();
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
