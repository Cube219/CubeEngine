#include "KeyboardMouseInput.h"

#include "Core/Renderer/RenderingThread.h"

namespace cube
{
    KeyboardMouseInput::KeyboardMouseInput() :
        mIsCursorLocked(false), mIsKeyboardActivated(true), mIsMouseActivated(true)
    {
        using namespace platform;

        mKeyDownEventFunc = Platform::GetKeyDownEvent().AddListener([this](KeyCode keyCode) {
            mIsKeyPressed[SCast(int)(keyCode)] = true;
            });
        mKeyUpEventFunc = Platform::GetKeyUpEvent().AddListener([this](KeyCode keyCode) {
            mIsKeyPressed[SCast(int)(keyCode)] = false;
            });

        mMouseDownEventFunc = Platform::GetMouseDownEvent().AddListener([this](MouseButtonType buttonType) {
            mIsMousePressed[SCast(int)(buttonType)] = true;

            if(mIsCursorLocked == true && mIsMouseActivated == false)
                SetMouseActivate();
            });
        mMouseUpEventFunc = Platform::GetMouseUpEvent().AddListener([this](MouseButtonType buttonType) {
            mIsMousePressed[SCast(int)(buttonType)] = false;
            });
        mMouseWheelEventFunc = Platform::GetMouseWheelEvent().AddListener([this](int wheelDelta) {
            });
        mMousePosEventFunc = Platform::GetMousePosEvent().AddListener([this](uint32_t x, uint32_t y) {
            if(mIsMouseActivated == false)
                return;

            mMousePos = Vector2(SCast(float)(x), SCast(float)(y));
            });

        mActivatedEventFunc = Platform::GetActivatedEvent().AddListener([this](platform::WindowActivatedState state) {
            if(state == platform::WindowActivatedState::Active) {
                mIsKeyboardActivated = true;
                SetMouseActivate();
            }
            else if(state == platform::WindowActivatedState::ClickActive) {
                mIsKeyboardActivated = true;

                if(mIsCursorLocked == false)
                    SetMouseActivate();
                // Defer to activate the mouse when the cursor is locked. Activate it after clicking
            }
            else if(state == platform::WindowActivatedState::Inactive) {
                mIsKeyboardActivated = false;
                SetMouseInactivate();
            }
            });

        mMousePos = Vector2::Zero();
        mLastMousePos = Vector2::Zero();
    }

    KeyboardMouseInput::~KeyboardMouseInput()
    {
        using namespace platform;

        Platform::GetKeyDownEvent().RemoveListener(mKeyDownEventFunc);
        Platform::GetKeyUpEvent().RemoveListener(mKeyUpEventFunc);
        Platform::GetMouseDownEvent().RemoveListener(mMouseDownEventFunc);
        Platform::GetMouseUpEvent().RemoveListener(mMouseUpEventFunc);
        Platform::GetMouseWheelEvent().RemoveListener(mMouseWheelEventFunc);
        Platform::GetMousePosEvent().RemoveListener(mMousePosEventFunc);
        Platform::GetActivatedEvent().RemoveListener(mActivatedEventFunc);
    }

    bool KeyboardMouseInput::IsButtonPressed(DigitalButton button)
    {
        if(mIsKeyboardActivated == false)
            return false;

        KeyCode code = GetKeyCode(button);
        return mIsKeyPressed[SCast(int)(code)];
    }

    bool KeyboardMouseInput::IsKeyPressed(KeyCode keyCode)
    {
        if(mIsKeyboardActivated == false)
            return false;

        return mIsKeyPressed[SCast(int)(keyCode)];
    }

    bool KeyboardMouseInput::IsMouseDown(MouseButtonType buttonType)
    {
        if(mIsMouseActivated == false)
            return false;

        return mIsMousePressed[SCast(int)(buttonType)];
    }

    void KeyboardMouseInput::LockCursor()
    {
        RenderingThread::QueueTask([]() {
            platform::Platform::HideCursor();
            });
        mIsCursorLocked = true;
    }

    void KeyboardMouseInput::UnlockCursor()
    {
        RenderingThread::QueueTask([]() {
            platform::Platform::ShowCursor();
            });
        mIsCursorLocked = false;
    }

    Vector2 KeyboardMouseInput::GetMousePosition()
    {
        return mMousePos;
    }

