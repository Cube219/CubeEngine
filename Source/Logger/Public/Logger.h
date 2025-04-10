#pragma once

#include "LoggerHeader.h"

#include "Allocator.h"
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

    // TODO: Thread-safe logging/allocation system (Using queue?)
    class Logger
    {
    public:
        template <typename T>
        class StdAllocator
        {
        public:
            using value_type = T;

            StdAllocator(const char* pName = nullptr) {}
            template <typename U>
            StdAllocator(const StdAllocator<U>& other) noexcept {}

            T* allocate(size_t n)
            {
                return (T*)Logger::mAllocator->Allocate(sizeof(T) * n);
            }

            void deallocate(T* p, size_t n)
            {
                Logger::mAllocator->Free(p, n);
            }
        };

        using LoggerString = std::basic_string<Character, std::char_traits<Character>, StdAllocator<Character>>;

    public:
        Logger(const Logger& other) = delete;
        Logger& operator=(const Logger& other) = delete;

        CUBE_LOGGER_EXPORT static void Init(IAllocator* loggerAllocator);

        template <typename LoggerExtension>
        static void RegisterExtension()
        {
            mExtensions.push_back(std::make_unique<LoggerExtension>());
        }

        CUBE_LOGGER_EXPORT static void SetFilePathSeparator(char separator);

        template <typename... Args>
        static void WriteLogFormatting(LogType type, const char* fullFileName, int lineNum, StringView category, StringView format, Args&&... args)
        {
            LoggerString str = Format<LoggerString>(format, std::forward<Args>(args)...);
            WriteLog(type, fullFileName, lineNum, category, str);
        }
        CUBE_LOGGER_EXPORT static void WriteLog(LogType type, const char* fullFileName, int lineNum, StringView category, StringView msg);

    private:
        Logger() = default;

        static const char* SplitFileName(const char* fullPath);

        CUBE_LOGGER_EXPORT static IAllocator* mAllocator;
        static char mFilePathSeparator;
        CUBE_LOGGER_EXPORT static Vector<UniquePtr<ILoggerExtension>> mExtensions;
    };
} // namespace cube

#define CUBE_LOG(type, category, format, ...) cube::Logger::WriteLogFormatting(type, __FILE__, __LINE__, CUBE_T(#category), CUBE_T(format), ##__VA_ARGS__)
