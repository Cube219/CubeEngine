#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSPlatform.h"

#include <MacTypes.h>
#include <mach/mach.h>
#include <Carbon/Carbon.h>
#include <unistd.h>

#include "Checker.h"
#include "CubeString.h"
#include "MacOS/MacOSDebug.h"
#include "MacOS/MacOSDLib.h"
#include "MacOS/MacOSString.h"
#include "MacOS/MacOSUtility.h"

namespace cube
{
    namespace platform
    {
        namespace internal
        {
            struct MacOSPlatformPrivateAccessor
            {
                static const Array<KeyCode, MaxKeyCode>& GetKeyCodeMapping()
                {
                    return MacOSPlatform::mKeyCodeMapping;
                }

                static void SetWindowSize(Uint32 newWidth, Uint32 newHeight)
                {
                    MacOSPlatform::mWindowWidth = newWidth;
                    MacOSPlatform::mWindowHeight = newHeight;
                }

                static void SetWindowPosition(Int32 newX, Int32 newY)
                {
                    MacOSPlatform::mWindowPositionX = newX;
                    MacOSPlatform::mWindowPositionY = newY;
                }

                static void SetMousePosition(Int32 newX, Int32 newY)
                {
                    MacOSPlatform::mMousePositionX = newX;
                    MacOSPlatform::mMousePositionY = newY;
                }
            };
        } // namespace internal
    } // namespace platform
} // namespace cube

@implementation CubeWindow

- (void) keyDown:(NSEvent* ) event
{
    using namespace cube;
    using namespace cube::platform;

    KeyCode code = internal::MacOSPlatformPrivateAccessor::GetKeyCodeMapping()[event.keyCode];
    if (code != KeyCode::Invalid)
    {
        MacOSPlatform::QueueEvent([code]() {
            using namespace cube;

            MacOSPlatform::GetKeyDownEvent().Dispatch(code);
        });
    }
}

- (void) keyUp:(NSEvent* ) event
{
    using namespace cube;
    using namespace cube::platform;

    KeyCode code = internal::MacOSPlatformPrivateAccessor::GetKeyCodeMapping()[event.keyCode];
    if (code != KeyCode::Invalid)
    {
        MacOSPlatform::QueueEvent([code]() {
            using namespace cube;

            MacOSPlatform::GetKeyUpEvent().Dispatch(code);
        });
    }
}

- (void) mouseDown:(NSEvent* ) event
{
    using namespace cube::platform;

    MacOSPlatform::QueueEvent([]() {
        using namespace cube;
        MacOSPlatform::GetMouseDownEvent().Dispatch(MouseButton::Left);
    });
}

- (void) mouseUp:(NSEvent* ) event
{
    using namespace cube::platform;

    MacOSPlatform::QueueEvent([]() {
        using namespace cube;
        MacOSPlatform::GetMouseUpEvent().Dispatch(MouseButton::Left);
    });
}

- (void) rightMouseDown:(NSEvent* ) event
{
    using namespace cube::platform;

    MacOSPlatform::QueueEvent([]() {
        using namespace cube;

        MacOSPlatform::GetMouseDownEvent().Dispatch(MouseButton::Right);
    });
}

- (void) rightMouseUp:(NSEvent* ) event
{
    using namespace cube::platform;

    MacOSPlatform::QueueEvent([]() {
        using namespace cube;

        MacOSPlatform::GetMouseUpEvent().Dispatch(MouseButton::Right);
    });
}

- (void) otherMouseDown:(NSEvent* ) event
{
    using namespace cube::platform;

    MacOSPlatform::QueueEvent([]() {
        using namespace cube;

        MacOSPlatform::GetMouseDownEvent().Dispatch(MouseButton::Middle);
    });
}

- (void) otherMouseUp:(NSEvent* ) event
{
    using namespace cube::platform;

    MacOSPlatform::QueueEvent([]() {
        using namespace cube;

        MacOSPlatform::GetMouseUpEvent().Dispatch(MouseButton::Middle);
    });
}

- (void) scrollWheel:(NSEvent* ) event
{
    using namespace cube::platform;

    MacOSPlatform::QueueEvent([delta = event.scrollingDeltaY]() {
        using namespace cube;

        MacOSPlatform::GetMouseWheelEvent().Dispatch(static_cast<int>(delta));
    });
}

