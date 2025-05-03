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

            static void InitWindow(StringView title, Uint32 width, Uint32 height, Int32 posX, Int32 posY);
            static void ShowWindow();
            static void ChangeWindowTitle(StringView title);

            static void* Allocate(Uint64 size);
            static void Free(void* ptr);
            static void* AllocateAligned(Uint64 size, Uint64 alignment);
            static void FreeAligned(void* ptr);

            static void SetEngineInitializeFunction(std::function<void()> function);
            static void SetEngineShutdownFunction(std::function<void()> function);
            static void StartLoop();
            static void FinishLoop();
            static void Sleep(float timeSec);

            static void ShowCursor();
            static void HideCursor();
            static void MoveCursor(int x, int y);
            static void GetCursorPos(int& x, int& y);

            static Uint32 GetWindowWidth();
            static Uint32 GetWindowHeight();
            static Int32 GetWindowPositionX();
            static Int32 GetWindowPositionY();

            static SharedPtr<DLib> LoadDLib(StringView path);

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
            Platform() = delete;
            ~Platform() = delete;
        };

#define PLATFORM_CLASS_DEFINITIONS(ChildClass) \
        void Platform::Initialize() { ChildClass::InitializeImpl(); } \
        void Platform::Shutdown() { ChildClass::ShutdownImpl(); } \
        \
        void Platform::InitWindow(StringView title, Uint32 width, Uint32 height, Int32 posX, Int32 posY) { \
            ChildClass::InitWindowImpl(title, width, height, posX, posY); \
        } \
        void Platform::ShowWindow() { ChildClass::ShowWindowImpl(); } \
        void Platform::ChangeWindowTitle(StringView title) { ChildClass::ChangeWindowTitleImpl(title); } \
        \
        void* Platform::Allocate(Uint64 size) { return ChildClass::AllocateImpl(size); } \
        void Platform::Free(void* ptr) { ChildClass::FreeImpl(ptr); } \
        void* Platform::AllocateAligned(Uint64 size, Uint64 alignment) { return ChildClass::AllocateAlignedImpl(size, alignment); } \
        void Platform::FreeAligned(void* ptr) { ChildClass::FreeAlignedImpl(ptr); } \
        \
        void Platform::SetEngineInitializeFunction(std::function<void()> function) { ChildClass::SetEngineInitializeFunctionImpl(function); } \
        void Platform::SetEngineShutdownFunction(std::function<void()> function) { ChildClass::SetEngineShutdownFunctionImpl(function); } \
        void Platform::StartLoop() { ChildClass::StartLoopImpl(); } \
        void Platform::FinishLoop() { ChildClass::FinishLoopImpl(); } \
        void Platform::Sleep(float timeSec) { ChildClass::SleepImpl(timeSec); } \
        \
        void Platform::ShowCursor() { ChildClass::ShowCursorImpl(); } \
        void Platform::HideCursor() { ChildClass::HideCursorImpl(); } \
        void Platform::MoveCursor(int x, int y) { ChildClass::MoveCursorImpl(x, y); } \
        void Platform::GetCursorPos(int& x, int& y) { ChildClass::GetCursorPosImpl(x, y); } \
        \
        Uint32 Platform::GetWindowWidth() { return ChildClass::GetWindowWidthImpl(); } \
        Uint32 Platform::GetWindowHeight() { return ChildClass::GetWindowHeightImpl(); } \
        Int32 Platform::GetWindowPositionX() { return ChildClass::GetWindowPositionXImpl(); } \
        Int32 Platform::GetWindowPositionY() { return ChildClass::GetWindowPositionYImpl(); } \
        \
        SharedPtr<DLib> Platform::LoadDLib(StringView path) { return ChildClass::LoadDLibImpl(path); }
    } // namespace platform
} // namespace cube
