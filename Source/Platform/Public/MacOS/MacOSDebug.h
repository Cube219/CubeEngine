#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "PlatformDebug.h"

#include <AppKit/AppKit.h>

#include "MacOSString.h"

@interface CubeLoggerWindow : NSWindow

@end

@interface CubeLoggerTextView : NSTextView

@end

@interface CubeLoggerWindowDelegate : NSObject <NSWindowDelegate>

@end

namespace cube
{
    namespace platform
    {
        class MacOSDebug : public PlatformDebug
        {
        public:
            static void PrintToDebugConsoleImpl(StringView str, PrintColorCategory colorCategory);

            static void ProcessFatalErrorImpl(StringView msg);

            static void ProcessFailedCheckImpl(const char* fileName, int lineNum, StringView formattedMsg);

            static String DumpStackTraceImpl(bool removeBeforeProjectFolderPath);

            static bool IsDebuggerAttachedImpl();

            static bool IsLoggerWindowCreated() { return mIsLoggerWindowCreated; }
            static void CreateAndShowLoggerWindow();
            static void AppendLogText(NSString* text, PrintColorCategory colorCategory);
            static void CloseAndDestroyLoggerWindow();

        private:
            static void ShowDebugMessageAlert(StringView title, StringView msg);
        
            static CubeLoggerWindow* mLoggerWindow;
            static CubeLoggerWindowDelegate* mLoggerWindowDelegaate;
            static CubeLoggerTextView* mLoggerTextView;

            static bool mIsLoggerWindowCreated;
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
