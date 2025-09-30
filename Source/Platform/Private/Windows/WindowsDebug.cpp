
#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsDebug.h"

#include <Windows.h>
#include <corecrt_io.h>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <DbgHelp.h> // Must be included after Windows.h

#include "FileSystem.h"
#include "Format.h"
#include "Logger.h"
#include "Windows/WindowsString.h"

namespace cube
{
    namespace platform
    {
        PLATFORM_DEBUG_CLASS_DEFINITIONS(WindowsDebug)

        void WindowsDebug::PrintToDebugConsoleImpl(StringView str, PrintColorCategory colorCategory)
        {
            // TODO: Use custom allocator (logger allocator?)
            WindowsString winStr = String_Convert<WindowsString>(str);

            std::wcout << winStr << std::endl;
            OutputDebugString(winStr.c_str());
            OutputDebugString(WINDOWS_T("\n"));
        }

        void WindowsDebug::ProcessFatalErrorImpl(StringView msg)
        {
            // TODO: Use custom allocator (logger allocator?)
            ShowDebugMessageBox(WINDOWS_T("Fatal error"), String_Convert<WindowsString>(msg));
        }

        void WindowsDebug::ProcessFailedCheckImpl(const char* fileName, int lineNum, StringView formattedMsg)
        {
            if (!PlatformDebug::IsDebuggerAttached())
            {
                // TODO: Use custom allocator (logger allocator?)
                ShowDebugMessageBox(WINDOWS_T("Check failed"), String_Convert<WindowsString>(formattedMsg));
            }
        }

        constexpr int MAX_NUM_FRAMES = 128;
        constexpr int MAX_NAME_LENGTH = 1024;

        String WindowsDebug::DumpStackTraceImpl(bool removeBeforeProjectFolderPath)
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

            IMAGEHLP_LINE64 lineInfo = {};
            lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            DWORD displacement;

            IMAGEHLP_MODULE64 moduleInfo = {};
            moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

            AnsiString str;
            // Skip two stack frame
            //     - WindowsDebug::DumpStackTraceImpl()
            //     - PlatformDebug::DumpStackTrace()
            for (WORD i = 2; i < NumFrames; ++i)
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
                    AnsiStringView fileNameCStr(lineInfo.FileName);
                    if (removeBeforeProjectFolderPath)
                    {
                        const AnsiCharacter* projectFolderName = "\\CubeEngine\\";
                        const int projectFolderNameSize = strlen(projectFolderName);
                        // File name index
                        for (Int32 fIndex = static_cast<Int32>(fileNameCStr.size() - projectFolderNameSize); fIndex >= 0; --fIndex)
                        {
                            bool isMatch = true;
                            // Project folder name index
                            for (int pIndex = 0; pIndex < projectFolderNameSize; ++pIndex)
                            {
                                if (projectFolderName[pIndex] != fileNameCStr[fIndex + pIndex])
                                {
                                    isMatch = false;
                                    break;
                                }
                            }

                            if (isMatch)
                            {
                                fileNameCStr.remove_prefix(fIndex + 1);
                                break;
                            }
                        }
                    }
                    fileName = fileNameCStr;
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

                if (functionName == "WinMain")
                {
                    break;
                }
            }

            return String_Convert<String>(str);
        }

        bool WindowsDebug::IsDebuggerAttachedImpl()
        {
            return IsDebuggerPresent();
        }

        void WindowsDebug::CreateAndShowLoggerWindow()
        {
            if (AllocConsole())
            {
                std::wcout.imbue(std::locale(""));

                SetConsoleOutputCP(CP_WINUNICODE);
                _setmode(_fileno(stdout), _O_WTEXT);

                FILE* acStreamIn;
                FILE* acStreamOut;
                FILE* acStreamErr;

                freopen_s(&acStreamIn, "CONIN$", "rb", stdin);
                freopen_s(&acStreamOut, "CONOUT$", "wb", stdout);
                freopen_s(&acStreamErr, "CONOUT$", "wb", stderr);
            }
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