    Vector2 KeyboardMouseInput::GetMouseDeltaPosition()
    {
        return mMouseDeltaPos;
    }

    void KeyboardMouseInput::UpdateMouseDelta(float dt)
    {
        if(mIsMouseActivated == false) {
            return;
        }

        if(mIsCursorLocked == false) {
            mMouseDeltaPos = (mMousePos - mLastMousePos) * dt * 3.0f;
        }
        else {
            int width = platform::Platform::GetWindowWidth();
            int height = platform::Platform::GetWindowHeight();

            mMouseDeltaPos = (mMousePos - Vector2(width / 2.0f, height / 2.0f)) * dt * 3.0f;

            platform::Platform::MoveCursor(width / 2, height / 2);
        }

        mLastMousePos = mMousePos;
    }

    void KeyboardMouseInput::SetMouseActivate()
    {
        if(mIsCursorLocked == true) {
            int width = platform::Platform::GetWindowWidth();
            int height = platform::Platform::GetWindowHeight();

            platform::Platform::MoveCursor(width / 2, height / 2);
            platform::Platform::HideCursor();
        }

        int x, y;
        platform::Platform::GetCursorPos(x, y);
        mLastMousePos = mMousePos = Vector2((float)x, (float)y);
        mIsMouseActivated = true;
    }

    void KeyboardMouseInput::SetMouseInactivate()
    {
        if(mIsCursorLocked == true) {
            platform::Platform::ShowCursor();
        }

        mMouseDeltaPos = Vector2::Zero();
        mIsMouseActivated = false;
    }

