#pragma once

#include "InputModuleHeader.h"

#include "Core/Module/Module.h"
#include "Utility/Vector.h"
#include "KeyboardMouseInput.h"
#include "XboxControllerInput.h"

#include <functional>

namespace cube
{
    struct ButtonInfo
    {
        std::function<float()> getButtonValue;
    };
#define KM_DIGIT_BTN_INFO(btn) ButtonInfo{[this](){return mKMInput.IsButtonPressed(btn) == true ? 1.0f : 0.0f;}}
#define XBOX_DIGIT_BTN_INFO(playerIdx, btn) ButtonInfo{[this](){return mXboxInput.IsButtonPressed(playerIdx, btn) == true ? 1.0f : 0.0f;}}
#define XBOX_ANALOG_BTN_INFO(playerIdx, btn) ButtonInfo{[this](){return mXboxInput.GetAnalogValue(playerIdx, btn);}}

    struct Action
    {
        bool isPressed;
        Vector<ButtonInfo> bindedDigitalButtons;
    };
    struct Axis
    {
        struct VirtualButtonInfo
        {
            ButtonInfo digitalButton;
            float sensitivity;
            float scale;

            float currentValue = 0.0f;
        };
        float deadZone;
        float sensitivityToZero;

        bool isClamped;
        float currentValue;
        Vector<ButtonInfo> bindedAnalogButtons;
        Vector<VirtualButtonInfo> bindedVirtualButtons;
    };

    class INPUTMODULE_EXPORT InputModule : public Module
    {
    public:
        InputModule();
        virtual ~InputModule();

        virtual void OnInit() override;
        virtual void OnUpdate(float dt) override;
        virtual void OnDestroy() override;

        bool IsActionPressed(StringView name);
        float GetAxisValue(StringView name);

        void LockCursor();
        void UnlockCursor();

        void SendVibration(Uint32 playerIndex, float time, float intensity);
        void SendVibration(Uint32 playerIndex, float time, float leftIntensity, float rightIntensity);

        Vector2 GetMousePosition();

    private:
        KeyboardMouseInput mKMInput;
        XboxControllerInput mXboxInput;

        HashMap<String, Action> mActions;
        HashMap<String, Axis> mAxes;

        Array<bool, 4> mIsVibrated;
        Array<float, 4> mRemainedVibrationTime;
    };
    CREATE_MODULE_FUNC(InputModule, INPUTMODULE_EXPORT)
} // namespace cube
