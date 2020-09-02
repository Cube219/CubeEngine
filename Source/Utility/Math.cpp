#include "Math.h"

namespace cube
{
    float Math::Rad2Deg(float rad)
    {
        return rad * 180.0f / Pi;
    }

    float Math::Deg2Rad(float deg)
    {
        return deg * Pi / 180.0f;
    }

    float Math::Sin(float rad) // TODO: 차후 구현
    {
        return PrecisionSin(rad);
    }

    float Math::Cos(float rad) // TODO: 차후 구현
    {
        return PrecisionCos(rad);
    }

    float Math::Tan(float rad) // TODO: 차후 구현
    {
        return PrecisionTan(rad);
    }

    float Math::PrecisionSin(float rad)
    {
        return std::sinf(rad);
    }

    float Math::PrecisionCos(float rad)
    {
        return std::cosf(rad);
    }

    float Math::PrecisionTan(float rad)
    {
        return std::tanf(rad);
    }

    float Math::Log(float v)
    {
        return std::log(v);
    }

    float Math::Log10(float v)
    {
        return std::log10(v);
    }

    float Math::Log2(float v)
    {
        return std::log2(v);
    }
}
