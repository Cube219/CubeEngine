#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsPlatform.h"

#include <conio.h>
#include <iostream>

#include "Checker.h"
#include "Windows/WindowsDebug.h"
#include "Windows/WindowsDLib.h"

namespace cube
{
    namespace platform
    {
        Event<void(KeyCode)> BasePlatform::mKeyDownEvent;
        Event<void(KeyCode)> BasePlatform::mKeyUpEvent;
        Event<void(MouseButton)> BasePlatform::mMouseDownEvent;
        Event<void(MouseButton)> BasePlatform::mMouseUpEvent;
        Event<void(int)> BasePlatform::mMouseWheelEvent;
        Event<void(Int32, Int32)> BasePlatform::mMousePositionEvent;

        Event<void()> BasePlatform::mLoopEvent;
        Event<void(Uint32, Uint32)> BasePlatform::mResizeEvent;
        Event<void(WindowActivatedState)> BasePlatform::mActivatedEvent;
        Event<void()> BasePlatform::mClosingEvent;

        bool WindowsPlatform::mIsFinished = false;

        HINSTANCE WindowsPlatform::mInstance;

        HWND WindowsPlatform::mWindow;
        WindowsString WindowsPlatform::mWindowTitle;
        Uint32 WindowsPlatform::mWindowWidth;
        Uint32 WindowsPlatform::mWindowHeight;
        Int32 WindowsPlatform::mWindowPosX;
        Int32 WindowsPlatform::mWindowPosY;

        bool WindowsPlatform::mIsCursorShown = true;

        std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> gImGUIWndProcFunction;

        void WindowsPlatform::Initialize()
        {
            // Show logger window in debug mode
#ifdef CUBE_DEBUG
            WindowsDebug::CreateAndShowLoggerWindow();
#endif // CUBE_DEBUG
        }

        void WindowsPlatform::Shutdown()
        {
#ifdef CUBE_DEBUG
            // Wait console input not to close console immediately
            std::wcout << L"Press any key to close the window..." << std::endl;

            FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
            _getch();
#endif // CUBE_DEBUG
        }

        void WindowsPlatform::InitWindow(StringView title, Uint32 width, Uint32 height, Int32 posX, Int32 posY)
        {
            mWindowTitle = String_Convert<WindowsString>(title);
            mWindowWidth = width;
            mWindowHeight = height;
            mWindowPosX = posX;
            mWindowPosY = posY;

            mInstance = GetModuleHandle(NULL);

            // Register winClass
            WNDCLASSEX winClass;
            winClass.cbSize = sizeof(WNDCLASSEX);
            winClass.style = CS_HREDRAW | CS_VREDRAW;
            winClass.lpfnWndProc = WndProc;
            winClass.cbClsExtra = 0;
            winClass.cbWndExtra = 0;
            winClass.hInstance = mInstance;
            winClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
            winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
            winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
            winClass.lpszMenuName = nullptr;
            winClass.lpszClassName = mWindowTitle.c_str();
            winClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

            auto res = RegisterClassEx(&winClass);
            CHECK_FORMAT(res, "Failed to registration while initializing window. (ErrorCode: {0})", GetLastError());
        }

        void WindowsPlatform::ShowWindow()
        {
            // Create Window
            RECT rect = { 0, 0, (LONG)mWindowWidth, (LONG)mWindowHeight };
            AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

            mWindow = CreateWindowEx(0,
                mWindowTitle.c_str(), mWindowTitle.c_str(),
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
                mWindowPosX, mWindowPosY,
                rect.right - rect.left, rect.bottom - rect.top,
                nullptr, nullptr, mInstance, nullptr
            );

            CHECK_FORMAT(mWindow, "Failed to create a window. (ErrorCode: {0})", GetLastError());
        }

        void WindowsPlatform::ChangeWindowTitle(StringView title)
        {
            mWindowTitle = String_Convert<WindowsString>(title);

            if (SetWindowText(mWindow, mWindowTitle.c_str()) == 0)
            {
                CUBE_LOG(Warning, WindowsPlatform, "Failed to change the window title '{}'. (ErrorCode: {})", title, GetLastError());
            }
        }

        void* WindowsPlatform::Allocate(Uint64 size)
        {
            return malloc(size);
        }

        void WindowsPlatform::Free(void* ptr)
        {
            free(ptr);
        }

        void* WindowsPlatform::AllocateAligned(Uint64 size, Uint64 alignment)
        {
            return _aligned_malloc(size, alignment);
        }

        void WindowsPlatform::FreeAligned(void* ptr)
        {
            _aligned_free(ptr);
        }

        void WindowsPlatform::SetEngineInitializeFunction(std::function<void()> function)
        {
            // Do nothing. Windows run loop in main thread.
        }

        void WindowsPlatform::SetEngineShutdownFunction(std::function<void()> function)
        {
            // Do nothing. Windows run loop in main thread.
        }

        void WindowsPlatform::SetPostLoopMainThreadFunction(std::function<void()> function)
        {
            // Do nothing. Windows run loop in main thread.
        }

        void WindowsPlatform::StartLoop()
        {
            MSG msg;

            while (1)
            {
                if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else
                {
                    if (mIsFinished == true) break;

                    mLoopEvent.Dispatch();
                }
            }
        }

        void WindowsPlatform::FinishLoop()
        {
            mIsFinished = true;
        }

        void WindowsPlatform::Sleep(float timeSec)
        {
            ::Sleep(timeSec * 1000.0f);
        }

