#pragma once

#include "PlatformHeader.h"

#include "Format.h"

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

        class CUBE_PLATFORM_EXPORT PlatformDebug
        {
        public:
            static void PrintToDebugConsole(StringView str, PrintColorCategory colorCategory = PrintColorCategory::Default);

            static void ProcessFatalError(StringView msg);

            static void ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg);

            static String DumpStackTrace(bool removeBeforeProjectFolderPath = true);

            static bool IsDebuggerAttached();
        };

#define PLATFORM_DEBUG_CLASS_DEFINITIONS(ChildClass) \
        void PlatformDebug::PrintToDebugConsole(StringView str, PrintColorCategory colorCategory) \
        { \
            ChildClass::PrintToDebugConsoleImpl(str, colorCategory); \
        } \
        \
        void PlatformDebug::ProcessFatalError(StringView msg) \
        { \
            ChildClass::ProcessFatalErrorImpl(msg); \
        } \
        void PlatformDebug::ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg) \
        { \
            ChildClass::ProcessFailedCheckImpl(fileName, lineNum, formattedMsg); \
        } \
        String PlatformDebug::DumpStackTrace(bool removeBeforeProjectFolderPath) \
        { \
            return ChildClass::DumpStackTraceImpl(removeBeforeProjectFolderPath); \
        } \
        bool PlatformDebug::IsDebuggerAttached() \
        { \
            return ChildClass::IsDebuggerAttachedImpl(); \
        }
    } // namespace platform
} // namespace cube
