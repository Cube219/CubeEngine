#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSLoggerSubprocess.h"

#if CUBE_MACOS_USE_LOGGER_WINDOW

#include <AppKit/AppKit.h>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <unistd.h>

#include "MacOS/MacOSPlatform.h"
#include "MacOS/MacOSString.h"
#include "MacOS/MacOSUtility.h"

@interface CubeLoggerSubprocessWindow : NSWindow
@end

@interface CubeLoggerSubprocessTextView : NSTextView
@end

@interface CubeLoggerSubprocessWindowDelegate : NSObject <NSWindowDelegate>
@end

@interface CubeLoggerSubprocessAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation CubeLoggerSubprocessWindow

- (void)keyDown:(NSEvent*)event
{
    if (cube::platform::MacOSLoggerSubprocess::IsPressAnyKeyActivated())
    {
        [NSApp terminate:nil];
    }
}

@end

@implementation CubeLoggerSubprocessTextView

- (void)keyDown:(NSEvent*)event
{
    if (cube::platform::MacOSLoggerSubprocess::IsPressAnyKeyActivated())
    {
        [NSApp terminate:nil];
        return;
    }
    [super keyDown:event];
}

@end

@implementation CubeLoggerSubprocessWindowDelegate

- (BOOL)windowShouldClose:(NSWindow*)sender
{
    [NSApp terminate:nil];
    return NO;
}

@end

@implementation CubeLoggerSubprocessAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    // Yield activation to the parent so the logger window does not hide the main window.
    pid_t parentPid = getppid();
    NSRunningApplication* parentApp = [NSRunningApplication runningApplicationWithProcessIdentifier:parentPid];
    if (parentApp != nil)
    {
        [NSApp yieldActivationToApplication:parentApp];
        [parentApp activateWithOptions:0];
    }
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    cube::platform::MacOSLoggerSubprocess::OnAppWillTerminate();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}

@end

namespace cube
{
    namespace platform
    {
        // === Parent-side static state ===
        NSTask* MacOSLoggerSubprocess::mLoggerTask = nil;
        NSFileHandle* MacOSLoggerSubprocess::mLoggerWriteHandle = nil;
        std::mutex MacOSLoggerSubprocess::mLoggerWriteMutex;
        std::atomic<bool> MacOSLoggerSubprocess::mIsAlive = false;

        // === Child-side static state ===
        CubeLoggerSubprocessWindow* MacOSLoggerSubprocess::mWindow = nil;
        CubeLoggerSubprocessTextView* MacOSLoggerSubprocess::mTextView = nil;
        CubeLoggerSubprocessWindowDelegate* MacOSLoggerSubprocess::mWindowDelegate = nil;
        CubeLoggerSubprocessAppDelegate* MacOSLoggerSubprocess::mAppDelegate = nil;
        std::atomic<bool> MacOSLoggerSubprocess::mIsPressAnyKeyActivated = false;
        std::thread MacOSLoggerSubprocess::mReaderThread;

        // === Parent side ===

        void MacOSLoggerSubprocess::Spawn()
        {
            CHECK_MACOS_MAIN_THREAD()
            CHECK(!mIsAlive.load());

            NSString* execPath = [[NSBundle mainBundle] executablePath];
            if (execPath == nil)
            {
                NSLog(@"MacOSLoggerSubprocess::Spawn: failed to resolve executable path");
                return;
            }

            mLoggerTask = [[NSTask alloc] init];
            mLoggerTask.launchPath = execPath;
            mLoggerTask.arguments = @[@"--logger-subprocess"];

            NSPipe* stdinPipe = [NSPipe pipe];
            // Inherit stdout/stderr so subprocess's std::cout lands in the same terminal.
            mLoggerTask.standardInput = stdinPipe;
            mLoggerTask.terminationHandler = ^(NSTask* task) {
                mIsAlive.store(false);

                // Force terminate main app.
                if (!MacOSPlatform::IsApplicationClosed())
                {
                    MacOSUtility::DispatchToMainThread([] {
                        [NSApp terminate:nil];
                    });
                }
            };

            NSError* error = nil;
            BOOL ok = [mLoggerTask launchAndReturnError:&error];
            if (!ok)
            {
                NSLog(@"MacOSLoggerSubprocess::Spawn: launch failed: %@", error);
                mLoggerTask = nil;
                return;
            }

            mLoggerWriteHandle = [stdinPipe fileHandleForWriting];
            mIsAlive.store(true);
        }

