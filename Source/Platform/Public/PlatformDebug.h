#pragma once

#include "PlatformHeader.h"

#include "Format.h"

namespace cube
{
    namespace platform
    {
        class CUBE_PLATFORM_EXPORT PlatformDebug
        {
        public:
            static void PrintToDebugConsole(StringView str);

            static void ProcessFatalError(StringView msg);

            static void ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg);

            static String DumpStackTrace();

            static bool IsDebuggerAttached();
            static void BreakDebug();
        };

#define PLATFORM_DEBUG_CLASS_DEFINITIONS(ChildClass) \
        inline void PlatformDebug::PrintToDebugConsole(StringView str) \
        { \
            ChildClass::PrintToDebugConsoleImpl(str); \
        } \
        \
        inline void PlatformDebug::ProcessFatalError(StringView msg) \
        { \
            ChildClass::ProcessFatalErrorImpl(msg); \
        } \
        inline void PlatformDebug::ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg) \
        { \
            ChildClass::ProcessFailedCheckImpl(fileName, lineNum, formattedMsg); \
        } \
        inline String PlatformDebug::DumpStackTrace() \
        { \
            return ChildClass::DumpStackTraceImpl(); \
        } \
        inline bool PlatformDebug::IsDebuggerAttached() \
        { \
            return ChildClass::IsDebuggerAttachedImpl(); \
        } \
        inline void PlatformDebug::BreakDebug() \
        { \
            ChildClass::BreakDebugImpl(); \
        }
    } // namespace platform
} // namespace cube
