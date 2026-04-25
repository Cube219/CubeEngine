#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSLoggerSubprocess.h"

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

namespace cube
{
    namespace platform
    {
        // === Parent-side static state ===
        NSTask* MacOSLoggerSubprocess::mLoggerTask = nil;
        NSFileHandle* MacOSLoggerSubprocess::mLoggerWriteHandle = nil;
        std::mutex MacOSLoggerSubprocess::mLoggerWriteMutex;
        std::atomic<bool> MacOSLoggerSubprocess::mIsAlive = false;
    } // namespace platform
} // namespace cube

namespace
{
    // === Child-side state ===
    // Only the subprocess process touches these (set up by Run() and accessed by the Obj-C
    // delegates / reader thread). File-local statics are sufficient.
    CubeLoggerSubprocessWindow* gWindow = nil;
    CubeLoggerSubprocessTextView* gTextView = nil;
    CubeLoggerSubprocessWindowDelegate* gWindowDelegate = nil;
    CubeLoggerSubprocessAppDelegate* gAppDelegate = nil;
    std::atomic<bool> gPromptActive = false;
    std::thread gReaderThread;

    void AppendAttributed(NSString* text, cube::platform::PrintColorCategory colorCategory)
    {
        if (gTextView == nil)
        {
            return;
        }

        NSColor* color = nil;
        switch (colorCategory)
        {
        case cube::platform::PrintColorCategory::Warning:
            color = [NSColor systemOrangeColor];
            break;
        case cube::platform::PrintColorCategory::Error:
            color = [NSColor systemRedColor];
            break;
        case cube::platform::PrintColorCategory::Default:
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
        NSTextStorage* storage = gTextView.textStorage;
        [storage appendAttributedString:attrText];
        [gTextView scrollRangeToVisible:NSMakeRange(storage.length, 0)];
    }

    bool ReadExact(int fd, void* buf, size_t len)
    {
        uint8_t* p = (uint8_t*)buf;
        size_t remaining = len;
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

    void ReaderThreadMain()
    {
        const int fd = STDIN_FILENO;
        while (true)
        {
            uint8_t header[5];
            if (!ReadExact(fd, header, sizeof(header)))
            {
                break;
            }

            uint8_t colorByte = header[0];
            uint32_t length = 0;
            length |= (uint32_t)header[1];
            length |= (uint32_t)header[2] << 8;
            length |= (uint32_t)header[3] << 16;
            length |= (uint32_t)header[4] << 24;

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

            cube::platform::PrintColorCategory category;
            switch (colorByte)
            {
            case 1:
                category = cube::platform::PrintColorCategory::Warning;
                break;
            case 2:
                category = cube::platform::PrintColorCategory::Error;
                break;
            default:
                category = cube::platform::PrintColorCategory::Default;
                break;
            }

            cube::platform::MacOSUtility::DispatchToMainThread([lineWithNewline, category] {
                AppendAttributed(lineWithNewline, category);
            });
        }

        // EOF or error. Show the "press any key" prompt on the main thread.
        cube::platform::MacOSUtility::DispatchToMainThread([] {
            AppendAttributed(@"\nPress any key to close the application...\n",
                             cube::platform::PrintColorCategory::Default);
            gPromptActive.store(true);
        });
    }

    void CreateWindow()
    {
        NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;

        gWindow = [[CubeLoggerSubprocessWindow alloc]
            initWithContentRect:NSMakeRect(0, 0, 1280, 720)
                      styleMask:style
                        backing:NSBackingStoreBuffered
                          defer:NO
        ];
        [gWindow setTitle:@"CubeEngine Logger"];
        [gWindow setReleasedWhenClosed:NO];

        NSScrollView* scrollView = [[NSScrollView alloc] initWithFrame:[[gWindow contentView] bounds]];
        [scrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

        gTextView = [[CubeLoggerSubprocessTextView alloc] initWithFrame:[[gWindow contentView] bounds]];
        gTextView.editable = NO;
        gTextView.selectable = YES;

        [scrollView setDocumentView:gTextView];
        [scrollView setHasVerticalScroller:YES];

        [[gWindow contentView] addSubview:scrollView];

        gWindowDelegate = [[CubeLoggerSubprocessWindowDelegate alloc] init];
        [gWindow setDelegate:gWindowDelegate];

        [gWindow makeKeyAndOrderFront:nil];

        // Move to top-left of the main screen.
        NSScreen* screen = [gWindow screen] ?: [NSScreen mainScreen];
        NSRect screenFrame = [screen visibleFrame];
        NSRect windowFrame = [gWindow frame];

        NSPoint topLeft;
        topLeft.x = screenFrame.origin.x;
        topLeft.y = NSMaxY(screenFrame) - windowFrame.size.height;
        [gWindow setFrameOrigin:topLeft];
    }
} // namespace

@implementation CubeLoggerSubprocessWindow

- (void)keyDown:(NSEvent*)event
{
    if (gPromptActive.load())
    {
        cube::platform::MacOSLoggerSubprocess::TerminateFromUI();
    }
}

@end

@implementation CubeLoggerSubprocessTextView

- (void)keyDown:(NSEvent*)event
{
    if (gPromptActive.load())
    {
        cube::platform::MacOSLoggerSubprocess::TerminateFromUI();
        return;
    }
    [super keyDown:event];
}

@end

@implementation CubeLoggerSubprocessWindowDelegate

- (BOOL)windowShouldClose:(NSWindow*)sender
{
    cube::platform::MacOSLoggerSubprocess::TerminateFromUI();
    return NO;
}

@end

@implementation CubeLoggerSubprocessAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
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
            mLoggerTask.standardInput = stdinPipe;
            // Inherit stdout/stderr so subprocess's std::cout lands in the same terminal.

            mLoggerTask.terminationHandler = ^(NSTask* task) {
                mIsAlive.store(false);

                // If the main app hasn't started its own shutdown, the user closed the
                // logger window first. Kick off the standard termination flow.
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

            // Wait until the subprocess exits (user presses a key, or it is already gone).
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
            Uint8 colorByte = static_cast<Uint8>(colorCategory);
            Uint32 length = static_cast<Uint32>(osStr.size());

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

        void MacOSLoggerSubprocess::TerminateFromUI()
        {
            // [NSApp terminate:] calls exit() directly without returning. Detach the reader thread
            // first so its std::thread destructor (run during __cxa_finalize_ranges) doesn't call
            // std::terminate on a still-joinable thread.
            if (gReaderThread.joinable())
            {
                gReaderThread.detach();
            }
            [NSApp terminate:nil];
        }

        void MacOSLoggerSubprocess::Run()
        {
            @autoreleasepool
            {
                [NSApplication sharedApplication];

                gAppDelegate = [[CubeLoggerSubprocessAppDelegate alloc] init];
                [NSApp setDelegate:gAppDelegate];

                CreateWindow();

                gReaderThread = std::thread(&ReaderThreadMain);

                [NSApp run];
            }

            // Belt-and-braces: if [NSApp run] ever returned without going through TerminateFromUI
            // (it currently always exits via terminate:), still detach before exit().
            if (gReaderThread.joinable())
            {
                gReaderThread.detach();
            }

            exit(0);
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
