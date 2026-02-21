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
        class CUBE_PLATFORM_EXPORT WindowsPlatform : public BasePlatform
        {
            // === Base member functions ===
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
            static void SetPostLoopMainThreadFunction(std::function<void()> function);
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

            static SharedPtr<WindowsDLib> LoadDLib(StringView path);
            // === Base member functions ===

        public:
            static HINSTANCE GetInstance();
            static HWND GetWindow();
            static void SetImGUIWndProcFunction(const std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>& function);

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

        public:
            WindowsPlatform() = delete;
            ~WindowsPlatform() = delete;
        };

        LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        using Platform = WindowsPlatform;
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
