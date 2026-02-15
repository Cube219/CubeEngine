#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "PlatformDebug.h"

#ifdef __OBJC__
#include <AppKit/AppKit.h>

#include "MacOSString.h"

@interface CubeLoggerWindow : NSWindow

@end

@interface CubeLoggerTextView : NSTextView

@end

@interface CubeLoggerWindowDelegate : NSObject <NSWindowDelegate>

@end
#endif // __OBJC__

namespace cube
{
    namespace platform
    {
        class MacOSDebug : public BaseDebug
        {
            // === Base member functions ===
        public:
            static void PrintToDebugConsole(StringView str, PrintColorCategory colorCategory = PrintColorCategory::Default);

            static void ProcessFatalError(StringView msg);

            static void ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg);

            static String DumpStackTrace(bool removeBeforeProjectFolderPath = true);

            static bool IsDebuggerAttached();
            // === Base member functions ===

#ifdef __OBJC__
            static bool IsLoggerWindowCreated() { return mIsLoggerWindowCreated; }
            static void CreateAndShowLoggerWindow();
            static void AppendLogText(NSString* text, PrintColorCategory colorCategory);
            static void CloseAndDestroyLoggerWindow();

        private:
            static void ShowDebugMessageAlert(StringView title, StringView msg);
            static void ShowDebugMessageAlert_MainThread(StringView title, StringView msg);

            static CubeLoggerWindow* mLoggerWindow;
            static CubeLoggerWindowDelegate* mLoggerWindowDelegaate;
            static CubeLoggerTextView* mLoggerTextView;

            static bool mIsLoggerWindowCreated;

            static bool mIsDebugBreakSetInDebugMessageAlert;
            static bool mIsForceTerminationSetInDebugMessageAlert;
#endif // __OBJC__
        };
        using Debug = MacOSDebug;
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
