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

    class ILoggerExtension
    {
    public:
        virtual ~ILoggerExtension() = default;

        virtual void WriteFormattedLog(LogType type, StringView formattedLog) = 0;
    };

    // TODO: Thread-safe logging system (Using queue?)
    class Logger
    {
    public:
        Logger(const Logger& other) = delete;
        Logger& operator=(const Logger& other) = delete;

        CUBE_CORE_EXPORT static void Init();
        CUBE_CORE_EXPORT static bool IsInitialized();

        template <typename LoggerExtension>
        static void RegisterExtension()
        {
            mExtensions.push_back(std::make_unique<LoggerExtension>());
        }

        CUBE_CORE_EXPORT static void SetFilePathSeparator(char separator);

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

        static const char* SplitFileName(const char* fullPath);

        static bool mIsInitialized;
        static char mFilePathSeparator;
        CUBE_CORE_EXPORT static Vector<UniquePtr<ILoggerExtension>> mExtensions;
    };
} // namespace cube

#define CUBE_LOG(type, category, format, ...) cube::Logger::WriteLogFormatting(LogType::type, __FILE__, __LINE__, CUBE_T(#category), CUBE_T(format), ##__VA_ARGS__)

