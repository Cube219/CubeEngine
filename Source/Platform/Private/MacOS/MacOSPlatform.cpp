#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSPlatform.h"

#include "Checker.h"

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

        void MacOSPlatform::InitializeImpl()
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::ShutdownImpl()
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::InitWindowImpl(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY)
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::ShowWindowImpl()
        {
            NOT_IMPLEMENTED();
        }

        void* MacOSPlatform::AllocateImpl(Uint64 size)
        {
            NOT_IMPLEMENTED();
            return nullptr;
        }

        void MacOSPlatform::FreeImpl(void* ptr)
        {
            NOT_IMPLEMENTED();
        }

        void* MacOSPlatform::AllocateAlignedImpl(Uint64 size, Uint64 alignment)
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::FreeAlignedImpl(void* ptr)
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::StartLoopImpl()
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::FinishLoopImpl()
        {
            NOT_IMPLEMENTED();
        }

        void MacOSPlatform::SleepImpl(Uint32 time)
        {
            NOT_IMPLEMENTED();
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
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
