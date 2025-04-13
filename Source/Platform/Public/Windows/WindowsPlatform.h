#pragma once

#ifdef CUBE_PLATFORM_WINDOWS

#include "PlatformHeader.h"
#include "Platform.h"

#include <Windows.h>

#include "WindowsString.h"

namespace cube
{
    namespace platform
    {
        class WindowsPlatform : public Platform
        {
        public:
            static void InitializeImpl();
            static void ShutdownImpl();

            static void InitWindowImpl(StringView title, Uint32 width, Int32 height, Int32 posX, Int32 posY);
            static void ShowWindowImpl();
            static void ChangeWindowTitleImpl(StringView title);

            static void* AllocateImpl(Uint64 size);
            static void FreeImpl(void* ptr);
            static void* AllocateAlignedImpl(Uint64 size, Uint64 alignment);
            static void FreeAlignedImpl(void* ptr);

            static void StartLoopImpl();
            static void FinishLoopImpl();
            static void SleepImpl(float timeSec);

            static void ShowCursorImpl();
            static void HideCursorImpl();
            static void MoveCursorImpl(int x, int y);
            static void GetCursorPosImpl(int& x, int& y);

            static Uint32 GetWindowWidthImpl();
            static Uint32 GetWindowHeightImpl();
            static Int32 GetWindowPositionXImpl();
            static Int32 GetWindowPositionYImpl();

            static SharedPtr<DLib> LoadDLibImpl(StringView path);

        public:
            static CUBE_PLATFORM_EXPORT HINSTANCE GetInstance();
            static CUBE_PLATFORM_EXPORT HWND GetWindow();
            static CUBE_PLATFORM_EXPORT void SetImGUIWndProcFunction(const std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>& function);

        private:
            friend LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

            static bool mIsFinished;

            static HINSTANCE mInstance;

            static HWND mWindow;
            static WindowsString mWindowTitle;
            static Uint32 mWindowWidth;
            static Uint32 mWindowHeight;
            static Int32 mWindowPosX;
            static Int32 mWindowPosY;

            static bool mIsCursorShown;

            WindowsPlatform() = delete;
            ~WindowsPlatform() = delete;
        };

        LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
