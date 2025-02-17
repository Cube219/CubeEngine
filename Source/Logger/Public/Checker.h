#pragma once

#include "LoggerHeader.h"

#include "CubeString.h"
#include "Logger.h"

#ifndef CUBE_USE_CHECK
#define CUBE_USE_CHECK _DEBUG
#endif

namespace cube
{
#ifdef CUBE_USE_CHECK

    class ICheckerExtension
    {
    public:
        virtual ~ICheckerExtension() = default;

        virtual void ProcessFailedCheck(const char* fullFileName, int lineNum, StringView exprAndMsg) = 0;
    };

    class Checker
    {
    public:
        Checker(const Checker& other) = delete;
        Checker& operator=(const Checker& other) = delete;

        template <typename CheckerExtension>
        static void RegisterExtension()
        {
            mExtensions.push_back(std::make_unique<CheckerExtension>());
        }

        template <typename... Args>
        static void ProcessFailedCheckFormatting(const char* fullFileName, int lineNum, StringView expr, StringView format, Args&&... args)
        {
            Logger::LoggerString msg = Format<Logger::LoggerString>(format, std::forward<Args>(args)...);
            Logger::LoggerString exprAndMsg = Format<Logger::LoggerString>(CUBE_T("{} / {}"), expr, msg);
            ProcessFailedCheck(fullFileName, lineNum, exprAndMsg);
        }

        CUBE_LOGGER_EXPORT static void ProcessFailedCheck(const char* fullFileName, int lineNum, StringView exprAndMsg);

    private:
        Checker() = default;

        CUBE_LOGGER_EXPORT static Vector<UniquePtr<ICheckerExtension>> mExtensions;
    };

#define CHECK(expr) \
	if (!(expr)) \
	{ \
		cube::Checker::ProcessFailedCheck(__FILE__, __LINE__, CUBE_T(#expr)); \
	}

#define CHECK_FORMAT(expr, format, ...) \
	if (!(expr)) \
	{ \
		cube::Checker::ProcessFailedCheckFormatting(__FILE__, __LINE__, CUBE_T(#expr), CUBE_T(format), ##__VA_ARGS__); \
	}

#else // CUBE_USE_CHECK

#define CHECK(expr)
#define CHECK_FORMAT(expr, format, ...)

#endif // CUBE_USE_CHECK
} // namespace cube
