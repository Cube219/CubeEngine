#pragma once

#ifdef _WIN32

#include "../PlatformHeader.h"

#include <Windows.h>

#include "../Platform.h"

namespace cube
{
    namespace platform
    {
        class PLATFORM_EXPORT Win32Platform : public Platform
        {
        public:
            static void InitImpl();

            static void InitWindowImpl(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY);
            static void ShowWindowImpl();

            static void* AllocateImpl(Uint64 size);
            static void FreeImpl(void* ptr);
            static void* AllocateAlignedImpl(Uint64 size, Uint64 alignment);
            static void FreeAlignedImpl(void* ptr);

            static void StartLoopImpl();
            static void FinishLoopImpl();
            static void SleepImpl(Uint32 time);

            static void ShowCursorImpl();
            static void HideCursorImpl();

            static void MoveCursorImpl(int x, int y);
            static void GetCursorPosImpl(int& x, int& y);

            static SPtr<DLib> LoadDLibImpl(StringView path);

            static HINSTANCE GetInstance();
            static HWND GetWindow();

        private:
            friend LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

            static HINSTANCE instance;
            static HWND window;
            static bool isCursorShown;

            Win32Platform() = delete;
            ~Win32Platform() = delete;
        };
        PLATFORM_DEFINITIONS(Win32Platform)

        LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    } // namespace platform
} // namespace cube

#endif // _WIN32
