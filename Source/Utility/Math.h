#pragma once

#include <cmath>

namespace cube
{
    class Math
    {
    public:
        static constexpr float Pi = 3.141592653f;

        static float Rad2Deg(float rad);
        static float Deg2Rad(float deg);

        // TODO: Use approximation
        static float Sin(float rad);
        static float Cos(float rad);
        static float Tan(float rad);

        static float PrecisionSin(float rad);
        static float PrecisionCos(float rad);
        static float PrecisionTan(float rad);

        static float Log(float v);
        static float Log10(float v);
        static float Log2(float v);

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
