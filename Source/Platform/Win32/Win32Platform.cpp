#ifdef _WIN32

#include "Win32Platform.h"

#include <iostream>
#include <io.h> 
#include <fcntl.h>

#include "../PlatformDebug.h"
#include "Win32DLib.h"
#include "Win32FileSystem.h"

namespace cube
{
    namespace platform
    {
        PString Platform::title;

        bool Platform::isFinished = false;

        uint32_t Platform::width;
        uint32_t Platform::height;
        uint32_t Platform::windowPosX;
        uint32_t Platform::windowPosY;

        Event<void(KeyCode)> Platform::keyDownEvent;
        Event<void(KeyCode)> Platform::keyUpEvent;
        Event<void(MouseButtonType)> Platform::mouseDownEvent;
        Event<void(MouseButtonType)> Platform::mouseUpEvent;
        Event<void(int)> Platform::mouseWheelEvent;
        Event<void(Uint32, Uint32)> Platform::mousePosEvent;

        Event<void()> Platform::loopEvent;
        Event<void(Uint32, Uint32)> Platform::resizeEvent;
        Event<void(WindowActivatedState)> Platform::activatedEvent;
        Event<void()> Platform::closingEvent;

        HINSTANCE Win32Platform::instance;
        HWND Win32Platform::window;
        bool Win32Platform::isCursorShown = true;

        void Win32Platform::InitImpl()
        {
        }

        void Win32Platform::InitWindowImpl(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY)
        {
            // Show console window in debug mode
#ifdef _DEBUG
            if(AllocConsole()) {
                std::wcout.imbue(std::locale(""));

                SetConsoleOutputCP(CP_WINUNICODE);
                _setmode(_fileno(stdout), _O_WTEXT);

                FILE* acStreamIn;
                FILE* acStreamOut;
                FILE* acStreamErr;

                freopen_s(&acStreamIn, "CONIN$", "rb", stdin);
                freopen_s(&acStreamOut, "CONOUT$", "wb", stdout);
                freopen_s(&acStreamErr, "CONOUT$", "wb", stderr);
            }
#endif // _DEBUG

            String_ConvertAndAppend(Platform::title, title);
            Platform::width = width;
            Platform::height = height;
            Platform::windowPosX = posX;
            Platform::windowPosY = posY;

            instance = GetModuleHandle(NULL);

            // Register winClass
            WNDCLASSEX winClass;
            winClass.cbSize = sizeof(WNDCLASSEX);
            winClass.style = CS_HREDRAW | CS_VREDRAW;
            winClass.lpfnWndProc = WndProc;
            winClass.cbClsExtra = 0;
            winClass.cbWndExtra = 0;
            winClass.hInstance = instance;
            winClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
            winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
            winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
            winClass.lpszMenuName = nullptr;
            winClass.lpszClassName = Platform::title.c_str();
            winClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

            auto res = RegisterClassEx(&winClass);
            PLATFORM_CHECK(res, "Failed to registration while initializing window. (ErrorCode: {0})", GetLastError());
        }

        void Win32Platform::ShowWindowImpl()
        {
            // Create Window
            window = CreateWindowEx(0,
                title.c_str(), title.c_str(),
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
                windowPosX, windowPosY, width, height,
                nullptr, nullptr, instance, nullptr);

            PLATFORM_CHECK(window, "Failed to create a window. (ErrorCode: {0})", GetLastError());
        }

        void* Win32Platform::AllocateImpl(Uint64 size)
        {
            return malloc(size);
        }

        void Win32Platform::FreeImpl(void* ptr)
        {
            free(ptr);
        }

        void* Win32Platform::AllocateAlignedImpl(Uint64 size, Uint64 alignment)
        {
            return _aligned_malloc(size, alignment);
        }

        void Win32Platform::FreeAlignedImpl(void* ptr)
        {
            _aligned_free(ptr);
        }

        void Win32Platform::StartLoopImpl()
        {
            MSG msg;

            while(1) {
                if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else {
                    if(isFinished == true)
                        break;

                    loopEvent.Dispatch();
                }
            }
        }

        void Win32Platform::FinishLoopImpl()
        {
            isFinished = true;
        }