        void WindowsPlatform::ShowCursor()
        {
            if (mIsCursorShown == false)
            {
                ::ShowCursor(TRUE);
                mIsCursorShown = true;
            }
        }

        void WindowsPlatform::HideCursor()
        {
            if (mIsCursorShown == true)
            {
                ::ShowCursor(FALSE);
                mIsCursorShown = false;
            }
        }

        void WindowsPlatform::MoveCursor(int x, int y)
        {
            POINT p;
            p.x = x;
            p.y = y;

            ClientToScreen(mWindow, &p);
            SetCursorPos(p.x, p.y);
        }

        void WindowsPlatform::GetCursorPos(int& x, int& y)
        {
            POINT p;
            ::GetCursorPos(&p);

            ScreenToClient(mWindow, &p);

            x = p.x;
            y = p.y;
        }

        Uint32 WindowsPlatform::GetWindowWidth()
        {
            return mWindowWidth;
        }

        Uint32 WindowsPlatform::GetWindowHeight()
        {
            return mWindowHeight;
        }

        Int32 WindowsPlatform::GetWindowPositionX()
        {
            return mWindowPosX;
        }

        Int32 WindowsPlatform::GetWindowPositionY()
        {
            return mWindowPosY;
        }

        SharedPtr<WindowsDLib> WindowsPlatform::LoadDLib(const FilePath& path)
        {
            auto res = std::make_shared<WindowsDLib>(path);
            if (res->GetModule() == nullptr)
            {
                return nullptr;
            }

            return res;
        }

        HINSTANCE WindowsPlatform::GetInstance()
        {
            return mInstance;
        }

        HWND WindowsPlatform::GetWindow()
        {
            return mWindow;
        }

        void WindowsPlatform::SetImGUIWndProcFunction(const std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>& function)
        {
            gImGUIWndProcFunction = function;
        }

        LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            if (gImGUIWndProcFunction)
            {
                if (gImGUIWndProcFunction(hWnd, uMsg, wParam, lParam))
                {
                    return true;
                }
            }

            static bool isActivatedByMouse = false;

            switch (uMsg)
            {
            // Clicking the menu buttons on the header doesn't dispatch WA_CLICKACTIVE in WM_ACTIVATE.
            // So using WM_MOUSEACTIVATE, check whether the window is activated by clicking.
            case WM_MOUSEACTIVATE:
                isActivatedByMouse = true;
                break;

            case WM_ACTIVATE:
            {
                WindowActivatedState state;
                WORD s = LOWORD(wParam);

                if (s == WA_ACTIVE)
                {
                    state = (isActivatedByMouse == true) ? WindowActivatedState::ClickActive : WindowActivatedState::Active;
                }
                else if (s == WA_CLICKACTIVE)
                {
                    state = WindowActivatedState::ClickActive;
                }
                else if (s == WA_INACTIVE)
                {
                    state = WindowActivatedState::Inactive;
                }
                else
                {
                    CUBE_LOG(Error, WindowsPlatform, "Invalid activated state ({0})", s);
                }

                BasePlatform::GetActivatedEvent().Dispatch(state);

                isActivatedByMouse = false;

                break;
            }

            case WM_CLOSE:
                BasePlatform::GetClosingEvent().Dispatch();
                break;

            case WM_SIZE:
                if (wParam != SIZE_MINIMIZED)
                {
                    WindowsPlatform::mWindowWidth = lParam & 0xffff;
                    WindowsPlatform::mWindowHeight = (lParam & 0xffff0000) >> 16;

                    BasePlatform::GetResizeEvent().Dispatch(WindowsPlatform::mWindowWidth, WindowsPlatform::mWindowHeight);
                }
                break;

            case WM_MOVE:
            {
                WindowsPlatform::mWindowPosX = (short)LOWORD(lParam);
                WindowsPlatform::mWindowPosY = (short)HIWORD(lParam);
                break;
            }

            case WM_KEYDOWN:
                BasePlatform::GetKeyDownEvent().Dispatch(static_cast<KeyCode>(wParam));
                break;
            case WM_KEYUP:
                BasePlatform::GetKeyUpEvent().Dispatch(static_cast<KeyCode>(wParam));
                break;

            case WM_LBUTTONDOWN:
                BasePlatform::GetMouseDownEvent().Dispatch(MouseButton::Left);
                break;
            case WM_LBUTTONUP:
                BasePlatform::GetMouseUpEvent().Dispatch(MouseButton::Left);
                break;
            case WM_RBUTTONDOWN:
                BasePlatform::GetMouseDownEvent().Dispatch(MouseButton::Right);
                break;
            case WM_RBUTTONUP:
                BasePlatform::GetMouseUpEvent().Dispatch(MouseButton::Right);
                break;
            case WM_MBUTTONDOWN:
                BasePlatform::GetMouseDownEvent().Dispatch(MouseButton::Middle);
                break;
            case WM_MBUTTONUP:
                BasePlatform::GetMouseUpEvent().Dispatch(MouseButton::Middle);
                break;

            case WM_MOUSEWHEEL:
                BasePlatform::GetMouseWheelEvent().Dispatch(GET_WHEEL_DELTA_WPARAM(wParam));
                break;

            case WM_MOUSEMOVE:
            {
                POINT p;
                ::GetCursorPos(&p);
                ScreenToClient(hWnd, &p);

                BasePlatform::GetMousePositionEvent().Dispatch(p.x, p.y);
                break;
            }
            }

            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
