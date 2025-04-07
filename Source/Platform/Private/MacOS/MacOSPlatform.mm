#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSPlatform.h"

#include <unistd.h>

#include "Checker.h"
#include "MacOS/MacOSString.h"

@implementation CubeAppDelegate

- (void) applicationDidFinishLaunching:(NSNotification*) notification
{
}

- (void) applicationWillTerminate:(NSNotification* ) notification
{
    cube::platform::Platform::GetClosingEvent().Dispatch();
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication* ) sender
{
    return YES;
}

@end

namespace cube
{
    namespace platform
    {
        Event<void(KeyCode)> Platform::mKeyDownEvent;
        Event<void(KeyCode)> Platform::mKeyUpEvent;
        Event<void(MouseButton)> Platform::mMouseDownEvent;
        Event<void(MouseButton)> Platform::mMouseUpEvent;
        Event<void(int)> Platform::mMouseWheelEvent;
        Event<void(Int32, Int32)> Platform::mMousePositionEvent;

        Event<void()> Platform::mLoopEvent;
        Event<void(Uint32, Uint32)> Platform::mResizeEvent;
        Event<void(WindowActivatedState)> Platform::mActivatedEvent;
        Event<void()> Platform::mClosingEvent;
        
        PLATFORM_CLASS_DEFINITIONS(MacOSPlatform)

        NSWindow* MacOSPlatform::mWindow;
        CubeAppDelegate* MacOSPlatform::mDelegate;
        NSTask* MacOSPlatform::mDebugConsoleTerminalTask;

        std::thread MacOSPlatform::mMainLoopThread;
        bool MacOSPlatform::mIsFinished = false;

        void MacOSPlatform::InitializeImpl()
        {
            [NSApplication sharedApplication];

            mDelegate = [[CubeAppDelegate alloc] init];
            [NSApp setDelegate:mDelegate];

            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

#if CUBE_DEBUG
            mDebugConsoleTerminalTask = [[NSTask alloc] init];
            mDebugConsoleTerminalTask.launchPath = @"/bin/sh";
            mDebugConsoleTerminalTask.arguments = @[@"-c"];
            [mDebugConsoleTerminalTask launch];
#endif

            CreateMainMenu();
        }

        void MacOSPlatform::ShutdownImpl()
        {
            [mDebugConsoleTerminalTask release];
            [mDelegate release];
        }

        void MacOSPlatform::InitWindowImpl(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY)
        {
            NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;

            mWindow = [[NSWindow alloc]
                initWithContentRect:NSMakeRect(width, height, width, height)
                styleMask:style
                backing:NSBackingStoreBuffered
                defer:NO
            ];

            U8String u8Title;
            String_ConvertAndAppend(u8Title, title);
            [mWindow setTitle:[NSString stringWithUTF8String:u8Title.c_str()]];
        }

        void MacOSPlatform::ShowWindowImpl()
        {
            [mWindow makeKeyAndOrderFront:nil];
        }

        void* MacOSPlatform::AllocateImpl(Uint64 size)
        {
            NOT_IMPLEMENTED();
            return nullptr;
        }

        void MacOSPlatform::FreeImpl(void* ptr)
        {
            // NOT_IMPLEMENTED();
        }

        void* MacOSPlatform::AllocateAlignedImpl(Uint64 size, Uint64 alignment)
        {
            NOT_IMPLEMENTED();
            return nullptr;
        }

        void MacOSPlatform::FreeAlignedImpl(void* ptr)
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::StartLoopImpl()
        {
            mMainLoopThread = std::thread(&MacOSPlatform::MainLoop);

            [NSApp run];
            // Remain logic will not be executed after [NSApp run].
        }

        void MacOSPlatform::FinishLoopImpl()
        {
            mIsFinished = true;
            mMainLoopThread.join();
        }

        void MacOSPlatform::SleepImpl(float timeSec)
        {
            timespec ts;
            ts.tv_sec = static_cast<int>(timeSec);
            ts.tv_nsec = static_cast<int>((static_cast<double>(timeSec) - static_cast<int>(timeSec)) * 1000000000);
            nanosleep(&ts, nullptr);
        }

        void MacOSPlatform::ShowCursorImpl()
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::HideCursorImpl()
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::MoveCursorImpl(int x, int y)
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::GetCursorPosImpl(int& x, int& y)
        {
            NOT_IMPLEMENTED();
        }

        Uint32 MacOSPlatform::GetWindowWidthImpl()
        {
            NOT_IMPLEMENTED();
            return 0;
        }

        Uint32 MacOSPlatform::GetWindowHeightImpl()
        {
            NOT_IMPLEMENTED();
            return 0;
        }

        Uint32 MacOSPlatform::GetWindowPositionXImpl()
        {
            NOT_IMPLEMENTED();
            return 0;
        }

        Uint32 MacOSPlatform::GetWindowPositionYImpl()
        {
            NOT_IMPLEMENTED();
            return 0;
        }

        SharedPtr<DLib> MacOSPlatform::LoadDLibImpl(StringView path)
        {
            NOT_IMPLEMENTED();
            return nullptr;
        }

        void MacOSPlatform::CreateMainMenu()
        { @autoreleasepool {
            NSMenu* mainMenu = [[NSMenu alloc] init];
            [NSApp setMainMenu:mainMenu];

            NSMenuItem* menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
            [[NSApp mainMenu] addItem:menuItem];

            // TODO
        }}

        void MacOSPlatform::MainLoop()
        {
            while (1)
            {
                static int cnt = 0;
                CUBE_LOG(LogType::Info, Engine, "Run loop: {}", cnt);
                cnt++;

                if (mIsFinished)
                {
                    break;
                }

                mLoopEvent.Dispatch();
                Sleep(0.5f);
            }

            CUBE_LOG(LogType::Info, Engine, "End run main loop");
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
