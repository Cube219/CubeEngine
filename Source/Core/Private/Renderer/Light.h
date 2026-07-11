#pragma once

#include "CoreHeader.h"

#include "Vector.h"

namespace cube
{
    class Light
    {
    public:
        Light() = default;
        virtual ~Light() = default;

        bool IsEnabled() const { return mEnabled; }
        void SetEnable(bool enable);

        const Float3& GetPosition() const { return mPosition; }
        const Float3& GetDirection() const { return mDirection; }
        const Float3& GetIntensity() const { return mIntensity; }

        void SetPosition(const Float3& newPosition);
        void SetDirection(const Float3& newDirection);
        void SetIntensity(const Float3& newIntensity);

        virtual void OnLoopImGUIContent() = 0;

    protected:
        bool mEnabled = true;

        Float3 mPosition = { 0.0f, 0.0f, 0.0f }; // TODO: Use Vector?
        Float3 mDirection = { 1.0f, 1.0f, 1.0f }; // TODO: Use Vector?
        Float3 mIntensity = { 1.0f, 1.0f, 1.0f };
    };

    class DirectionalLight : public Light
    {
    public:
        DirectionalLight() = default;

        virtual void OnLoopImGUIContent() override;
    };

    class PointLight : public Light
    {
    public:
        PointLight() = default;

        virtual void OnLoopImGUIContent() override;
    };
} // namespace cube