        void MacOSLoggerSubprocess::Shutdown()
        {
            if (mLoggerTask == nil)
            {
                return;
            }

            // Close write handle → subprocess sees EOF on stdin and shows the "Press any key" prompt.
            if (mLoggerWriteHandle != nil)
            {
                std::lock_guard<std::mutex> lock(mLoggerWriteMutex);
                @try
                {
                    [mLoggerWriteHandle closeFile];
                }
                @catch (NSException* ex)
                {
                    // Already closed (subprocess died). Ignore.
                }
                mLoggerWriteHandle = nil;
            }

            // Yield activation to the logger process so it can get keyboard events immediately
            // without the user needing to click the logger window.
            pid_t loggerPID = [mLoggerTask processIdentifier];
            NSRunningApplication* loggerApp = [NSRunningApplication runningApplicationWithProcessIdentifier:loggerPID];
            if (loggerApp != nil)
            {
                [NSApp yieldActivationToApplication:loggerApp];
                [loggerApp activateWithOptions:0];
            }

            if (mIsAlive.load())
            {
                [mLoggerTask waitUntilExit];
            }

            mLoggerTask = nil;
        }

        void MacOSLoggerSubprocess::Send(StringView str, PrintColorCategory colorCategory)
        {
            if (!mIsAlive.load() || mLoggerWriteHandle == nil)
            {
                return;
            }

            MacOSString osStr = String_Convert<MacOSString>(str);
            const Uint8 colorByte = static_cast<Uint8>(colorCategory);
            const Uint32 length = static_cast<Uint32>(osStr.size());

            NSMutableData* data = [NSMutableData dataWithCapacity:5 + length];
            [data appendBytes:&colorByte length:1];
            [data appendBytes:&length length:4];
            [data appendBytes:osStr.data() length:length];

            std::lock_guard<std::mutex> lock(mLoggerWriteMutex);
            NSError* error = nil;
            if (![mLoggerWriteHandle writeData:data error:&error])
            {
                // Pipe broken (subprocess exited). Mark dead and silently drop.
                mIsAlive.store(false);
            }
        }

        // === Child side ===

        void MacOSLoggerSubprocess::AppendLogString(NSString* text, PrintColorCategory colorCategory)
        {
            CHECK_MACOS_MAIN_THREAD()

            if (mTextView == nil)
            {
                return;
            }

            NSColor* color = nil;
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

            NSAttributedString* attrText = [[NSAttributedString alloc]
                initWithString:text
                    attributes:@{
                        NSForegroundColorAttributeName: color,
                        NSBackgroundColorAttributeName: [NSColor textBackgroundColor],
                        NSFontAttributeName: [NSFont fontWithName:@"Menlo" size:12]
                    }
            ];
            NSTextStorage* storage = mTextView.textStorage;
            [storage appendAttributedString:attrText];
            [mTextView scrollRangeToVisible:NSMakeRange(storage.length, 0)];
        }

        bool MacOSLoggerSubprocess::ReadExact(int fd, void* buf, Uint64 len)
        {
            Byte* p = (Byte*)buf;
            Uint64 remaining = len;
            while (remaining > 0)
            {
                ssize_t n = read(fd, p, remaining);
                if (n > 0)
                {
                    p += n;
                    remaining -= (size_t)n;
                }
                else if (n == 0)
                {
                    return false; // EOF
                }
                else
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    return false;
                }
            }
            return true;
        }

