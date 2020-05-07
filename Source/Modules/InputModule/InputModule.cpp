#include "InputModule.h"

namespace cube
{
    InputModule::InputModule()
    {
        mInfo.name = "InputModule";
    }

    InputModule::~InputModule()
    {
    }

    void InputModule::OnInit()
    {
        for(int i = 0; i < 4; i++) {
            mIsVibrated[i] = false;
        }

        Action jump;
        jump.bindedDigitalButtons.push_back(KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::SpaceBar));
        mActions[CUBE_T("Jump")] = jump;

        Action submit;
        submit.bindedDigitalButtons.push_back(KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::Enter));
        submit.bindedDigitalButtons.push_back(XBOX_DIGIT_BTN_INFO(0, XboxControllerInput::DigitalButton::A));
        mActions[CUBE_T("Submit")] = submit;

        Action cancel;
        cancel.bindedDigitalButtons.push_back(KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::Escape));
        submit.bindedDigitalButtons.push_back(XBOX_DIGIT_BTN_INFO(0, XboxControllerInput::DigitalButton::B));
        mActions[CUBE_T("Cancel")] = cancel;

        Axis moveHorizontally;
        moveHorizontally.isClamped = true;
        moveHorizontally.currentValue = 0.0f;
        moveHorizontally.deadZone = 0.001f;
        moveHorizontally.sensitivityToZero = 10.0f;
        moveHorizontally.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::D), 10.0f, 1.0f });
        moveHorizontally.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::A), 10.0f, -1.0f });
        moveHorizontally.bindedAnalogButtons.push_back(XBOX_ANALOG_BTN_INFO(0, XboxControllerInput::AnalogButton::LeftStickX));
        mAxes[CUBE_T("MoveHorizontally")] = moveHorizontally;

        Axis moveVertically;
        moveVertically.isClamped = true;
        moveVertically.currentValue = 0.0f;
        moveVertically.deadZone = 0.001f;
        moveVertically.sensitivityToZero = 10.0f;
        moveVertically.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::W), 10.0f, 1.0f });
        moveVertically.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::S), 10.0f, -1.0f });
        moveVertically.bindedAnalogButtons.push_back(XBOX_ANALOG_BTN_INFO(0, XboxControllerInput::AnalogButton::LeftStickY));
        mAxes[CUBE_T("MoveVertically")] = moveVertically;

        Axis moveUpDown;
        moveUpDown.isClamped = true;
        moveUpDown.currentValue = 0.0f;
        moveUpDown.deadZone = 0.001f;
        moveUpDown.sensitivityToZero = 10.0f;
        moveUpDown.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::E), 10.0f, 1.0f });
        moveUpDown.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::Q), 10.0f, -1.0f });
        moveUpDown.bindedVirtualButtons.push_back({ XBOX_DIGIT_BTN_INFO(0, XboxControllerInput::DigitalButton::RightTrigger), 10.0f, 1.0f });
        moveUpDown.bindedVirtualButtons.push_back({ XBOX_DIGIT_BTN_INFO(0, XboxControllerInput::DigitalButton::LeftTrigger), 10.0f, -1.0f });
        mAxes[CUBE_T("MoveUpDown")] = moveUpDown;

        Axis lookHorizontally;
        lookHorizontally.isClamped = false;
        lookHorizontally.currentValue = 0.0f;
        lookHorizontally.deadZone = 0.001f;
        lookHorizontally.sensitivityToZero = 10.0f;
        lookHorizontally.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::RightArrow), 10.0f, 1.0f });
        lookHorizontally.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::LeftArrow), 10.0f, -1.0f });
        lookHorizontally.bindedAnalogButtons.push_back(ButtonInfo{ [this]() {
            Vector2 mouseDelta = mKMInput.GetMouseDeltaPosition();
            Float2 f2;
            f2 = mouseDelta.GetFloat2();
            return f2.x;
        } });
        lookHorizontally.bindedAnalogButtons.push_back(XBOX_ANALOG_BTN_INFO(0, XboxControllerInput::AnalogButton::RightStickX));
        mAxes[CUBE_T("LookHorizontally")] = lookHorizontally;

        Axis lookVertically;
        lookVertically.isClamped = false;
        lookVertically.currentValue = 0.0f;
        lookVertically.deadZone = 0.001f;
        lookVertically.sensitivityToZero = 10.0f;
        lookVertically.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::UpArrow), 10.0f, 1.0f });
        lookVertically.bindedVirtualButtons.push_back({ KM_DIGIT_BTN_INFO(KeyboardMouseInput::DigitalButton::DownArrow), 10.0f, -1.0f });
        lookVertically.bindedAnalogButtons.push_back(ButtonInfo{ [this]() {
            Vector2 mouseDelta = mKMInput.GetMouseDeltaPosition();
            Float2 f2;
            f2 = mouseDelta.GetFloat2();
            return -f2.y;
        } });
        lookVertically.bindedAnalogButtons.push_back(XBOX_ANALOG_BTN_INFO(0, XboxControllerInput::AnalogButton::RightStickY));
        mAxes[CUBE_T("LookVertically")] = lookVertically;

        Axis triggerLeft;
        triggerLeft.bindedAnalogButtons.push_back(XBOX_ANALOG_BTN_INFO(0, XboxControllerInput::AnalogButton::LeftTrigger));
        mAxes[CUBE_T("TriggerLeft")] = triggerLeft;

        Axis triggerRight;
        triggerRight.bindedAnalogButtons.push_back(XBOX_ANALOG_BTN_INFO(0, XboxControllerInput::AnalogButton::RightTrigger));
        mAxes[CUBE_T("TriggerRight")] = triggerRight;
    }

    void InputModule::OnUpdate(float dt)
    {
        mKMInput.UpdateMouseDelta(dt);

        mXboxInput.Update();

        for(int i = 0; i < 4; i++) {
            if(mIsVibrated[i] == false)
                continue;

            mRemainedVibrationTime[i] -= dt;
            if(mRemainedVibrationTime[i] < 0.0f) {
                mIsVibrated[i] = false;
                mXboxInput.SetVibration(i, 0.0f, 0.0f);
            }
        }

        for(auto& action : mActions) {
            bool isPressed = false;

            for(auto& btnInfo : action.second.bindedDigitalButtons) {
                if(btnInfo.getButtonValue() > 0.0f) {
                    isPressed = true;
                    break;
                }
            }

            action.second.isPressed = isPressed;
        }

        for(auto& axis : mAxes) {
            float value = 0.0f;

            // Analog button
            for(auto& aBtn : axis.second.bindedAnalogButtons) {
                value += aBtn.getButtonValue();
            }

            // Virtual button
            for(auto& vBtn : axis.second.bindedVirtualButtons) {
                bool isPressed = false;
                float deltaVal = 0.0f;
                float vBtnCurrentVal = vBtn.currentValue;

                if(vBtn.digitalButton.getButtonValue() > 0.0f) {
                    isPressed = true;
                    deltaVal += vBtn.sensitivity * vBtn.scale * dt;
                }

                if(vBtnCurrentVal != 0.0f && isPressed == false) {
                    // Go to zero
                    float sign = vBtnCurrentVal > 0.0f ? -1.0f : 1.0f;
                    deltaVal = axis.second.sensitivityToZero * sign * dt;

                    if((sign < 0.0f && vBtnCurrentVal + deltaVal < 0.0f) || (sign > 0.0f && vBtnCurrentVal + deltaVal > 0.0f)) {
                        deltaVal = -vBtnCurrentVal;
                    }
                }

                vBtnCurrentVal += deltaVal;

                if(vBtnCurrentVal > 1.0f)
                    vBtnCurrentVal = 1.0f;
                else if(vBtnCurrentVal < -1.0f)
                    vBtnCurrentVal = -1.0f;

                float vBtnCurrentValAbs = vBtnCurrentVal > 0.0f ? vBtnCurrentVal : -vBtnCurrentVal;
                if(vBtnCurrentValAbs != 0.0f && vBtnCurrentValAbs <= axis.second.deadZone) {
                    vBtnCurrentVal = 0.0f;
                }

                vBtn.currentValue = vBtnCurrentVal;
                value += vBtnCurrentVal;
            }

            if(axis.second.isClamped == true) {
                if(value > 1.0f)
                    value = 1.0f;
                if(value < -1.0f)
                    value = -1.0f;
            }

            axis.second.currentValue = value;
        }
    }

    void InputModule::OnDestroy()
    {
    }

    bool InputModule::IsActionPressed(StringView name)
    {
        return mActions[name.data()].isPressed;
    }

    float InputModule::GetAxisValue(StringView name)
    {
        return mAxes[name.data()].currentValue;
    }

    void InputModule::LockCursor()
    {
        mKMInput.LockCursor();
    }

    void InputModule::UnlockCursor()
    {
        mKMInput.UnlockCursor();
    }

    void InputModule::SendVibration(Uint32 playerIndex, float time, float intensity)
    {
        mIsVibrated[playerIndex] = true;
        mRemainedVibrationTime[playerIndex] = time;

        mXboxInput.SetVibration(playerIndex, intensity, intensity);
    }

    void InputModule::SendVibration(Uint32 playerIndex, float time, float leftIntensity, float rightIntensity)
    {
        mIsVibrated[playerIndex] = true;
        mRemainedVibrationTime[playerIndex] = time;

        mXboxInput.SetVibration(playerIndex, leftIntensity, rightIntensity);
    }

    Vector2 InputModule::GetMousePosition()
    {
        return mKMInput.GetMousePosition();
    }
} // namespace cube
