
#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsDebug.h"

#include <iostream>
#include <csignal>
#include <Windows.h>
#include <DbgHelp.h>

#include "Format.h"
#include "FileSystem.h"
#include "Logger.h"
#include "Windows/WindowsString.h"

namespace cube
{
    namespace platform
    {
        void WindowsDebug::PrintToDebugConsoleImpl(StringView str)
        {
            // TODO: Use custom allocator (logger allocator?)
            WindowsString winStr;
            String_ConvertAndAppend(winStr, str);

            std::wcout << winStr << std::endl;
            OutputDebugString(winStr.c_str());
            OutputDebugString(WINDOWS_T("\n"));
        }

        void WindowsDebug::ProcessFatalErrorImpl(StringView msg)
        {
            // TODO: Use custom allocator (logger allocator?)
            WindowsString winStr;
            String_ConvertAndAppend(winStr, msg);

            ShowDebugMessageBox(WINDOWS_T("Fatal error"), winStr);
        }

        void WindowsDebug::ProcessFailedCheckImpl(const char* fileName, int lineNum, StringView formattedMsg)
        {
            // TODO: Use custom allocator (logger allocator?)
            WindowsString winStr;
            String_ConvertAndAppend(winStr, formattedMsg);

            ShowDebugMessageBox(WINDOWS_T("Check failed"), winStr);
        }

        constexpr int MAX_NUM_FRAMES = 128;
        constexpr int MAX_NAME_LENGTH = 1024;

        String WindowsDebug::DumpStackTraceImpl()
        {
            HANDLE process = GetCurrentProcess();

            static bool isSymbolInitialized = false;
            if (isSymbolInitialized == false)
            {
                SymInitialize(process, NULL, TRUE); // TODO: check if is failed
                SymSetOptions(SYMOPT_LOAD_LINES);

                isSymbolInitialized = true;
            }

            PVOID backTrace[MAX_NUM_FRAMES];
            WORD NumFrames = RtlCaptureStackBackTrace(0, MAX_NUM_FRAMES, backTrace, NULL);

            char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_NAME_LENGTH + 1] = {};
            SYMBOL_INFO* symbol = (SYMBOL_INFO*)(symbolBuffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_NAME_LENGTH;

            IMAGEHLP_LINE64	lineInfo = {};
	        lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            DWORD displacement;

            IMAGEHLP_MODULE64 moduleInfo = {};
	        moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

            AnsiString str;
            for (WORD i = 0; i < NumFrames; ++i)
            {
                UINT64 pc = (UINT64)backTrace[i];

                AnsiString moduleName;
                if (SymGetModuleInfo64(process, pc, &moduleInfo))
                {
                    moduleName = FileSystem::SplitFileNameFromFullPath(moduleInfo.ImageName);
                }
                else
                {
                    moduleName = "???";
                }

                AnsiString fileName;
                Uint32 lineNum = 0;
                if (SymGetLineFromAddr64(process, pc, &displacement, &lineInfo))
                {
                    fileName = lineInfo.FileName;
                    lineNum = lineInfo.LineNumber;
                }
                else
                {
                    fileName = "???";
                }

                AnsiString functionName;
                if (SymFromAddr(process, pc, nullptr, symbol))
                {
                    functionName = AnsiString(symbol->Name);
                }
                else
                {
                    functionName = "???";
                }

                str += Format<AnsiString>("{}!{}() - {}:{}\n", moduleName, functionName, fileName, lineNum);
            }

            String res;
            String_ConvertAndAppend(res, str);
            return res;
        }

        bool WindowsDebug::IsDebuggerAttachedImpl()
        {
            return IsDebuggerPresent();
        }

        void WindowsDebug::BreakDebugImpl()
        {
            DebugBreak();
        }

        void WindowsDebug::ShowDebugMessageBox(const WindowsString& title, const WindowsString& msg)
        {
            // TODO: Use custom allocator (logger allocator?)
            WindowsString winStr = Format<WindowsString>(WINDOWS_T("{0}\n\n(Press Retry to debug the application)"), msg);

            int nCode = MessageBox(NULL, winStr.c_str(), title.c_str(),
                                   MB_TASKMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);

            switch (nCode)
            {
            case IDABORT:
                raise(SIGABRT);

                exit(3);
                break;

            case IDRETRY:
                DebugBreak();
                break;

            case IDIGNORE:
                break;
            }
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
