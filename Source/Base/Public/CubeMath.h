#pragma once

#include <cmath>

namespace cube
{
    class Math
    {
    public:
        static constexpr float Pi = 3.141592653f;

        static float Rad2Deg(float rad)
        {
            return rad * 180.0f / Pi;
        }
        static float Deg2Rad(float deg)
        {
            return deg * Pi / 180.0f;
        }

        // TODO: Use approximation
        static float Sin(float rad)
        {
            return PrecisionSin(rad);
        }
        static float Cos(float rad)
         {
            return PrecisionCos(rad);
        }
        static float Tan(float rad)
        {
            return PrecisionTan(rad);
        }

        static float PrecisionSin(float rad)
        {
            return std::sinf(rad);
        }
        static float PrecisionCos(float rad)
        {
            return std::cosf(rad);
        }
        static float PrecisionTan(float rad)
        {
            return std::tanf(rad);
        }

        static float Log(float v)
        {
            return std::log(v);
        }
        static float Log10(float v)
        {
            return std::log10(v);
        }
        static float Log2(float v)
        {
            return std::log2(v);
        }

        template <typename T>
        static T Min(const T a, const T b)
        {
            return (a <= b) ? a : b;
        }
        template <typename T>
        static T Max(const T a, const T b)
        {
            return (a >= b) ? a : b;
        }
    };
} // namespace cube