    KeyCode KeyboardMouseInput::GetKeyCode(DigitalButton button)
    {
        KeyCode keyCode;

        switch(button) {
        case DigitalButton::A:
            keyCode = KeyCode::A;
            break;
        case DigitalButton::B:
            keyCode = KeyCode::B;
            break;
        case DigitalButton::C:
            keyCode = KeyCode::C;
            break;
        case DigitalButton::D:
            keyCode = KeyCode::D;
            break;
        case DigitalButton::E:
            keyCode = KeyCode::E;
            break;
        case DigitalButton::F:
            keyCode = KeyCode::F;
            break;
        case DigitalButton::G:
            keyCode = KeyCode::G;
            break;
        case DigitalButton::H:
            keyCode = KeyCode::H;
            break;
        case DigitalButton::I:
            keyCode = KeyCode::I;
            break;
        case DigitalButton::J:
            keyCode = KeyCode::J;
            break;
        case DigitalButton::K:
            keyCode = KeyCode::K;
            break;
        case DigitalButton::L:
            keyCode = KeyCode::L;
            break;
        case DigitalButton::M:
            keyCode = KeyCode::M;
            break;
        case DigitalButton::N:
            keyCode = KeyCode::N;
            break;
        case DigitalButton::O:
            keyCode = KeyCode::O;
            break;
        case DigitalButton::P:
            keyCode = KeyCode::P;
            break;
        case DigitalButton::Q:
            keyCode = KeyCode::Q;
            break;
        case DigitalButton::R:
            keyCode = KeyCode::R;
            break;
        case DigitalButton::S:
            keyCode = KeyCode::S;
            break;
        case DigitalButton::T:
            keyCode = KeyCode::T;
            break;
        case DigitalButton::U:
            keyCode = KeyCode::U;
            break;
        case DigitalButton::V:
            keyCode = KeyCode::V;
            break;
        case DigitalButton::W:
            keyCode = KeyCode::W;
            break;
        case DigitalButton::X:
            keyCode = KeyCode::X;
            break;
        case DigitalButton::Y:
            keyCode = KeyCode::Y;
            break;
        case DigitalButton::Z:
            keyCode = KeyCode::Z;
            break;
        case DigitalButton::Tab:
            keyCode = KeyCode::Tab;
            break;
        case DigitalButton::Enter:
            keyCode = KeyCode::Enter;
            break;
        case DigitalButton::Pause:
            keyCode = KeyCode::Pause;
            break;
        case DigitalButton::CapsLock:
            keyCode = KeyCode::CapsLock;
            break;
        case DigitalButton::Escape:
            keyCode = KeyCode::Escape;
            break;
        case DigitalButton::SpaceBar:
            keyCode = KeyCode::Space;
            break;
        case DigitalButton::PageUp:
            keyCode = KeyCode::Prior;
            break;
        case DigitalButton::PageDown:
            keyCode = KeyCode::Next;
            break;
        case DigitalButton::End:
            keyCode = KeyCode::End;
            break;
        case DigitalButton::Home:
            keyCode = KeyCode::Home;
            break;
        case DigitalButton::LeftArrow:
            keyCode = KeyCode::LeftArrow;
            break;
        case DigitalButton::UpArrow:
            keyCode = KeyCode::UpArrow;
            break;
        case DigitalButton::RightArrow:
            keyCode = KeyCode::RightArrow;
            break;
        case DigitalButton::DownArrow:
            keyCode = KeyCode::DownArrow;
            break;
        case DigitalButton::Insert:
            keyCode = KeyCode::Insert;
            break;
        case DigitalButton::Backspace:
            keyCode = KeyCode::Backspace;
            break;
        case DigitalButton::Delete:
            keyCode = KeyCode::Delete;
            break;
        case DigitalButton::Num0:
            keyCode = KeyCode::Num0;
            break;
        case DigitalButton::Num1:
            keyCode = KeyCode::Num1;
            break;
        case DigitalButton::Num2:
            keyCode = KeyCode::Num2;
            break;
        case DigitalButton::Num3:
            keyCode = KeyCode::Num3;
            break;
        case DigitalButton::Num4:
            keyCode = KeyCode::Num4;
            break;
        case DigitalButton::Num5:
            keyCode = KeyCode::Num5;
            break;
        case DigitalButton::Num6:
            keyCode = KeyCode::Num6;
            break;
        case DigitalButton::Num7:
            keyCode = KeyCode::Num7;
            break;
        case DigitalButton::Num8:
            keyCode = KeyCode::Num8;
            break;
        case DigitalButton::Num9:
            keyCode = KeyCode::Num9;
            break;
        case DigitalButton::LeftShift:
            keyCode = KeyCode::LeftShift;
            break;
        case DigitalButton::RightShift:
            // TODO: 추가
            break;
        case DigitalButton::LeftCtrl:
            keyCode = KeyCode::LeftControl;
            break;
        case DigitalButton::RightCtrl:
            // TODO: 추가
            break;
        case DigitalButton::LeftAlt:
            keyCode = KeyCode::LeftAlt;
            break;
        case DigitalButton::RightAlt:
            // TODO: 추가
            break;
        case DigitalButton::F1:
            keyCode = KeyCode::F1;
            break;
        case DigitalButton::F2:
            keyCode = KeyCode::F2;
            break;
        case DigitalButton::F3:
            keyCode = KeyCode::F3;
            break;
        case DigitalButton::F4:
            keyCode = KeyCode::F4;
            break;
        case DigitalButton::F5:
            keyCode = KeyCode::F5;
            break;
        case DigitalButton::F6:
            keyCode = KeyCode::F6;
            break;
        case DigitalButton::F7:
            keyCode = KeyCode::F7;
            break;
        case DigitalButton::F8:
            keyCode = KeyCode::F8;
            break;
        case DigitalButton::F9:
            keyCode = KeyCode::F9;
            break;
        case DigitalButton::F10:
            keyCode = KeyCode::F10;
            break;
        case DigitalButton::F11:
            keyCode = KeyCode::F11;
            break;
        case DigitalButton::F12:
            keyCode = KeyCode::F12;
            break;
        case DigitalButton::NumPad0:
            keyCode = KeyCode::NumPad0;
            break;
        case DigitalButton::NumPad1:
            keyCode = KeyCode::NumPad1;
            break;
        case DigitalButton::NumPad2:
            keyCode = KeyCode::NumPad2;
            break;
        case DigitalButton::NumPad3:
            keyCode = KeyCode::NumPad3;
            break;
        case DigitalButton::NumPad4:
            keyCode = KeyCode::NumPad4;
            break;
        case DigitalButton::NumPad5:
            keyCode = KeyCode::NumPad5;
            break;
        case DigitalButton::NumPad6:
            keyCode = KeyCode::NumPad6;
            break;
        case DigitalButton::NumPad7:
            keyCode = KeyCode::NumPad7;
            break;
        case DigitalButton::NumPad8:
            keyCode = KeyCode::NumPad8;
            break;
        case DigitalButton::NumPad9:
            keyCode = KeyCode::NumPad9;
            break;

        default:
            break;
        }

        return keyCode;
    }
} // namespace cube
