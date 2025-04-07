#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSDebug.h"

#include <iostream> // cout
#include <signal.h> // raise

#include "Checker.h"
#include "MacOS/MacOSString.h"

namespace cube
{
    namespace platform
    {
        PLATFORM_DEBUG_CLASS_DEFINITIONS(MacOSDebug)

        void MacOSDebug::PrintToDebugConsoleImpl(StringView str)
        {
            MacOSString osStr;
            String_ConvertAndAppend(osStr, str);

            std::cout << osStr << std::endl;
        }

        void MacOSDebug::ProcessFatalErrorImpl(StringView msg)
        {
            // TODO
            raise(SIGTRAP);
        }

        void MacOSDebug::ProcessFailedCheckImpl(const char* fileName, int lineNum, StringView formattedMsg)
        {
            // TODO: Fixed infinite loop
            // NOT_IMPLEMENTED();
            raise(SIGTRAP);
        }

        constexpr int MAX_NUM_FRAMES = 128;
        constexpr int MAX_NAME_LENGTH = 1024;

        String MacOSDebug::DumpStackTraceImpl(bool removeBeforeProjectFolderPath)
        {
            // NOT_IMPLEMENTED();
            return {};
        }

        bool MacOSDebug::IsDebuggerAttachedImpl()
        {
            NOT_IMPLEMENTED();
            return false;
        }

        void MacOSDebug::BreakDebugImpl()
        {
            raise(SIGTRAP);
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
