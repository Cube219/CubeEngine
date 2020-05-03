#pragma once

#include "PlatformHeader.h"

#include "Utility/Event.h"
#include "Utility/KeyCode.h"
#include "PlatformString.h"
#include "DLib.h"

namespace cube
{
    namespace platform
    {
        enum class WindowActivatedState
        {
            Active, ClickActive, Inactive
        };

        class PLATFORM_EXPORT Platform
        {
        public:
            static void Init();

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

            static Uint32 GetWindowWidth() { return width; }
            static Uint32 GetWindowHeight() { return height; }
            static Uint32 GetWindowPositionX() { return windowPosX; }
            static Uint32 GetWindowPositionY() { return windowPosY; }

            static SPtr<DLib> LoadDLib(StringView path);

            static auto& GetKeyDownEvent() { return keyDownEvent; }
            static auto& GetKeyUpEvent() { return keyUpEvent; }
            static auto& GetMouseDownEvent() { return mouseDownEvent; }
            static auto& GetMouseUpEvent() { return mouseUpEvent; }
            static auto& GetMouseWheelEvent() { return mouseWheelEvent; }
            static auto& GetMousePosEvent() { return mousePosEvent; }
            static auto& GetLoopEvent() { return loopEvent; }
            static auto& GetResizeEvent() { return resizeEvent; }
            static auto& GetActivatedEvent() { return activatedEvent; }
            static auto& GetClosingEvent() { return closingEvent; }

        protected:
            static PString title;

            static bool isFinished;

            static Uint32 width;
            static Uint32 height;
            static Uint32 windowPosX;
            static Uint32 windowPosY;

            static Event<void(KeyCode)> keyDownEvent;
            static Event<void(KeyCode)> keyUpEvent;
            static Event<void(MouseButtonType)> mouseDownEvent;
            static Event<void(MouseButtonType)> mouseUpEvent;
            static Event<void(int)> mouseWheelEvent;
            static Event<void(Uint32, Uint32)> mousePosEvent;

            static Event<void()> loopEvent;
            static Event<void(Uint32, Uint32)> resizeEvent;
            static Event<void(WindowActivatedState)> activatedEvent;
            static Event<void()> closingEvent;

        public:
            Platform() = delete;
            ~Platform() = delete;
        };

#define PLATFORM_DEFINITIONS(Child)                                                     \
        inline void Platform::Init() {                                                  \
            Child::InitImpl();                                                          \
        }                                                                               \
                                                                                        \
        inline void Platform::InitWindow                                                \
            (StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY) { \
            Child::InitWindowImpl(title, width, height, posX, posY);                    \
        }                                                                               \
        inline void Platform::ShowWindow() {                                            \
            Child::ShowWindowImpl();                                                    \
        }                                                                               \
                                                                                        \
        inline void* Platform::Allocate(Uint64 size) {                                  \
            return Child::AllocateImpl(size);                                           \
        }                                                                               \
        inline void Platform::Free(void* ptr) {                                         \
            Child::FreeImpl(ptr);                                                       \
        }                                                                               \
        inline void* Platform::AllocateAligned(Uint64 size, Uint64 alignment) {         \
            return Child::AllocateAlignedImpl(size, alignment);                         \
        }                                                                               \
        inline void Platform::FreeAligned(void* ptr) {                                  \
            Child::FreeAlignedImpl(ptr);                                                \
        }                                                                               \
                                                                                        \
        inline void Platform::StartLoop() {                                             \
            Child::StartLoopImpl();                                                     \
        }                                                                               \
        inline void Platform::FinishLoop() {                                            \
            Child::FinishLoopImpl();                                                    \
        }                                                                               \
        inline void Platform::Sleep(Uint32 time) {                                      \
            Child::SleepImpl(time);                                                     \
        }                                                                               \
                                                                                        \
        inline void Platform::ShowCursor() {                                            \
            Child::ShowCursorImpl();                                                    \
        }                                                                               \
        inline void Platform::HideCursor() {                                            \
            Child::HideCursorImpl();                                                    \
        }                                                                               \
        inline void Platform::MoveCursor(int x, int y) {                                \
            Child::MoveCursorImpl(x, y);                                                \
        }                                                                               \
        inline void Platform::GetCursorPos(int& x, int& y) {                            \
            Child::GetCursorPosImpl(x, y);                                              \
        }                                                                               \
                                                                                        \
        inline SPtr<DLib> Platform::LoadDLib(StringView path) {                         \
            return Child::LoadDLibImpl(path);                                           \
        }
    } // namespace platform
} // namespace cube
