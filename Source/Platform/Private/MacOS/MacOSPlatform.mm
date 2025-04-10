#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSPlatform.h"

#include <MacTypes.h>
#include <unistd.h>

#include "Checker.h"
#include "MacOS/MacOSDebug.h"
#include "MacOS/MacOSString.h"
#include "MacOS/MacOSUtility.h"

@implementation CubeAppDelegate

- (void) applicationDidFinishLaunching:(NSNotification* ) notification
{
}

- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication* ) sender
{
    if (cube::platform::MacOSDebug::IsLoggerWindowCreated()
        && !cube::platform::MacOSPlatform::IsApplicationClosed())
    {
        // Prevent termination if the logger window is created to see the logs
        cube::platform::MacOSPlatform::CloseMainWindow();
        return NSApplicationTerminateReply::NSTerminateCancel;
    }
    return NSApplicationTerminateReply::NSTerminateNow;
}

- (void) applicationWillTerminate:(NSNotification* ) notification
{
    cube::platform::Platform::GetClosingEvent().Dispatch();
    cube::platform::MacOSDebug::CloseAndDestroyLoggerWindow();
    cube::platform::MacOSPlatform::Cleanup();
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication* ) sender
{
    return YES;
}

@end

@implementation CubeWindowDelegate

- (void) windowWillClose:(NSNotification* ) notification;
{
    cube::platform::Platform::GetClosingEvent().Dispatch();
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

        bool MacOSPlatform::mIsApplicationClosed = false;
        NSWindow* MacOSPlatform::mWindow;
        CubeAppDelegate* MacOSPlatform::mAppDelegate;
        CubeWindowDelegate* MacOSPlatform::mWindowDelegate;

        std::thread MacOSPlatform::mMainLoopThread;
        bool MacOSPlatform::mIsFinished = false;

        void MacOSPlatform::InitializeImpl()
        {
            [NSApplication sharedApplication];

            mAppDelegate = [[CubeAppDelegate alloc] init];
            [NSApp setDelegate:mAppDelegate];

            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

            CreateMainMenu();
            
#if CUBE_DEBUG
            MacOSDebug::CreateAndShowLoggerWindow();
#endif
        }

        void MacOSPlatform::ShutdownImpl()
        {
            mIsApplicationClosed = true;
            if (MacOSDebug::IsLoggerWindowCreated())
            {
                MacOSUtility::DispatchToMainThread([] {
                    MacOSDebug::AppendLogText(@"Press any key to close the application...", PrintColorCategory::Default);
                });
            }
            // Not run clean up logic at this. It runs in termination and termination will be executed when all window are closed.
        }

        void MacOSPlatform::InitWindowImpl(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY)
        {
            CHECK_MAIN_THREAD()
            
            NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;

            // Unlike Windows, y position starts at the bottom of screen.
            // So, flip it for consistency.
            NSRect screenFrame = [[NSScreen mainScreen] frame];
            posY = NSMaxY(screenFrame) - height - posY;
            
            mWindow = [[NSWindow alloc]
                initWithContentRect:NSMakeRect(posX, posY, width, height)
                styleMask:style
                backing:NSBackingStoreBuffered
                defer:NO
            ];

            U8String u8Title;
            String_ConvertAndAppend(u8Title, title);
            [mWindow setTitle:[NSString stringWithUTF8String:u8Title.c_str()]];

            mWindowDelegate = [[CubeWindowDelegate alloc] init];
            [mWindow setDelegate:mWindowDelegate];
        }

        void MacOSPlatform::ShowWindowImpl()
        {
            CHECK_MAIN_THREAD()

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

        bool MacOSPlatform::IsMainWindowCreated()
        {
            return mWindow != nullptr;
        }
        
        void MacOSPlatform::CloseMainWindow()
        {
            [mWindow close];
        }

        void MacOSPlatform::Cleanup()
        {
            [mWindowDelegate release];
            [mWindow release];

            [mAppDelegate release];
        }

        void MacOSPlatform::CreateMainMenu()
        {
            CHECK_MAIN_THREAD()

            @autoreleasepool {
                NSString* title = @"CubeEngine";

                NSMenu* mainMenu = [[NSMenu alloc] init];
                [NSApp setMainMenu:mainMenu];

                NSMenu *mainSubMenu = [[NSMenu alloc] initWithTitle:@""];

                [mainSubMenu
                    addItemWithTitle:[@"Hide " stringByAppendingString:title]
                    action:@selector(hide:)
                    keyEquivalent:@"h"
                ];
                [[mainSubMenu
                    addItemWithTitle:@"Hide Others"
                    action:@selector(hideOtherApplications:)
                    keyEquivalent:@"h"
                ] setKeyEquivalentModifierMask:NSEventModifierFlagOption|NSEventModifierFlagCommand];
                [mainSubMenu
                    addItemWithTitle:@"Show All"
                    action:@selector(unhideAllApplications:)
                    keyEquivalent:@""
                ];
                [mainSubMenu addItem:[NSMenuItem separatorItem]];
                [mainSubMenu
                    addItemWithTitle:[@"Quit " stringByAppendingString:title]
                    action:@selector(terminate:)
                    keyEquivalent:@"q"
                ];

                NSMenuItem* mainMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
                [mainMenuItem setSubmenu:mainSubMenu];
                [[NSApp mainMenu] addItem:mainMenuItem];
            }
        }

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
