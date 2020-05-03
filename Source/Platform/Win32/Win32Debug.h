#pragma once

#include "../PlatformHeader.h"

#include <Windows.h>
#include <mutex>

#include "../PlatformDebug.h"

namespace cube
{
    namespace platform
    {
        class PLATFORM_EXPORT Win32Debug : public PlatformDebug
        {
        public:
            static void AssertionFailedImpl(StringView msg, const char* funcName, const char* fileName, int lineNum);

            static void PrintLogImpl(StringView str, const char* funcName, const char* fileName, int lineNum);
            static void PrintToConsoleImpl(StringView str);

        private:
            static const char* GetBaseName(const char* absolutePath);

            static std::mutex printMutex;
        };
        PLATFORM_DEBUG_DEFINITIONS(Win32Debug)
    } // namespace platform
} // namespace cube
