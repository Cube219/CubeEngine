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

        bool WindowsPlatform::mIsFinished = false;

        HINSTANCE WindowsPlatform::mInstance;

        HWND WindowsPlatform::mWindow;
        WindowsString WindowsPlatform::mWindowTitle;
        Uint32 WindowsPlatform::mWindowWidth;
        Uint32 WindowsPlatform::mWindowHeight;
        Uint32 WindowsPlatform::mWindowPosX;
        Uint32 WindowsPlatform::mWindowPosY;

        bool WindowsPlatform::mIsCursorShown = true;

        std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> gImGUIWndProcFunction;

        void WindowsPlatform::InitializeImpl()
        {
            // Show logger window in debug mode
#ifdef _DEBUG
            WindowsDebug::CreateAndShowLoggerWindow();
#endif // _DEBUG
        }

        void WindowsPlatform::ShutdownImpl()
        {
#ifdef _DEBUG
            // Wait console input not to close console immediately
            std::wcout << L"Press any key to close the window..." << std::endl;

            FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
            _getch();
#endif
        }

        void WindowsPlatform::InitWindowImpl(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY)
        {
            String_ConvertAndAppend(mWindowTitle, title);
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

        void WindowsPlatform::ShowWindowImpl()
        {
            // Create Window
            mWindow = CreateWindowEx(0,
                                     mWindowTitle.c_str(), mWindowTitle.c_str(),
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
                                     mWindowPosX, mWindowPosY, mWindowWidth, mWindowHeight,
                                     nullptr, nullptr, mInstance, nullptr);

            CHECK_FORMAT(mWindow, "Failed to create a window. (ErrorCode: {0})", GetLastError());
        }

        void WindowsPlatform::ChangeWindowTitleImpl(StringView title)
        {
            mWindowTitle.clear();
            String_ConvertAndAppend(mWindowTitle, title);

            if (SetWindowText(mWindow, mWindowTitle.c_str()) == 0)
            {
                CUBE_LOG(LogType::Warning, WindowsPlatform, "Failed to change the window title '{}'. (ErrorCode: {})", title, GetLastError());
            }
        }

        void* WindowsPlatform::AllocateImpl(Uint64 size)
        {
            return malloc(size);
        }

        void WindowsPlatform::FreeImpl(void* ptr)
        {
            free(ptr);
        }

        void* WindowsPlatform::AllocateAlignedImpl(Uint64 size, Uint64 alignment)
        {
            return _aligned_malloc(size, alignment);
        }

        void WindowsPlatform::FreeAlignedImpl(void* ptr)
        {
            _aligned_free(ptr);
        }

        void WindowsPlatform::StartLoopImpl()
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

        void WindowsPlatform::FinishLoopImpl()
        {
            mIsFinished = true;
        }

        void WindowsPlatform::SleepImpl(Uint32 time)
        {
            ::Sleep(time);
        }

        void WindowsPlatform::ShowCursorImpl()
        {
            if (mIsCursorShown == false)
            {
                ::ShowCursor(TRUE);
                mIsCursorShown = true;
            }
        }

        void WindowsPlatform::HideCursorImpl()
        {
            if (mIsCursorShown == true)
            {
                ::ShowCursor(FALSE);
                mIsCursorShown = false;
            }
        }

        void WindowsPlatform::MoveCursorImpl(int x, int y)
        {
            POINT p;
            p.x = x;
            p.y = y;

            ClientToScreen(mWindow, &p);
            SetCursorPos(p.x, p.y);
        }

        void WindowsPlatform::GetCursorPosImpl(int& x, int& y)
        {
            POINT p;
            ::GetCursorPos(&p);

            ScreenToClient(mWindow, &p);

            x = p.x;
            y = p.y;
        }

        Uint32 WindowsPlatform::GetWindowWidthImpl()
        {
            return mWindowWidth;
        }

        Uint32 WindowsPlatform::GetWindowHeightImpl()
        {
            return mWindowHeight;
        }

        Uint32 WindowsPlatform::GetWindowPositionXImpl()
        {
            return mWindowPosX;
        }

        Uint32 WindowsPlatform::GetWindowPositionYImpl()
        {
            return mWindowPosY;
        }

        SharedPtr<DLib> WindowsPlatform::LoadDLibImpl(StringView path)
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
                    CUBE_LOG(LogType::Error, WindowsPlatform, "Invalid activated state ({0})", s);
                }

                Platform::GetActivatedEvent().Dispatch(state);

                isActivatedByMouse = false;

                break;
            }

            case WM_CLOSE:
                Platform::GetClosingEvent().Dispatch();
                break;

            case WM_SIZE:
                if (wParam != SIZE_MINIMIZED)
                {
                    WindowsPlatform::mWindowWidth = lParam & 0xffff;
                    WindowsPlatform::mWindowHeight = (lParam & 0xffff0000) >> 16;

                    Platform::GetResizeEvent().Dispatch(WindowsPlatform::mWindowWidth, WindowsPlatform::mWindowHeight);
                }
                break;

            case WM_KEYDOWN:
                Platform::GetKeyDownEvent().Dispatch(static_cast<KeyCode>(wParam));
                break;
            case WM_KEYUP:
                Platform::GetKeyUpEvent().Dispatch(static_cast<KeyCode>(wParam));
                break;

            case WM_LBUTTONDOWN:
                Platform::GetMouseDownEvent().Dispatch(MouseButton::Left);
                break;
            case WM_LBUTTONUP:
                Platform::GetMouseUpEvent().Dispatch(MouseButton::Left);
                break;
            case WM_RBUTTONDOWN:
                Platform::GetMouseDownEvent().Dispatch(MouseButton::Right);
                break;
            case WM_RBUTTONUP:
                Platform::GetMouseUpEvent().Dispatch(MouseButton::Right);
                break;
            case WM_MBUTTONDOWN:
                Platform::GetMouseDownEvent().Dispatch(MouseButton::Middle);
                break;
            case WM_MBUTTONUP:
                Platform::GetMouseUpEvent().Dispatch(MouseButton::Middle);
                break;

            case WM_MOUSEWHEEL:
                Platform::GetMouseWheelEvent().Dispatch(GET_WHEEL_DELTA_WPARAM(wParam));
                break;

            case WM_MOUSEMOVE:
            {
                POINT p;
                ::GetCursorPos(&p);
                ScreenToClient(hWnd, &p);

                Platform::GetMousePositionEvent().Dispatch(p.x, p.y);
                break;
            }
            }

            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
