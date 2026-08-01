
#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsDebug.h"

#include <Windows.h>
#include <corecrt_io.h>
#include <csignal>
#include <cstdlib>
#include <exception>
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
        bool WindowsDebug::mIsTestMode = false;

        void WindowsDebug::PrintToDebugConsole(StringView str, PrintColorCategory colorCategory)
        {
            // TODO: Use custom allocator (logger allocator?)
            WindowsString winStr = String_Convert<WindowsString>(str);

            std::wcout << winStr << std::endl;
            OutputDebugString(winStr.c_str());
            OutputDebugString(WINDOWS_T("\n"));
        }

        void WindowsDebug::ProcessFatalError(StringView msg)
        {
            // TODO: Use custom allocator (logger allocator?)
            ShowDebugMessageBox(WINDOWS_T("Fatal error"), String_Convert<WindowsString>(msg));
        }

        void WindowsDebug::ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg)
        {
            if (!WindowsDebug::IsDebuggerAttached())
            {
                // TODO: Use custom allocator (logger allocator?)
                ShowDebugMessageBox(WINDOWS_T("Check failed"), String_Convert<WindowsString>(formattedMsg));
            }

            if (mIsTestMode)
            {
                // Force terminate in test mode.
                exit(3);
            }
        }

        constexpr int MAX_NUM_FRAMES = 128;
        constexpr int MAX_NAME_LENGTH = 1024;

        String WindowsDebug::DumpStackTrace(bool removeBeforeProjectFolderPath)
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
            // Skip one stack frame
            //     - WindowsDebug::DumpStackTrace()
            for (WORD i = 1; i < NumFrames; ++i)
            {
                UINT64 pc = (UINT64)backTrace[i];

                AnsiString moduleName;
                if (SymGetModuleInfo64(process, pc, &moduleInfo))
                {
                    moduleName = WindowsFileSystem::SplitFileNameFromFullPath(moduleInfo.ImageName);
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

        bool WindowsDebug::IsDebuggerAttached()
        {
            // Ignore debugger to continue the program.
            if (mIsTestMode)
            {
                return false;
            }

            return IsDebuggerPresent();
        }

        void WindowsDebug::SetTestMode(bool enable)
        {
            mIsTestMode = enable;
        }

        bool WindowsDebug::IsTestMode()
        {
            return mIsTestMode;
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

                HWND consoleWindow = GetConsoleWindow();
                RECT consoleWindowRect;
                GetWindowRect(consoleWindow, &consoleWindowRect);
                MoveWindow(consoleWindow, consoleWindowRect.left, consoleWindowRect.top, 1280, 720, TRUE);
            }
        }

        void WindowsDebug::ShowDebugMessageBox(const WindowsString& title, const WindowsString& msg)
        {
            if (mIsTestMode)
            {
                return;
            }

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

        namespace
        {
            StringView GetExceptionCodeString(DWORD exceptionCode)
            {
                switch (exceptionCode)
                {
                case EXCEPTION_ACCESS_VIOLATION:
                    return CUBE_T("Access Violation");
                case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
                    return CUBE_T("Array Bounds Exceeded");
                case EXCEPTION_BREAKPOINT:
                    return CUBE_T("Breakpoint");
                case EXCEPTION_DATATYPE_MISALIGNMENT:
                    return CUBE_T("Datatype Misalignment");
                case EXCEPTION_FLT_DENORMAL_OPERAND:
                    return CUBE_T("Float Denormal Operand");
                case EXCEPTION_FLT_DIVIDE_BY_ZERO:
                    return CUBE_T("Float Divide By Zero");
                case EXCEPTION_FLT_INEXACT_RESULT:
                    return CUBE_T("Float Inexact Result");
                case EXCEPTION_FLT_INVALID_OPERATION:
                    return CUBE_T("Float Invalid Operation");
                case EXCEPTION_FLT_OVERFLOW:
                    return CUBE_T("Float Overflow");
                case EXCEPTION_FLT_STACK_CHECK:
                    return CUBE_T("Float Stack Check");
                case EXCEPTION_FLT_UNDERFLOW:
                    return CUBE_T("Float Underflow");
                case EXCEPTION_ILLEGAL_INSTRUCTION:
                    return CUBE_T("Illegal Instruction");
                case EXCEPTION_IN_PAGE_ERROR:
                    return CUBE_T("In Page Error");
                case EXCEPTION_INT_DIVIDE_BY_ZERO:
                    return CUBE_T("Integer Divide By Zero");
                case EXCEPTION_INT_OVERFLOW:
                    return CUBE_T("Integer Overflow");
                case EXCEPTION_INVALID_DISPOSITION:
                    return CUBE_T("Invalid Disposition");
                case EXCEPTION_NONCONTINUABLE_EXCEPTION:
                    return CUBE_T("Noncontinuable Exception");
                case EXCEPTION_PRIV_INSTRUCTION:
                    return CUBE_T("Privileged Instruction");
                case EXCEPTION_SINGLE_STEP:
                    return CUBE_T("Single Step");
                case EXCEPTION_STACK_OVERFLOW:
                    return CUBE_T("Stack Overflow");
                default:
                    return CUBE_T("Unknown Exception");
                }
            }

            void LogExceptionMessage(StringView msg)
            {
                if (Logger::IsInitialized())
                {
                    CUBE_LOG(Error, WindowsDebug, "{0}", msg);
                }
                else
                {
                    WindowsDebug::PrintToDebugConsole(msg, PrintColorCategory::Error);
                }
            }

            void HandleFatalException(StringView exceptionTypeName, StringView details)
            {
                String msg = Format<String>(CUBE_T("Unhandled exception: {0}\n{1}"), exceptionTypeName, details);

                LogExceptionMessage(msg);

                String stackTrace = WindowsDebug::DumpStackTrace();
                if (!stackTrace.empty())
                {
                    String stackMsg = Format<String>(CUBE_T("Stack trace:\n{0}"), stackTrace);
                    LogExceptionMessage(stackMsg);
                }

                String fullMsg = Format<String>(CUBE_T("{0}\n\n{1}"), msg, stackTrace);
                WindowsDebug::ProcessFatalError(fullMsg);
            }

            LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* ep)
            {
                DWORD exceptionCode = ep->ExceptionRecord->ExceptionCode;
                StringView exceptionName = GetExceptionCodeString(exceptionCode);

                String details = Format<String>(CUBE_T("Code: 0x{0:x}"), exceptionCode);

                if (exceptionCode == EXCEPTION_ACCESS_VIOLATION)
                {
                    StringView accessType = (ep->ExceptionRecord->ExceptionInformation[0] == 0) ? CUBE_T("Read") : CUBE_T("Write");
                    details = Format<String>(
                        CUBE_T("Code: 0x{0:x}\n{1} at address 0x{2:x}"),
                        exceptionCode,
                        accessType,
                        ep->ExceptionRecord->ExceptionInformation[1]
                    );
                }

                HandleFatalException(exceptionName, details);

                if (WindowsDebug::IsTestMode())
                {
                    exit(3);
                }

                return EXCEPTION_EXECUTE_HANDLER;
            }

            void SignalHandler(int signal)
            {
                StringView signalName;
                switch (signal)
                {
                case SIGABRT:
                    signalName = CUBE_T("SIGABRT");
                    break;
                case SIGSEGV:
                    signalName = CUBE_T("SIGSEGV");
                    break;
                case SIGILL:
                    signalName = CUBE_T("SIGILL");
                    break;
                case SIGFPE:
                    signalName = CUBE_T("SIGFPE");
                    break;
                default:
                    signalName = CUBE_T("Unknown Signal");
                    break;
                }

                String details = Format<String>(CUBE_T("Signal: {0} ({1})"), signalName, signal);
                HandleFatalException(signalName, details);

                if (WindowsDebug::IsTestMode())
                {
                    exit(3);
                }
            }

            void TerminateHandler()
            {
                StringView exceptionName = CUBE_T("std::terminate");

                String details = String(CUBE_T("Uncaught C++ exception"));
                try
                {
                    std::rethrow_exception(std::current_exception());
                }
                catch (const std::exception& e)
                {
                    String whatStr = String_Convert<String>(e.what());
                    details = Format<String>(CUBE_T("Uncaught C++ exception: {0}"), whatStr);
                }
                catch (...)
                {
                }

                HandleFatalException(exceptionName, details);

                if (WindowsDebug::IsTestMode())
                {
                    exit(3);
                }
            }

            void InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
            {
                WindowsString winMsg = Format<WindowsString>(
                    WINDOWS_T("Invalid parameter\nExpression: {0}\nFunction: {1}\nFile: {2}:{3}"),
                    expression ? expression : WINDOWS_T("(null)"),
                    function ? function : WINDOWS_T("(null)"),
                    file ? file : WINDOWS_T("(null)"),
                    line
                );

                WindowsDebug::PrintToDebugConsole(String_Convert<String>(winMsg), PrintColorCategory::Error);

                String stackTrace = WindowsDebug::DumpStackTrace();
                if (!stackTrace.empty())
                {
                    String stackMsg = Format<String>(CUBE_T("Stack trace:\n{0}"), stackTrace);
                    LogExceptionMessage(stackMsg);
                }

                if (WindowsDebug::IsTestMode())
                {
                    exit(3);
                }
            }
        } // namespace

        void WindowsDebug::InstallCrashHandlers()
        {
            SetUnhandledExceptionFilter(&UnhandledExceptionFilter);

            signal(SIGABRT, &SignalHandler);
            signal(SIGILL, &SignalHandler);
            signal(SIGFPE, &SignalHandler);

            _set_invalid_parameter_handler(&InvalidParameterHandler);

            std::set_terminate(&TerminateHandler);
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
