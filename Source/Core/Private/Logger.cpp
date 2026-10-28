#include "Logger.h"

#include <chrono>
#include <fmt/chrono.h>

#include "Platform/PlatformDebug.h"

namespace cube
{
    void Logger::WriteLog(LogType type, const char* fullFileName, int lineNum, StringView category, StringView msg)
    {
        // INFO [2025-03-02 18:22:22.111 / EngineCategory] : Message

        const char* prefix = "";
        platform::PrintColorCategory colorCategory = platform::PrintColorCategory::Default;

        switch (type)
        {
        case LogType::Info:
            prefix = "   INFO";
            colorCategory = platform::PrintColorCategory::Default;
            break;
        case LogType::Warning:
            prefix = "WARNING";
            colorCategory = platform::PrintColorCategory::Warning;
            break;
        case LogType::Error:
            prefix = "  ERROR";
            colorCategory = platform::PrintColorCategory::Error;
            break;
        }

        auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

        if (GetMyThreadFrameAllocator().IsInitialized())
        {
            FrameAllocationScope scope;

            FrameString res = Format<FrameString>(CUBE_T("{0} [{1:%Y-%m-%d %H:%M:%S} / {2}] : {3}"), prefix, now, category, msg);
            platform::Debug::PrintToDebugConsole(res, colorCategory);
        }
        else
        {
            // Use the default allocator if the frame allocator is not initialized.
            String res = Format<String>(CUBE_T("{0} [{1:%Y-%m-%d %H:%M:%S} / {2}] : {3}"), prefix, now, category, msg);
            platform::Debug::PrintToDebugConsole(res, colorCategory);
        }
    }
} // namespace cube
