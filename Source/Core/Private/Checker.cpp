#include "Checker.h"

#include "Logger.h"
#include "Platform/FileSystem.h"
#include "Platform/PlatformDebug.h"

namespace cube
{
#if CUBE_USE_CHECK
    bool Checker::mIsDebuggerAttached = false;

    void Checker::ProcessFailedCheck(const char* fullFileName, int lineNum, StringView exprAndMsg)
    {
        String stackTrace = platform::Debug::DumpStackTrace();
        const char* fileName = platform::FileSystem::SplitFileNameFromFullPath(fullFileName);

        String formattedMsg = Format(CUBE_T("Check failed!\n\n{0}\n    In {1}:{2}\n    {3}"), stackTrace, fileName, lineNum, exprAndMsg);
        Logger::WriteLog(LogType::Error, fullFileName, lineNum, CUBE_T("Checker"), formattedMsg);

        mIsDebuggerAttached = platform::Debug::IsDebuggerAttached();
        platform::Debug::ProcessFailedCheck(fileName, lineNum, formattedMsg);
    }
#endif // CUBE_USE_CHECK
} // namespace cube
