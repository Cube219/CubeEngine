#pragma once

#include "CoreHeader.h"

#include "Allocator/FrameAllocator.h"
#include "Format.h"

namespace cube
{
    enum class LogType : Uint8
    {
        Info,
        Warning,
        Error
    };

    // TODO: Thread-safe logging system (Using queue?)
    class Logger
    {
    public:
        Logger(const Logger& other) = delete;
        Logger& operator=(const Logger& other) = delete;

        template <typename... Args>
        static void WriteLogFormatting(LogType type, const char* fullFileName, int lineNum, StringView category, StringView format, Args&&... args)
        {
            if (GetMyThreadFrameAllocator().IsInitialized())
            {
                FrameAllocationScope scope;

                FrameString str = Format<FrameString>(format, std::forward<Args>(args)...);
                WriteLog(type, fullFileName, lineNum, category, str);
            }
            else
            {
                // Use the default allocator if the frame allocator is not initialized.
                String str = Format<String>(format, std::forward<Args>(args)...);
                WriteLog(type, fullFileName, lineNum, category, str);
            }
        }
        CUBE_CORE_EXPORT static void WriteLog(LogType type, const char* fullFileName, int lineNum, StringView category, StringView msg);

    private:
        Logger() = default;
    };
} // namespace cube

#define CUBE_LOG(type, category, format, ...) cube::Logger::WriteLogFormatting(LogType::type, __FILE__, __LINE__, CUBE_T(#category), CUBE_T(format), ##__VA_ARGS__)