        void MacOSLoggerSubprocess::ReaderThreadMain()
        {
            const int fd = STDIN_FILENO;
            while (true)
            { @autoreleasepool {
                Byte header[5];
                if (!ReadExact(fd, header, sizeof(header)))
                {
                    break;
                }

                Uint8 colorByte = header[0];
                Uint32 length;
                memcpy(&length, &header[1], sizeof(length));

                NSMutableData* payload = [NSMutableData dataWithLength:length];
                if (length > 0)
                {
                    if (!ReadExact(fd, [payload mutableBytes], length))
                    {
                        break;
                    }
                }

                NSString* msg = [[NSString alloc] initWithData:payload encoding:NSUTF8StringEncoding];
                if (msg == nil)
                {
                    msg = @"<invalid UTF-8 log message>";
                }
                NSString* lineWithNewline = [msg stringByAppendingString:@"\n"];

                PrintColorCategory category;
                switch (colorByte)
                {
                case 1:
                    category = PrintColorCategory::Warning;
                    break;
                case 2:
                    category = PrintColorCategory::Error;
                    break;
                default:
                    category = PrintColorCategory::Default;
                    break;
                }

                MacOSUtility::DispatchToMainThread([lineWithNewline, category] {
                    AppendLogString(lineWithNewline, category);
                });
            }}

            // EOF or error. Show the "press any key" prompt on the main thread.
            MacOSUtility::DispatchToMainThread([] {
                AppendLogString(@"\nPress any key to close the application...\n", PrintColorCategory::Default);
                mIsPressAnyKeyActivated.store(true);
            });
        }

        void MacOSLoggerSubprocess::CreateWindow()
        {
            NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;

            mWindow = [[CubeLoggerSubprocessWindow alloc]
                initWithContentRect:NSMakeRect(0, 0, 1280, 720)
                styleMask:style
                backing:NSBackingStoreBuffered
                defer:NO
            ];
            [mWindow setTitle:@"CubeEngine Logger"];
            [mWindow setReleasedWhenClosed:NO];

            NSScrollView* scrollView = [[NSScrollView alloc] initWithFrame:[[mWindow contentView] bounds]];
            [scrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

            mTextView = [[CubeLoggerSubprocessTextView alloc] initWithFrame:[[mWindow contentView] bounds]];
            mTextView.editable = NO;
            mTextView.selectable = YES;

            [scrollView setDocumentView:mTextView];
            [scrollView setHasVerticalScroller:YES];

            [[mWindow contentView] addSubview:scrollView];

            mWindowDelegate = [[CubeLoggerSubprocessWindowDelegate alloc] init];
            [mWindow setDelegate:mWindowDelegate];

            // Show the window without making this app foreground. The parent's [NSApp activate]
            // would otherwise be ignored under macOS 14+ cooperative activation, leaving the
            // logger window above the main window. The logger window can still become key when
            // the user clicks it (e.g., during a breakpoint or to dismiss the prompt).
            [mWindow orderFrontRegardless];

            // Move to top-left of the main screen.
            NSScreen* screen = [mWindow screen] ?: [NSScreen mainScreen];
            NSRect screenFrame = [screen visibleFrame];
            NSRect windowFrame = [mWindow frame];

            NSPoint topLeft;
            topLeft.x = screenFrame.origin.x;
            topLeft.y = NSMaxY(screenFrame) - windowFrame.size.height;
            [mWindow setFrameOrigin:topLeft];
        }

        void MacOSLoggerSubprocess::OnAppWillTerminate()
        {
            if (mReaderThread.joinable())
            {
                mReaderThread.detach();
            }
        }

        void MacOSLoggerSubprocess::Run()
        {
            @autoreleasepool
            {
                [NSApplication sharedApplication];

                mAppDelegate = [[CubeLoggerSubprocessAppDelegate alloc] init];
                [NSApp setDelegate:mAppDelegate];

                // Install a minimal main menu so AppKit binds Cmd+Q to terminate:.
                NSMenu* mainMenu = [[NSMenu alloc] init];
                NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
                [mainMenu addItem:appMenuItem];
                NSMenu* appMenu = [[NSMenu alloc] init];
                [appMenu addItemWithTitle:@"Quit Logger"
                    action:@selector(terminate:)
                    keyEquivalent:@"q"
                ];
                [appMenuItem setSubmenu:appMenu];
                [NSApp setMainMenu:mainMenu];

                CreateWindow();

                mReaderThread = std::thread(&MacOSLoggerSubprocess::ReaderThreadMain);

                [NSApp run];
            }

            OnAppWillTerminate();

            exit(0);
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_MACOS_USE_LOGGER_WINDOW

#endif // CUBE_PLATFORM_MACOS
