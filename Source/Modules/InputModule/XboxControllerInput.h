#pragma once

#include "InputModuleHeader.h"

#ifdef _WIN32
#include <Windows.h>
#include <Xinput.h>
#endif // _WIN32

namespace cube
{
    class XboxControllerInput
    {
    public:
        enum class DigitalButton
        {
            X, Y, A, B, Back, Start, LeftButton, RightButton,
            DPadLeft, DPadRight, DPadUp, DPadDown, LeftStick, RightStick,
            LeftTrigger, RightTrigger,
            /*LeftStickLeft, LeftStickRight, LeftStickUp, LeftStickDown,
            RightStickLeft, RightStickRight, RightStickUp, RightStickDown*/ // TODO: Stick들의 Digital화
        };

        enum class AnalogButton
        {
            LeftStickX, LeftStickY, RightStickX, RightStickY,
            LeftTrigger, RightTrigger
        };

        XboxControllerInput();
        ~XboxControllerInput();

        bool IsButtonPressed(Uint32 playerIndex, DigitalButton button);
        float GetAnalogValue(Uint32 playerIndex, AnalogButton button);
        void SetVibration(Uint32 playerIndex, float leftIntensity, float rightIntensity);

        void Update();

    private:
#ifdef _WIN32
        Array<XINPUT_STATE, XUSER_MAX_COUNT> mState;
        Array<bool, XUSER_MAX_COUNT> mStateAvailable;
#endif // _WIN32
    };
} // namespace cube
