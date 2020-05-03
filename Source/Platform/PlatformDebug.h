#pragma once

#include "PlatformHeader.h"

#include "Utility/Format.h"

namespace cube
{
    namespace platform
    {
        class PLATFORM_EXPORT PlatformDebug
        {
        public:
            static void AssertionFailed(StringView msg, const char* funcName, const char* fileName, int lineNum);

            static void PrintLog(StringView str, const char* funcName, const char* fileName, int lineNum);
            static void PrintToConsole(StringView str);
        };

#define PLATFORM_DEBUG_DEFINITIONS(Child)                                               \
        inline void PlatformDebug::AssertionFailed                                      \
            (StringView msg, const char* funcName, const char* fileName, int lineNum) { \
            Child::AssertionFailedImpl(msg, funcName, fileName, lineNum);               \
        }                                                                               \
                                                                                        \
        inline void PlatformDebug::PrintLog                                             \
            (StringView str, const char* funcName, const char* fileName, int lineNum)   \
        {                                                                               \
            Child::PrintLogImpl(str, funcName, fileName, lineNum);                      \
        }                                                                               \
                                                                                        \
        inline void PlatformDebug::PrintToConsole(StringView str)                       \
        {                                                                               \
            Child::PrintToConsoleImpl(str);                                             \
        }                                                                               

        template <typename ...Args>
        void AssertionFailedFormatting(const char* fileName, int lineNum, StringView msg, Args&&... args)
        {
            PlatformDebug::AssertionFailed(Format(msg.data(), std::forward<Args>(args)...), nullptr, fileName, lineNum);
        }

        template <typename ...Args>
        void PrintLogFormatting(const char* fileName, int lineNum, StringView msg, Args&&... args)
        {
            PlatformDebug::PrintLog(Format(msg.data(), std::forward<Args>(args)...), nullptr, fileName, lineNum);
        }
    } // namespace platform
} // namespace cube

#ifdef _DEBUG

#define PLATFORM_ASSERTION_FAILED(msg, ...)                                                   \
	cube::platform::AssertionFailedFormatting(__FILE__, __LINE__, CUBE_T(msg), ##__VA_ARGS__);

#define PLATFORM_CHECK(expr, msg, ...)                 \
	if(!(expr)){                                       \
		PLATFORM_ASSERTION_FAILED(msg, ##__VA_ARGS__); \
	}

#define PLATFORM_PRINT_LOG(msg, ...)                                                   \
	cube::platform::PrintLogFormatting(__FILE__, __LINE__, CUBE_T(msg), ##__VA_ARGS__);

#else // _DEBUG

#define PLATFORM_ASSERTION_FAILED(msg, ...)
#define PLATFORM_CHECK(expr, msg, ...)
#define PLATFORM_PRINT_LOG(msg, ...)

#endif // _DEBUG
