#include "XboxControllerInput.h"

#include "Core/LogWriter.h"

namespace cube
{
    XboxControllerInput::XboxControllerInput()
    {
    }

    XboxControllerInput::~XboxControllerInput()
    {
    }

    // ----- Win32 version -----
#ifdef _WIN32
    bool XboxControllerInput::IsButtonPressed(Uint32 playerIndex, DigitalButton button)
    {
#ifdef _DEBUG
        if(playerIndex >= XUSER_MAX_COUNT) {
            CUBE_LOG(LogType::Error, "Invalid player index. (index: {0} / maxIndex: {1})", playerIndex, XUSER_MAX_COUNT - 1);
        }
#endif // _DEBUG

        if(mStateAvailable[playerIndex] == false)
            return false;

        WORD padBtn = mState[playerIndex].Gamepad.wButtons;

        switch(button) {
        case DigitalButton::X:
        {
            if((padBtn & XINPUT_GAMEPAD_X) > 0)
                return true;
            else
                return false;
        }

        case DigitalButton::Y:
        {
            if((padBtn & XINPUT_GAMEPAD_Y) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::A:
        {
            if((padBtn & XINPUT_GAMEPAD_A) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::B:
        {
            if((padBtn & XINPUT_GAMEPAD_B) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::Back:
        {
            if((padBtn & XINPUT_GAMEPAD_BACK) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::Start:
        {
            if((padBtn & XINPUT_GAMEPAD_START) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::LeftButton:
        {
            if((padBtn & XINPUT_GAMEPAD_LEFT_SHOULDER) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::RightButton:
        {
            if((padBtn & XINPUT_GAMEPAD_RIGHT_SHOULDER) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::DPadLeft:
        {
            if((padBtn & XINPUT_GAMEPAD_DPAD_LEFT) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::DPadRight:
        {
            if((padBtn & XINPUT_GAMEPAD_DPAD_RIGHT) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::DPadUp:
        {
            if((padBtn & XINPUT_GAMEPAD_DPAD_UP) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::DPadDown:
        {
            if((padBtn & XINPUT_GAMEPAD_DPAD_DOWN) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::LeftStick:
        {
            if((padBtn & XINPUT_GAMEPAD_LEFT_THUMB) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::RightStick:
        {
            if((padBtn & XINPUT_GAMEPAD_RIGHT_THUMB) > 0)
                return true;
            else
                return false;
        }
        case DigitalButton::LeftTrigger:
        {
            BYTE triggerValue;
            triggerValue = mState[playerIndex].Gamepad.bLeftTrigger;
            if(triggerValue > 180)
                return true;
            else
                return false;
        }
        case DigitalButton::RightTrigger:
        {
            BYTE triggerValue;
            triggerValue = mState[playerIndex].Gamepad.bRightTrigger;
            if(triggerValue > 180)
                return true;
            else
                return false;
        }
        default:
            CUBE_LOG(LogType::Error, "Invalid digital button. ({0})", (int)button);
            return false;
        }

        return false;
    }

    float XboxControllerInput::GetAnalogValue(Uint32 playerIndex, AnalogButton button)
    {
#ifdef _DEBUG
        if(playerIndex >= XUSER_MAX_COUNT) {
            CUBE_LOG(LogType::Error, "Invalid player index. (index: {0} / maxIndex: {1})", playerIndex, XUSER_MAX_COUNT - 1);
            return 0.0f;
        }
#endif // _DEBUG

        if(mStateAvailable[playerIndex] == false)
            return 0.0f;

        SHORT stickValue;
        SHORT stickValueAbs;

        BYTE triggerValue;

        switch(button) {
        case AnalogButton::LeftStickX:
        {
            stickValue = mState[playerIndex].Gamepad.sThumbLX;
            stickValueAbs = stickValue >= 0 ? stickValue : -stickValue;
            if(stickValue != -32768 && stickValueAbs < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
                stickValue = 0;

            return static_cast<float>(stickValue) / 32767.0f;
        }
        case AnalogButton::LeftStickY:
        {
            stickValue = mState[playerIndex].Gamepad.sThumbLY;
            stickValueAbs = stickValue >= 0 ? stickValue : -stickValue;
            if(stickValue != -32768 && stickValueAbs < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
                stickValue = 0;

            return static_cast<float>(stickValue) / 32767.0f;
        }
        case AnalogButton::RightStickX:
        {
            stickValue = mState[playerIndex].Gamepad.sThumbRX;
            stickValueAbs = stickValue >= 0 ? stickValue : -stickValue;
            if(stickValue != -32768 && stickValueAbs < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                stickValue = 0;

            return static_cast<float>(stickValue) / 32767.0f;
        }
        case AnalogButton::RightStickY:
        {
            stickValue = mState[playerIndex].Gamepad.sThumbRY;
            stickValueAbs = stickValue >= 0 ? stickValue : -stickValue;
            if(stickValue != -32768 && stickValueAbs < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                stickValue = 0;

            return static_cast<float>(stickValue) / 32767.0f;
        }
        case AnalogButton::LeftTrigger:
        {
            triggerValue = mState[playerIndex].Gamepad.bLeftTrigger;
            if(triggerValue <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
                triggerValue = 0;
            return static_cast<float>(triggerValue) / 255.0f;
        }
        case AnalogButton::RightTrigger:
        {
            triggerValue = mState[playerIndex].Gamepad.bRightTrigger;
            if(triggerValue <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
                triggerValue = 0;
            return static_cast<float>(triggerValue) / 255.0f;
        }
        default:
            CUBE_LOG(LogType::Error, "Invalid analog button. ({0})", (int)button);
            return 0.0f;
        }

        return 0.0f;
    }

    void XboxControllerInput::SetVibration(Uint32 playerIndex, float leftIntensity, float rightIntensity)
    {
#ifdef _DEBUG
        if(playerIndex >= XUSER_MAX_COUNT) {
            CUBE_LOG(LogType::Error, "Invalid player index. (index: {0} / maxIndex: {1})", playerIndex, XUSER_MAX_COUNT - 1);
            return;
        }
#endif // _DEBUG

        if(mStateAvailable[playerIndex] == false)
            return;

        XINPUT_VIBRATION vib;
        vib.wLeftMotorSpeed = SCast(WORD)(65535 * leftIntensity);
        vib.wRightMotorSpeed = SCast(WORD)(65535 * rightIntensity);

        XInputSetState(playerIndex, &vib);
    }

    void XboxControllerInput::Update()
    {
        for(DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
            DWORD res = XInputGetState(i, &mState[i]);

            if(res != ERROR_SUCCESS) {
                mStateAvailable[i] = false;
                continue;
            }

            mStateAvailable[i] = true;
        }
    }
#else // _WIN32 // ----- Other platforms (don't support) -----
    bool XboxControllerInput::IsButtonPressed(Uint32 playerIndex, DigitalButton button)
    {
        return false;
    }

    float XboxControllerInput::GetAnalogValue(Uint32 playerIndex, AnalogButton button)
    {
        return 0.0f;
    }

    void XboxControllerInput::SetVibration(Uint32 playerIndex, float leftIntensity, float rightIntensity)
    {
    }

    void XboxControllerInput::Update()
    {
    }
#endif // _WIN32
} // namespace cube
