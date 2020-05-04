#include "LogWriter.h"

#include "Platform/PlatformDebug.h"

namespace cube
{
    void LogWriter::Init()
    {
    }

    const char* LogWriter::SplitFileName(const char* fullPath)
    {
        const char* pitch;

#ifdef _WIN32
        pitch = strrchr(fullPath, '\\');
#else
        pitch = fullPath;
#endif

        if(pitch != nullptr)
            return pitch + 1;

        return fullPath;
    }

    void LogWriter::WriteLog(LogType type, const char* fileName, int lineNum, StringView msg)
    {
        // INFO [18:22:22.1111 / EngineCore.cpp:152] : 내용

        const char* prefix = "";

        switch(type) {
        case LogType::Info:
            prefix = "    INFO [";
            break;
        case LogType::Warning:
            prefix = " WARNING [";
            break;
        case LogType::Error:
            prefix = "   ERROR [";
            break;

        default:
            break;
        }

        FrameString res = FrameFormat(CUBE_T("{0} / {1}:{2}] : {3}"), prefix, SplitFileName(fileName), lineNum, msg);
        WriteLog(res);
    }

    void LogWriter::WriteLog(StringView log)
    {
        // TODO: File에 쓰는 방식 추가
        // TODO: 시간 기록
#ifdef _DEBUG
        platform::PlatformDebug::PrintToConsole(log);
#endif // _DEBUG
    }
} // namespace cube
