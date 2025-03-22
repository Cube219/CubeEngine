#pragma once

#include "PlatformHeader.h"

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

        class CUBE_PLATFORM_EXPORT Platform
        {
        public:
            static void Initialize();
            static void Shutdown();

            static void InitWindow(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY);
            static void ShowWindow();

            static void* Allocate(Uint64 size);
            static void Free(void* ptr);
            static void* AllocateAligned(Uint64 size, Uint64 alignment);
            static void FreeAligned(void* ptr);

            static void StartLoop();
            static void FinishLoop();
            static void Sleep(Uint32 time);

            static void ShowCursor();
            static void HideCursor();
            static void MoveCursor(int x, int y);
            static void GetCursorPos(int& x, int& y);

            static Uint32 GetWindowWidth();
            static Uint32 GetWindowHeight();
            static Uint32 GetWindowPositionX();
            static Uint32 GetWindowPositionY();

            static SharedPtr<DLib> LoadDLib(StringView path);

            static Event<void(KeyCode)>& GetKeyDownEvent() { return mKeyDownEvent; }
            static Event<void(KeyCode)>& GetKeyUpEvent() { return mKeyUpEvent; }
            static Event<void(MouseButton)>& GetMouseDownEvent() { return mMouseDownEvent; }
            static Event<void(MouseButton)>& GetMouseUpEvent() { return mMouseUpEvent; }
            static Event<void(int)>& GetMouseWheelEvent() { return mMouseWheelEvent; }
            static Event<void(Uint32, Uint32)>& GetMousePositionEvent() { return mMousePositionEvent; }
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
            static Event<void(Uint32, Uint32)> mMousePositionEvent;

            static Event<void()> mLoopEvent;
            static Event<void(Uint32, Uint32)> mResizeEvent;
            static Event<void(WindowActivatedState)> mActivatedEvent;
            static Event<void()> mClosingEvent;

        public:
            Platform() = delete;
            ~Platform() = delete;
        };

#define PLATFORM_CLASS_DEFINITIONS(ChildClass) \
        inline void Platform::Initialize() { ChildClass::InitializeImpl(); } \
        inline void Platform::Shutdown() { ChildClass::ShutdownImpl(); } \
        \
        inline void Platform::InitWindow(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY) { \
            ChildClass::InitWindowImpl(title, width, height, posX, posY); \
        } \
        inline void Platform::ShowWindow() { ChildClass::ShowWindowImpl(); } \
        \
        inline void* Platform::Allocate(Uint64 size) { return ChildClass::AllocateImpl(size); } \
        inline void Platform::Free(void* ptr) { ChildClass::FreeImpl(ptr); } \
        inline void* Platform::AllocateAligned(Uint64 size, Uint64 alignment) { return ChildClass::AllocateAlignedImpl(size, alignment); } \
        inline void Platform::FreeAligned(void* ptr) { ChildClass::FreeAlignedImpl(ptr); } \
        \
        inline void Platform::StartLoop() { ChildClass::StartLoopImpl(); } \
        inline void Platform::FinishLoop() { ChildClass::FinishLoopImpl(); } \
        inline void Platform::Sleep(Uint32 time) { ChildClass::SleepImpl(time); } \
        \
        inline void Platform::ShowCursor() { ChildClass::ShowCursorImpl(); } \
        inline void Platform::HideCursor() { ChildClass::HideCursorImpl(); } \
        inline void Platform::MoveCursor(int x, int y) { ChildClass::MoveCursorImpl(x, y); } \
        inline void Platform::GetCursorPos(int& x, int& y) { ChildClass::GetCursorPosImpl(x, y); } \
        \
        inline Uint32 Platform::GetWindowWidth() { return ChildClass::GetWindowWidthImpl(); } \
        inline Uint32 Platform::GetWindowHeight() { return ChildClass::GetWindowHeightImpl(); } \
        inline Uint32 Platform::GetWindowPositionX() { return ChildClass::GetWindowPositionXImpl(); } \
        inline Uint32 Platform::GetWindowPositionY() { return ChildClass::GetWindowPositionYImpl(); } \
        \
        inline SharedPtr<DLib> Platform::LoadDLib(StringView path) { return ChildClass::LoadDLibImpl(path); }
    } // namespace platform
} // namespace cube
