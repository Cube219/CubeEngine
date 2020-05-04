#pragma once

#ifdef CORE_EXPORTS
#define CORE_EXPORT __declspec(dllexport) 
#else // CORE_EXPORTS
#define CORE_EXPORT __declspec(dllimport) 
#endif // CORE_EXPORTS

#include "Utility/Types.h"
#include "Utility/Format.h"
#include "Allocator/FrameAllocator.h"

namespace cube
{
    enum class LogType
    {
        Info, Warning, Error
    };

    class CORE_EXPORT LogWriter
    {
    public:
        LogWriter() = delete;
        LogWriter(const LogWriter& other) = delete;
        LogWriter& operator=(const LogWriter& other) = delete;

        static void Init();

        template <typename ...Args>
        static void WriteLogFormatting(LogType type, const char* fileName, int lineNum, StringView msg, Args&&... args)
        {
            FrameString str = FrameFormat(msg.data(), std::forward<Args>(args)...);
            WriteLog(type, fileName, lineNum, str);
        }

        static void WriteLog(LogType type, const char* fileName, int lineNum, StringView msg);

    private:
        static void WriteLog(StringView log);

        static const char* SplitFileName(const char* fullPath);
    };
} // namespace cube

#define CUBE_LOG(type, msg, ...) cube::LogWriter::WriteLogFormatting(type, __FILE__, __LINE__, CUBE_T(msg), ##__VA_ARGS__)