        void Win32Platform::SleepImpl(Uint32 time)
        {
            ::Sleep(time);
        }

        void Win32Platform::ShowCursorImpl()
        {
            if(isCursorShown == false) {
                ::ShowCursor(TRUE);
                isCursorShown = true;
            }
        }

        void Win32Platform::HideCursorImpl()
        {
            if(isCursorShown == true) {
                ::ShowCursor(FALSE);
                isCursorShown = false;
            }
        }

        void Win32Platform::MoveCursorImpl(int x, int y)
        {
            POINT p;
            p.x = x;
            p.y = y;

            ClientToScreen(window, &p);
            SetCursorPos(p.x, p.y);
        }

        void Win32Platform::GetCursorPosImpl(int& x, int& y)
        {
            POINT p;
            ::GetCursorPos(&p);

            ScreenToClient(window, &p);

            x = p.x;
            y = p.y;
        }

        SPtr<DLib> Win32Platform::LoadDLibImpl(StringView path)
        {
            return std::make_shared<Win32DLib>(path);
        }

        HINSTANCE Win32Platform::GetInstance()
        {
            return instance;
        }

        HWND Win32Platform::GetWindow()
        {
            return window;
        }

        bool isActivatedByMouse = false;

        LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            switch(uMsg) {
                // Clicking the menu buttons on the header doesn't dispatch WA_CLICKACTIVE in WM_ACTIVATE.
                // So using WM_MOUSEACTIVATE, check whether the window is activated by clicking.
            case WM_MOUSEACTIVATE:
                isActivatedByMouse = true;
                break;

            case WM_ACTIVATE:
            {
                WindowActivatedState state;
                WORD s = LOWORD(wParam);

                if(s == WA_ACTIVE)
                    state = (isActivatedByMouse == true) ? WindowActivatedState::ClickActive : WindowActivatedState::Active;
                else if(s == WA_CLICKACTIVE)
                    state = WindowActivatedState::ClickActive;
                else if(s == WA_INACTIVE)
                    state = WindowActivatedState::Inactive;
                else {
                    PLATFORM_PRINT_LOG("Invalid activated state ({0})", s);
                }

                Platform::activatedEvent.Dispatch(state);

                isActivatedByMouse = false;

                break;
            }

            case WM_CLOSE:
                Platform::closingEvent.Dispatch();
                break;

            case WM_SIZE:
                if(wParam != SIZE_MINIMIZED) {
                    Platform::width = lParam & 0xffff;
                    Platform::height = (lParam & 0xffff0000) >> 16;

                    Platform::resizeEvent.Dispatch(Platform::width, Platform::height);
                }
                break;

            case WM_KEYDOWN:
                Platform::keyDownEvent.Dispatch(static_cast<KeyCode>(wParam));
                break;
            case WM_KEYUP:
                Platform::keyUpEvent.Dispatch(static_cast<KeyCode>(wParam));
                break;

            case WM_LBUTTONDOWN:
                Platform::mouseDownEvent.Dispatch(MouseButtonType::Left);
                break;
            case WM_LBUTTONUP:
                Platform::mouseUpEvent.Dispatch(MouseButtonType::Left);
                break;
            case WM_RBUTTONDOWN:
                Platform::mouseDownEvent.Dispatch(MouseButtonType::Right);
                break;
            case WM_RBUTTONUP:
                Platform::mouseUpEvent.Dispatch(MouseButtonType::Right);
                break;
            case WM_MBUTTONDOWN:
                Platform::mouseDownEvent.Dispatch(MouseButtonType::Middle);
                break;
            case WM_MBUTTONUP:
                Platform::mouseUpEvent.Dispatch(MouseButtonType::Middle);
                break;

            case WM_MOUSEWHEEL:
                Platform::mouseWheelEvent.Dispatch(GET_WHEEL_DELTA_WPARAM(wParam));
                break;

            case WM_MOUSEMOVE:
            {
                int posX = LOWORD(lParam);
                int posY = HIWORD(lParam);

                Platform::mousePosEvent.Dispatch(posX, posY);
                break;
            }
            }

            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    } // namespace platform
} // namespace cube

#endif // _WIN32
