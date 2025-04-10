#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSDebug.h"

#include <iostream> // cout
#include <signal.h> // raise

#include "Checker.h"
#include "MacOS/MacOSPlatform.h"
#include "MacOS/MacOSString.h"
#include "MacOS/MacOSUtility.h"

@implementation CubeLoggerWindow

- (void)keyDown:(NSEvent* )event
{
    if (cube::platform::MacOSPlatform::IsApplicationClosed())
    {
        cube::platform::MacOSDebug::CloseAndDestroyLoggerWindow();
    }
}

@end

@implementation CubeLoggerTextView

- (void)keyDown:(NSEvent* ) event
{
    if (cube::platform::MacOSPlatform::IsApplicationClosed())
    {
        cube::platform::MacOSDebug::CloseAndDestroyLoggerWindow();
    }
    
    [super keyDown:event];
}

@end

@implementation CubeLoggerWindowDelegate

- (BOOL)windowShouldClose:(NSWindow* ) sender
{
    // Also closing main window if it is created
    if (cube::platform::MacOSPlatform::IsMainWindowCreated())
    {
        cube::platform::MacOSPlatform::CloseMainWindow();
        // Closing event will be dispatched in main window delegate
    }
    else
    {
        // Otherwise, dispatch the event here
        cube::platform::Platform::GetClosingEvent().Dispatch();
    }

    return YES;
}

@end

namespace cube
{
    namespace platform
    {
        PLATFORM_DEBUG_CLASS_DEFINITIONS(MacOSDebug)

        CubeLoggerWindow* MacOSDebug::mLoggerWindow;
        CubeLoggerWindowDelegate* MacOSDebug::mLoggerWindowDelegaate;
        CubeLoggerTextView* MacOSDebug::mLoggerTextView;

        bool MacOSDebug::mIsLoggerWindowCreated = false;

        void MacOSDebug::PrintToDebugConsoleImpl(StringView str, PrintColorCategory colorCategory)
        {
            MacOSString osStr;
            String_ConvertAndAppend(osStr, str);

            std::cout << osStr << std::endl;

            if (mIsLoggerWindowCreated)
            {
                osStr.push_back('\n');
                MacOSUtility::DispatchToMainThread([osStr, colorCategory] {
                    NSString *nsText = [NSString stringWithUTF8String:osStr.c_str()];
                    AppendLogText(nsText, colorCategory);
                    [nsText release];
                });
            }
        }

        void MacOSDebug::ProcessFatalErrorImpl(StringView msg)
        {
            // TODO
            raise(SIGTRAP);
        }

        void MacOSDebug::ProcessFailedCheckImpl(const char* fileName, int lineNum, StringView formattedMsg)
        {
            // TODO: Fixed infinite loop
            // NOT_IMPLEMENTED();
            raise(SIGTRAP);
        }

        constexpr int MAX_NUM_FRAMES = 128;
        constexpr int MAX_NAME_LENGTH = 1024;

        String MacOSDebug::DumpStackTraceImpl(bool removeBeforeProjectFolderPath)
        {
            // NOT_IMPLEMENTED();
            return {};
        }

        bool MacOSDebug::IsDebuggerAttachedImpl()
        {
            NOT_IMPLEMENTED();
            return false;
        }

        void MacOSDebug::BreakDebugImpl()
        {
            raise(SIGTRAP);
        }

        void MacOSDebug::CreateAndShowLoggerWindow()
        {
            CHECK_MAIN_THREAD()
            CHECK(!mIsLoggerWindowCreated);

            @autoreleasepool {
                NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
                
                mLoggerWindow = [[CubeLoggerWindow alloc]
                    initWithContentRect:NSMakeRect(0, 0, 800, 400)
                    styleMask:style
                    backing:NSBackingStoreBuffered
                    defer:NO
                ];
                [mLoggerWindow setTitle:@"CubeEngine Logger"];

                NSScrollView* scrollView = [[NSScrollView alloc] initWithFrame:[[mLoggerWindow contentView] bounds]];
                [scrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

                mLoggerTextView = [[CubeLoggerTextView alloc] initWithFrame:[[mLoggerWindow contentView] bounds]];
                mLoggerTextView.editable = NO;
                mLoggerTextView.selectable = YES;

                [scrollView setDocumentView:mLoggerTextView];
                [scrollView setHasVerticalScroller:YES];

                [[mLoggerWindow contentView] addSubview:scrollView];

                mLoggerWindowDelegaate = [[CubeLoggerWindowDelegate alloc] init];
                [mLoggerWindow setDelegate:mLoggerWindowDelegaate];

                [mLoggerWindow makeKeyAndOrderFront:nil];
                mIsLoggerWindowCreated = true;

                // Move to top-left
                NSScreen* screen = [mLoggerWindow screen] ?: [NSScreen mainScreen];
                NSRect screenFrame = [screen visibleFrame];
                NSRect windowFrame = [mLoggerWindow frame];

                NSPoint topLeft;
                topLeft.x = screenFrame.origin.x;
                topLeft.y = NSMaxY(screenFrame) - windowFrame.size.height;

                [mLoggerWindow setFrameOrigin:topLeft];
            }
        }

        void MacOSDebug::AppendLogText(NSString* text, PrintColorCategory colorCategory)
        {
            CHECK_MAIN_THREAD()
            CHECK(mIsLoggerWindowCreated);

            @autoreleasepool {
                NSColor* color;
                switch (colorCategory)
                {
                case PrintColorCategory::Warning:
                    color = [NSColor systemOrangeColor];
                    break;
                case PrintColorCategory::Error:
                    color = [NSColor systemRedColor];
                    break;
                case PrintColorCategory::Default:
                default:
                    color = [NSColor textColor];
                    break;
                }
                
                NSTextStorage* textStorage = mLoggerTextView.textStorage;
                NSAttributedString* attrText = [[NSAttributedString alloc]
                    initWithString:text
                        attributes:@{
                            NSForegroundColorAttributeName: color,
                            NSBackgroundColorAttributeName: [NSColor textBackgroundColor],
                            NSFontAttributeName: [NSFont fontWithName:@"Menlo" size:12]
                    }
                ];
                [textStorage appendAttributedString:attrText];

                // Scroll to bottom
                [mLoggerTextView scrollRangeToVisible:NSMakeRange(textStorage.length, 0)];
            }
        }
        

        void MacOSDebug::CloseAndDestroyLoggerWindow()
        {
            CHECK_MAIN_THREAD()

            if (mIsLoggerWindowCreated)
            {
                [mLoggerWindow close];

                [mLoggerTextView release];
                [mLoggerWindow release];
                [mLoggerWindowDelegaate release];

                mIsLoggerWindowCreated = false;
            }
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
