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
    };

    class DirectionalLight : public Light
    {
    public:
        DirectionalLight() = default;

        const Float3& GetDirection() const { return mDirection; }
        const Float3& GetIntensity() const { return mIntensity; }

        void SetDirection(const Float3& newDirection) { mDirection = newDirection; }
        void SetIntensity(const Float3& newIntensity) { mIntensity = newIntensity; }

    private:
        Float3 mDirection;
        Float3 mIntensity;
    };
} // namespace cube