namespace cube { namespace platform { namespace internal
{
    void ProcessMouseMoveEvent(NSEvent* event, NSWindow* window)
    {
        using namespace cube::platform;

        NSPoint location = [event locationInWindow];
        int x = static_cast<Int32>(location.x);
        int y = static_cast<Int32>(window.contentView.frame.size.height - location.y); // Flip y

        MacOSPlatform::QueueEvent([x, y]() {
            using namespace cube;

            MacOSPlatformPrivateAccessor::SetMousePosition(x, y);
            MacOSPlatform::GetMousePositionEvent().Dispatch(x, y);
        });
    }
}}} // namespace cube::platform::internal

- (void) mouseMoved:(NSEvent* ) event
{
    cube::platform::internal::ProcessMouseMoveEvent(event, self);
}

- (void) mouseDragged:(NSEvent* ) event
{
    cube::platform::internal::ProcessMouseMoveEvent(event, self);
}

- (void) rightMouseDragged:(NSEvent* ) event
{
    cube::platform::internal::ProcessMouseMoveEvent(event, self);
}

- (void) otherMouseDragged:(NSEvent* ) event
{
    cube::platform::internal::ProcessMouseMoveEvent(event, self);
}

@end

@implementation CubeAppDelegate

- (void) applicationDidFinishLaunching:(NSNotification* ) notification
{
}

- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication* ) sender
{
    using namespace cube::platform;

    // Cancel termination if the engine/platform is not shutdown
    // Termination will be called after shutdown (in platform::Shutdown)
    if (!MacOSPlatform::IsApplicationClosed())
    {
        cube::platform::Platform::GetClosingEvent().Dispatch();

        return NSApplicationTerminateReply::NSTerminateCancel;
    }
    return NSApplicationTerminateReply::NSTerminateNow;
}

- (void) applicationWillTerminate:(NSNotification* ) notification
{
    using namespace cube::platform;

    MacOSPlatform::LastCleanup();
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication* ) sender
{
    return YES;
}

@end

@implementation CubeWindowDelegate

- (void) windowDidBecomeMain:(NSNotification* ) notification
{
    using namespace cube::platform;
    Platform::GetActivatedEvent().Dispatch(WindowActivatedState::Active);
}

- (void) windowDidResignMain:(NSNotification* ) notification
{
    using namespace cube::platform;
    Platform::GetActivatedEvent().Dispatch(WindowActivatedState::Inactive);
}

- (BOOL) windowShouldClose:(NSWindow* ) window
{
    // Just call termination. Remain logic will be processed in applicationShouldTerminate.
    [NSApp terminate:nil];

    return NO;
}

- (void) windowDidResize:(NSNotification* ) notification
{
    using namespace cube::platform;

    NSWindow* window = notification.object;
    cube::Uint32 newWidth = static_cast<cube::Uint32>(window.contentView.frame.size.width);
    cube::Uint32 newHeight = static_cast<cube::Uint32>(window.contentView.frame.size.height);

    MacOSPlatform::QueueEvent([newWidth, newHeight]() {
        using namespace cube;

        internal::MacOSPlatformPrivateAccessor::SetWindowSize(newWidth, newHeight);
        MacOSPlatform::GetResizeEvent().Dispatch(newWidth, newHeight);
    });
}

