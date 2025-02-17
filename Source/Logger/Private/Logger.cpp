#include "Logger.h"

#include <chrono>
#include <fmt/chrono.h>

namespace cube
{
    IAllocator* Logger::mAllocator = nullptr;
    char Logger::mFilePathSeparator = '\\';
    Vector<UniquePtr<ILoggerExtension>> Logger::mExtensions;

    void Logger::Init(IAllocator* loggerAllocator)
    {
        mAllocator = loggerAllocator;
    }

    void Logger::SetFilePathSeparator(char separator)
    {
        mFilePathSeparator = separator;
    }

    const char* Logger::SplitFileName(const char* fullPath)
    {
        const char* lastSeparator = strrchr(fullPath, mFilePathSeparator);

        if (lastSeparator != nullptr)
        {
            return lastSeparator + 1;
        }

        return fullPath;
    }

    void Logger::WriteLog(LogType type, const char* fullFileName, int lineNum, StringView category, StringView msg)
    {
        // INFO [2025-03-02 18:22:22.111 / EngineCategory] : Message

        const char* prefix = "";

        switch (type)
        {
        case LogType::Info:
            prefix = "   INFO";
            break;
        case LogType::Warning:
            prefix = "WARNING";
            break;
        case LogType::Error:
            prefix = "  ERROR";
            break;
        }

        auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

        LoggerString res = Format<LoggerString>(CUBE_T("{0} [{1:%Y-%m-%d %H:%M:%S} / {2}] : {3}"), prefix, now, category, msg);

        for (auto& extension : mExtensions)
        {
            extension->WriteFormattedLog(res);
        }
    }
} // namespace cube
