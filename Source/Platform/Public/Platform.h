#pragma once

#include "PlatformHeader.h"

#include "Checker.h"
#include "DLib.h"
#include "Event.h"
#include "KeyCode.h"
#include "Mouse.h"

namespace cube
{
    namespace platform
    {
        enum class WindowActivatedState : Uint8
        {
            Active, ClickActive, Inactive
        };

        class CUBE_PLATFORM_EXPORT BasePlatform
        {
        public:
            static void Initialize() { NOT_IMPLEMENTED() }
            static void Shutdown() { NOT_IMPLEMENTED() }

            static void InitWindow(StringView title, Uint32 width, Uint32 height, Int32 posX, Int32 posY) { NOT_IMPLEMENTED() }
            static void ShowWindow() { NOT_IMPLEMENTED() }
            static void ChangeWindowTitle(StringView title) { NOT_IMPLEMENTED() }

            static void* Allocate(Uint64 size) { NOT_IMPLEMENTED() return nullptr; }
            static void Free(void* ptr) { NOT_IMPLEMENTED() }
            static void* AllocateAligned(Uint64 size, Uint64 alignment) { NOT_IMPLEMENTED() return nullptr; }
            static void FreeAligned(void* ptr) { NOT_IMPLEMENTED() }

            static void SetEngineInitializeFunction(std::function<void()> function) { NOT_IMPLEMENTED() }
            static void SetEngineShutdownFunction(std::function<void()> function) { NOT_IMPLEMENTED() }
            static void StartLoop() { NOT_IMPLEMENTED() }
            static void FinishLoop() { NOT_IMPLEMENTED() }
            static void Sleep(float timeSec) { NOT_IMPLEMENTED() }

            static void ShowCursor() { NOT_IMPLEMENTED() }
            static void HideCursor() { NOT_IMPLEMENTED() }
            static void MoveCursor(int x, int y) { NOT_IMPLEMENTED() }
            static void GetCursorPos(int& x, int& y) { NOT_IMPLEMENTED() }

            static Uint32 GetWindowWidth() { NOT_IMPLEMENTED() return 0; }
            static Uint32 GetWindowHeight() { NOT_IMPLEMENTED() return 0; }
            static Int32 GetWindowPositionX() { NOT_IMPLEMENTED() return 0; }
            static Int32 GetWindowPositionY() { NOT_IMPLEMENTED() return 0; }

            static SharedPtr<BaseDLib> LoadDLib(StringView path) { NOT_IMPLEMENTED() return nullptr; }

            static Event<void(KeyCode)>& GetKeyDownEvent() { return mKeyDownEvent; }
            static Event<void(KeyCode)>& GetKeyUpEvent() { return mKeyUpEvent; }
            static Event<void(MouseButton)>& GetMouseDownEvent() { return mMouseDownEvent; }
            static Event<void(MouseButton)>& GetMouseUpEvent() { return mMouseUpEvent; }
            static Event<void(int)>& GetMouseWheelEvent() { return mMouseWheelEvent; }
            static Event<void(Int32, Int32)>& GetMousePositionEvent() { return mMousePositionEvent; }
            static Event<void()>& GetLoopEvent() { return mLoopEvent; }
            static Event<void(Uint32, Uint32)>& GetResizeEvent() { return mResizeEvent; }
            static Event<void(WindowActivatedState)>& GetActivatedEvent() { return mActivatedEvent; }
            static Event<void()>& GetClosingEvent() { return mClosingEvent; }

        protected:
            static Event<void(KeyCode)> mKeyDownEvent;
            static Event<void(KeyCode)> mKeyUpEvent;
            static Event<void(MouseButton)> mMouseDownEvent;
            static Event<void(MouseButton)> mMouseUpEvent;
            static Event<void(int)> mMouseWheelEvent;
            static Event<void(Int32, Int32)> mMousePositionEvent;

            static Event<void()> mLoopEvent;
            static Event<void(Uint32, Uint32)> mResizeEvent;
            static Event<void(WindowActivatedState)> mActivatedEvent;
            static Event<void()> mClosingEvent;

        public:
            BasePlatform() = delete;
            ~BasePlatform() = delete;
        };
    } // namespace platform
} // namespace cube

#if defined(CUBE_PLATFORM_MACOS)
#include "MacOS/MacOSPlatform.h"
#elif defined(CUBE_PLATFORM_WINDOWS)
#include "Windows/WindowsPlatform.h"
#endif