- (void) windowDidMove:(NSNotification* ) notification
{
    using namespace cube::platform;

    NSWindow* window = notification.object;
    NSRect screenFrame = [[NSScreen mainScreen] frame];

    cube::Int32 newX = static_cast<cube::Int32>(window.frame.origin.x);
    cube::Int32 newY = static_cast<cube::Int32>(NSMaxY(screenFrame) - NSMaxY(window.frame)); // Flip y

    MacOSPlatform::QueueEvent([newX, newY]() {
        using namespace cube;

        internal::MacOSPlatformPrivateAccessor::SetWindowPosition(newX, newY);
    });
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

        Array<KeyCode, MaxKeyCode> MacOSPlatform::mKeyCodeMapping;

        std::atomic<bool> MacOSPlatform::mIsApplicationClosed = false;
        CubeWindow* MacOSPlatform::mWindow;
        CubeAppDelegate* MacOSPlatform::mAppDelegate;
        CubeWindowDelegate* MacOSPlatform::mWindowDelegate;
        Uint32 MacOSPlatform::mWindowWidth;
        Uint32 MacOSPlatform::mWindowHeight;
        Int32 MacOSPlatform::mWindowPositionX;
        Int32 MacOSPlatform::mWindowPositionY;

        bool MacOSPlatform::mIsMouseHidden = false;
        Int32 MacOSPlatform::mMousePositionX;
        Int32 MacOSPlatform::mMousePositionY;

        id MacOSPlatform::mModifierEventHandler;
        bool MacOSPlatform::mIsCapsLockKeyPressed = false;
        bool MacOSPlatform::mIsShiftKeyPressed = false;
        bool MacOSPlatform::mIsControlKeyPressed = false;
        bool MacOSPlatform::mIsOptionKeyPressed = false;
        bool MacOSPlatform::mIsCommandKeyPressed = false;
        bool MacOSPlatform::mIsFunctionKeyPressed = false;

        std::function<void()> MacOSPlatform::mEngineInitializeFunction;
        std::function<void()> MacOSPlatform::mEngineShutdownFunction;
        Signal MacOSPlatform::mRunMainLoopSignal;
        std::thread MacOSPlatform::mMainLoopThread;
        bool MacOSPlatform::mIsLoopStarted = false;
        bool MacOSPlatform::mIsLoopFinished = false;

        std::mutex MacOSPlatform::mEventQueueMutex;
        Vector<std::function<void()>> MacOSPlatform::mEventQueue;

        void MacOSPlatform::InitializeImpl()
        {
            if (![NSThread isMainThread])
            {
                // Called in main loop thread and initialization was already finished in main thread.
                return;
            }

            InitializeKeyCodeMapping();
            // Register modifier key event
            mModifierEventHandler = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskFlagsChanged handler:^(NSEvent* event)
            {
                ReceiveModifierKeyEvent(event.modifierFlags);
                return event;
            }];

            [NSApplication sharedApplication];

            mAppDelegate = [[CubeAppDelegate alloc] init];
            [NSApp setDelegate:mAppDelegate];

            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

            CreateMainMenu();

#if CUBE_DEBUG
            MacOSDebug::CreateAndShowLoggerWindow();
#endif
            mMainLoopThread = std::thread(&MacOSPlatform::MainLoop);

            [NSApp run];
            // Remain logic will not be executed after [NSApp run].
        }

        void MacOSPlatform::ShutdownImpl()
        {
            CHECK_NOT_MAIN_THREAD();

            mIsApplicationClosed = true;

            MacOSUtility::DispatchToMainThread([] {
                // Wait until finishing shutdown
                mMainLoopThread.join();

                if (mWindow)
                {
                    [mWindow close];
                }

                [NSEvent removeMonitor:mModifierEventHandler];

                if (MacOSDebug::IsLoggerWindowCreated())
                {
                    MacOSDebug::AppendLogText(@"Press any key to close the application...", PrintColorCategory::Default);
                }
                else
                {
                    [NSApp terminate:nil];
                }
            });
        }

        void MacOSPlatform::InitWindowImpl(StringView title, Uint32 width, Uint32 height, Int32 posX, Int32 posY)
        {
            MacOSUtility::DispatchToMainThreadAndWait([&]{
                mWindowWidth = width;
                mWindowHeight = height;
                mWindowPositionX = posX;
                mWindowPositionY = posY;

                NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;

                // Unlike Windows, y position starts at the bottom of screen.
                // So, flip it for consistency.
                NSRect screenFrame = [[NSScreen mainScreen] frame];
                posY = NSMaxY(screenFrame) - height - posY;

                mWindow = [[CubeWindow alloc]
                    initWithContentRect:NSMakeRect(posX, posY, width, height)
                    styleMask:style
                    backing:NSBackingStoreBuffered
                    defer:NO
                ];

                [mWindow setTitle:String_Convert<NSString*>(title)];
                [mWindow setAcceptsMouseMovedEvents:YES];

                mWindowDelegate = [[CubeWindowDelegate alloc] init];
                [mWindow setDelegate:mWindowDelegate];
            });
        }

        void MacOSPlatform::ShowWindowImpl()
        {
            MacOSUtility::DispatchToMainThreadAndWait([&]{
                [mWindow makeKeyAndOrderFront:nil];
            });
        }

        void MacOSPlatform::ChangeWindowTitleImpl(StringView title)
        {
            MacOSUtility::DispatchToMainThread([nsTitle = String_Convert<NSString*>(title)]() {
                [mWindow setTitle:nsTitle];
            });
        }

        void* MacOSPlatform::AllocateImpl(Uint64 size)
        {
            return malloc(size);
        }

        void MacOSPlatform::FreeImpl(void* ptr)
        {
            free(ptr);
        }

        void* MacOSPlatform::AllocateAlignedImpl(Uint64 size, Uint64 alignment)
        {
            return aligned_alloc(alignment, size);
        }

        void MacOSPlatform::FreeAlignedImpl(void* ptr)
        {
            free(ptr);
        }

        void MacOSPlatform::SetEngineInitializeFunctionImpl(std::function<void()> function)
        {
            mEngineInitializeFunction = function;
        }

        void MacOSPlatform::SetEngineShutdownFunctionImpl(std::function<void()> function)
        {
            mEngineShutdownFunction = function;
        }

        void MacOSPlatform::StartLoopImpl()
        {
            mIsLoopStarted = true;
            mRunMainLoopSignal.Notify();
        }

        void MacOSPlatform::FinishLoopImpl()
        {
            mIsLoopStarted = false;
            mIsLoopFinished = true;
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
            // Prevent call hide/unhide multiple times because they are stack-based.
            if (!mIsMouseHidden)
            {
                [NSCursor hide];
                mIsMouseHidden = true;
            }
        }

        void MacOSPlatform::HideCursorImpl()
        {
            if (mIsMouseHidden)
            {
                [NSCursor unhide];
                mIsMouseHidden = false;
            }
        }

        void MacOSPlatform::MoveCursorImpl(int x, int y)
        {
            y = mWindow.contentView.frame.size.height - y; // Flip y
            NSPoint screenLocation = [mWindow convertPointToScreen:NSMakePoint(x, y)];

            // CG's y coordinate moves from top to bottom. So flip again.
            float cgY = [[NSScreen mainScreen] frame].size.height - screenLocation.y;
            CGWarpMouseCursorPosition(CGPointMake(screenLocation.x, cgY));
        }

        void MacOSPlatform::GetCursorPosImpl(int& x, int& y)
        {
            NSPoint location = [mWindow convertPointFromScreen:[NSEvent mouseLocation]];

            x = static_cast<int>(location.x);
            y = static_cast<int>(mWindow.contentView.frame.size.height - location.y); // Flip y
        }

        Uint32 MacOSPlatform::GetWindowWidthImpl()
        {
            return mWindowWidth;
        }

        Uint32 MacOSPlatform::GetWindowHeightImpl()
        {
            return mWindowHeight;
        }

        Int32 MacOSPlatform::GetWindowPositionXImpl()
        {
            return mWindowPositionX;
        }

        Int32 MacOSPlatform::GetWindowPositionYImpl()
        {
            return mWindowPositionY;
        }

        SharedPtr<DLib> MacOSPlatform::LoadDLibImpl(StringView path)
        {
            auto res = std::make_shared<MacOSDLib>(path);
            if (res->GetHandle() == nullptr)
            {
                return nullptr;
            }

            return res;
        }

        CubeWindow* MacOSPlatform::GetWindow()
        {
            return mWindow;
        }

        bool MacOSPlatform::IsMainWindowCreated()
        {
            return mWindow != nullptr;
        }
        
        void MacOSPlatform::CloseMainWindow()
        {
            [mWindow close];
        }

        void MacOSPlatform::LastCleanup()
        {
            CHECK_MAIN_THREAD();

            MacOSDebug::CloseAndDestroyLoggerWindow();

            [mWindowDelegate release];

            [mAppDelegate release];
        }

        void MacOSPlatform::ForceTerminateMainLoopThread()
        {
            if (mIsLoopStarted)
            {
                pthread_t pThread = mMainLoopThread.native_handle();
                thread_t machThread;
                pthread_threadid_np(pThread, (uint64_t*)&machThread);
                thread_terminate(machThread);

                mMainLoopThread.detach();
            }
            mIsLoopStarted = false;
            mIsLoopFinished = true;
        }

        void MacOSPlatform::QueueEvent(std::function<void()> eventFunction)
        {
            std::unique_lock<std::mutex> lock(mEventQueueMutex);

            mEventQueue.push_back(std::move(eventFunction));
        }

        void MacOSPlatform::InitializeKeyCodeMapping()
        {
            mKeyCodeMapping[kVK_ANSI_A] = KeyCode::A;
            mKeyCodeMapping[kVK_ANSI_S] = KeyCode::S;
            mKeyCodeMapping[kVK_ANSI_D] = KeyCode::D;
            mKeyCodeMapping[kVK_ANSI_F] = KeyCode::F;
            mKeyCodeMapping[kVK_ANSI_H] = KeyCode::H;
            mKeyCodeMapping[kVK_ANSI_G] = KeyCode::G;
            mKeyCodeMapping[kVK_ANSI_Z] = KeyCode::Z;
            mKeyCodeMapping[kVK_ANSI_X] = KeyCode::X;
            mKeyCodeMapping[kVK_ANSI_C] = KeyCode::C;
            mKeyCodeMapping[kVK_ANSI_V] = KeyCode::V;
            mKeyCodeMapping[kVK_ANSI_B] = KeyCode::B;
            mKeyCodeMapping[kVK_ANSI_Q] = KeyCode::Q;
            mKeyCodeMapping[kVK_ANSI_W] = KeyCode::W;
            mKeyCodeMapping[kVK_ANSI_E] = KeyCode::E;
            mKeyCodeMapping[kVK_ANSI_R] = KeyCode::R;
            mKeyCodeMapping[kVK_ANSI_Y] = KeyCode::Y;
            mKeyCodeMapping[kVK_ANSI_T] = KeyCode::T;
            mKeyCodeMapping[kVK_ANSI_1] = KeyCode::Num1;
            mKeyCodeMapping[kVK_ANSI_2] = KeyCode::Num2;
            mKeyCodeMapping[kVK_ANSI_3] = KeyCode::Num3;
            mKeyCodeMapping[kVK_ANSI_4] = KeyCode::Num4;
            mKeyCodeMapping[kVK_ANSI_6] = KeyCode::Num6;
            mKeyCodeMapping[kVK_ANSI_5] = KeyCode::Num5;
            mKeyCodeMapping[kVK_ANSI_Equal] = KeyCode::Plus;
            mKeyCodeMapping[kVK_ANSI_9] = KeyCode::Num9;
            mKeyCodeMapping[kVK_ANSI_7] = KeyCode::Num7;
            mKeyCodeMapping[kVK_ANSI_Minus] = KeyCode::Minus;
            mKeyCodeMapping[kVK_ANSI_8] = KeyCode::Num8;
            mKeyCodeMapping[kVK_ANSI_0] = KeyCode::Num0;
            mKeyCodeMapping[kVK_ANSI_RightBracket] = KeyCode::RightBracket;
            mKeyCodeMapping[kVK_ANSI_O] = KeyCode::O;
            mKeyCodeMapping[kVK_ANSI_U] = KeyCode::U;
            mKeyCodeMapping[kVK_ANSI_LeftBracket] = KeyCode::LeftBracket;
            mKeyCodeMapping[kVK_ANSI_I] = KeyCode::I;
            mKeyCodeMapping[kVK_ANSI_P] = KeyCode::P;
            mKeyCodeMapping[kVK_ANSI_L] = KeyCode::L;
            mKeyCodeMapping[kVK_ANSI_J] = KeyCode::J;
            mKeyCodeMapping[kVK_ANSI_Quote] = KeyCode::Quote;
            mKeyCodeMapping[kVK_ANSI_K] = KeyCode::K;
            mKeyCodeMapping[kVK_ANSI_Semicolon] = KeyCode::Semicolon;
            mKeyCodeMapping[kVK_ANSI_Backslash] = KeyCode::BackSlash;
            mKeyCodeMapping[kVK_ANSI_Comma] = KeyCode::Comma;
            mKeyCodeMapping[kVK_ANSI_Slash] = KeyCode::Slash;
            mKeyCodeMapping[kVK_ANSI_N] = KeyCode::N;
            mKeyCodeMapping[kVK_ANSI_M] = KeyCode::M;
            // mKeyCodeMapping[kVK_ANSI_Period] = ;
            // mKeyCodeMapping[kVK_ANSI_Grave] = ;
            mKeyCodeMapping[kVK_ANSI_KeypadDecimal] = KeyCode::NumPadDecimal;
            mKeyCodeMapping[kVK_ANSI_KeypadMultiply] = KeyCode::NumPadMultiply;
            mKeyCodeMapping[kVK_ANSI_KeypadPlus] = KeyCode::NumPadAdd;
            mKeyCodeMapping[kVK_ANSI_KeypadClear] = KeyCode::NumLock;
            mKeyCodeMapping[kVK_ANSI_KeypadDivide] = KeyCode::NumPadDivide;
            // mKeyCodeMapping[kVK_ANSI_KeypadEnter] = ;
            mKeyCodeMapping[kVK_ANSI_KeypadMinus] = KeyCode::NumPadSubtract;
            // mKeyCodeMapping[kVK_ANSI_KeypadEquals] = ;
            mKeyCodeMapping[kVK_ANSI_Keypad0] = KeyCode::NumPad0;
            mKeyCodeMapping[kVK_ANSI_Keypad1] = KeyCode::NumPad1;
            mKeyCodeMapping[kVK_ANSI_Keypad2] = KeyCode::NumPad2;
            mKeyCodeMapping[kVK_ANSI_Keypad3] = KeyCode::NumPad3;
            mKeyCodeMapping[kVK_ANSI_Keypad4] = KeyCode::NumPad4;
            mKeyCodeMapping[kVK_ANSI_Keypad5] = KeyCode::NumPad5;
            mKeyCodeMapping[kVK_ANSI_Keypad6] = KeyCode::NumPad6;
            mKeyCodeMapping[kVK_ANSI_Keypad7] = KeyCode::NumPad7;
            mKeyCodeMapping[kVK_ANSI_Keypad8] = KeyCode::NumPad8;
            mKeyCodeMapping[kVK_ANSI_Keypad9] = KeyCode::NumPad9;

            mKeyCodeMapping[kVK_Return] = KeyCode::Enter;
            mKeyCodeMapping[kVK_Tab] = KeyCode::Tab;
            mKeyCodeMapping[kVK_Space] = KeyCode::Space;
            mKeyCodeMapping[kVK_Delete] = KeyCode::Backspace;
            mKeyCodeMapping[kVK_Escape] = KeyCode::Escape;
            mKeyCodeMapping[kVK_Command] = KeyCode::Windows;
            mKeyCodeMapping[kVK_Shift] = KeyCode::Shift;
            mKeyCodeMapping[kVK_CapsLock] = KeyCode::CapsLock;
            mKeyCodeMapping[kVK_Option] = KeyCode::Alt;
            mKeyCodeMapping[kVK_Control] = KeyCode::Control;
            mKeyCodeMapping[kVK_RightCommand] = KeyCode::RightWindows;
            mKeyCodeMapping[kVK_RightShift] = KeyCode::RightShift;
            mKeyCodeMapping[kVK_RightOption] = KeyCode::RightAlt;
            mKeyCodeMapping[kVK_RightControl] = KeyCode::RightControl;
            // mKeyCodeMapping[kVK_Function] = ;
            mKeyCodeMapping[kVK_F17] = KeyCode::F17;
            // mKeyCodeMapping[kVK_VolumeUp] = ;
            // mKeyCodeMapping[kVK_VolumeDown] = ;
            // mKeyCodeMapping[kVK_Mute] = ;
            mKeyCodeMapping[kVK_F18] = KeyCode::F18;
            mKeyCodeMapping[kVK_F19] = KeyCode::F19;
            mKeyCodeMapping[kVK_F20] = KeyCode::F20;
            mKeyCodeMapping[kVK_F5] = KeyCode::F5;
            mKeyCodeMapping[kVK_F6] = KeyCode::F6;
            mKeyCodeMapping[kVK_F7] = KeyCode::F7;
            mKeyCodeMapping[kVK_F3] = KeyCode::F3;
            mKeyCodeMapping[kVK_F8] = KeyCode::F8;
            mKeyCodeMapping[kVK_F9] = KeyCode::F9;
            mKeyCodeMapping[kVK_F11] = KeyCode::F11;
            mKeyCodeMapping[kVK_F13] = KeyCode::F13;
            mKeyCodeMapping[kVK_F16] = KeyCode::F16;
            mKeyCodeMapping[kVK_F14] = KeyCode::F14;
            mKeyCodeMapping[kVK_F10] = KeyCode::F10;
            mKeyCodeMapping[kVK_ContextualMenu] = KeyCode::AppKey;
            mKeyCodeMapping[kVK_F12] = KeyCode::F12;
            mKeyCodeMapping[kVK_F15] = KeyCode::F15;
            mKeyCodeMapping[kVK_Help] = KeyCode::Insert;
            mKeyCodeMapping[kVK_Home] = KeyCode::Home;
            mKeyCodeMapping[kVK_PageUp] = KeyCode::PageUp;
            mKeyCodeMapping[kVK_ForwardDelete] = KeyCode::Delete;
            mKeyCodeMapping[kVK_F4] = KeyCode::F4;
            mKeyCodeMapping[kVK_End] = KeyCode::End;
            mKeyCodeMapping[kVK_F2] = KeyCode::F2;
            mKeyCodeMapping[kVK_PageDown] = KeyCode::PageDown;
            mKeyCodeMapping[kVK_F1] = KeyCode::F1;
            mKeyCodeMapping[kVK_LeftArrow] = KeyCode::LeftArrow;
            mKeyCodeMapping[kVK_RightArrow] = KeyCode::RightArrow;
            mKeyCodeMapping[kVK_DownArrow] = KeyCode::DownArrow;
            mKeyCodeMapping[kVK_UpArrow] = KeyCode::UpArrow;
        }

        void MacOSPlatform::ReceiveModifierKeyEvent(NSEventModifierFlags flags)
        {
            CHECK_MAIN_THREAD()

#define PROCESS_MODIFIER_KEY(variable, eventFlag, kVK_keyCode) \
    if (variable != ((flags & eventFlag) > 0)) \
    { \
        KeyCode code = internal::MacOSPlatformPrivateAccessor::GetKeyCodeMapping()[kVK_keyCode]; \
        if (code != KeyCode::Invalid) \
        { \
            variable = ((flags & eventFlag) > 0); \
            std::function<void()> eventFunc; \
            if (variable) \
            { \
                eventFunc = [code]() { MacOSPlatform::GetKeyDownEvent().Dispatch(code); }; \
            } \
            else \
            { \
                eventFunc = [code]() { MacOSPlatform::GetKeyUpEvent().Dispatch(code); }; \
            } \
            MacOSPlatform::QueueEvent(eventFunc); \
        } \
    }

            PROCESS_MODIFIER_KEY(mIsCapsLockKeyPressed, NSEventModifierFlagCapsLock, kVK_CapsLock)
            PROCESS_MODIFIER_KEY(mIsShiftKeyPressed, NSEventModifierFlagShift, kVK_Shift)
            PROCESS_MODIFIER_KEY(mIsControlKeyPressed, NSEventModifierFlagControl, kVK_Control)
            PROCESS_MODIFIER_KEY(mIsOptionKeyPressed, NSEventModifierFlagOption, kVK_Option)
            PROCESS_MODIFIER_KEY(mIsCommandKeyPressed, NSEventModifierFlagCommand, kVK_Command)
            PROCESS_MODIFIER_KEY(mIsFunctionKeyPressed, NSEventModifierFlagFunction, kVK_Function)

#undef PROCESS_MODIFIER_KEY
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
            if (mEngineInitializeFunction)
            {
                mEngineInitializeFunction();
                mEngineInitializeFunction = nullptr;
            }

            MacOSUtility::DispatchToMainThread([]{
                Platform::StartLoop();
            });

            mRunMainLoopSignal.Wait();
            while (1)
            {
                if (mIsLoopFinished)
                {
                    break;
                }

                Vector<std::function<void()>> eventFunctions;
                {
                    std::unique_lock<std::mutex> lock(mEventQueueMutex);
                    eventFunctions = std::move(mEventQueue);
                    mEventQueue.empty();
                }

                for (auto& func : eventFunctions)
                {
                    func();
                }

                mLoopEvent.Dispatch();
            }

            if (mEngineShutdownFunction)
            {
                mEngineShutdownFunction();
                mEngineShutdownFunction = nullptr;
            }
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
