#pragma once

#include "Defines.h"

#include "Format.h"

#ifndef CUBE_VECTOR_USE_SSE
#define CUBE_VECTOR_USE_SSE 0
#endif

#ifndef CUBE_VECTOR_USE_NEON
#define CUBE_VECTOR_USE_NEON 0
#endif

#if CUBE_VECTOR_USE_SSE
#include <immintrin.h>
namespace cube
{
    template <int N>
    using VectorData = __m128;
} // namespace cube
#elif CUBE_VECTOR_USE_NEON
#include <arm_neon.h>
namespace cube
{
    template <int N>
    using VectorData = float32x4_t;
} // namespace cube
#else
namespace cube
{
    template <int N>
    using VectorData = float[N];
} // namespace cube
#endif

namespace cube
{
    struct Float2
    {
        float x;
        float y;
    };

    struct Float3
    {
        float x;
        float y;
        float z;
    };

    struct Float4
    {
        float x;
        float y;
        float z;
        float w;
    };

    class Matrix;

    template <int N>
    class VectorBase;

    template <int N>
    VectorBase<N> operator* (float lhs, const VectorBase<N>& rhs);
    template <int N>
    VectorBase<N> operator/ (float lhs, const VectorBase<N>& rhs);

    template <int N>
    class VectorBase
    {
        static_assert(2 <= N && N <= 4, "Only 2, 3 and 4 dimension can be used.");

    public:
        static VectorBase Zero();
    
        VectorBase() = default;
        ~VectorBase() = default;

        explicit VectorBase(float v);
        VectorBase(float x, float y);
        VectorBase(float x, float y, float z);
        VectorBase(float x, float y, float z, float w);

        VectorBase(const VectorBase& other);
        template <int M>
        explicit VectorBase(const VectorBase<M>& other);

        VectorBase& operator=(const VectorBase& rhs);
        VectorBase& operator= (float rhs);
    
        bool operator== (const VectorBase& rhs) const;
        bool operator!= (const VectorBase& rhs) const;
    
        VectorBase operator+(const VectorBase& rhs) const;
        VectorBase operator-(const VectorBase& rhs) const;
        VectorBase operator* (float rhs) const;
        VectorBase operator*(const VectorBase& rhs) const;
        VectorBase operator/ (float rhs) const;
        VectorBase operator/(const VectorBase& rhs) const;
    
        const VectorBase& operator+() const;
        VectorBase operator-() const;
    
        VectorBase& operator+=(const VectorBase& rhs);
        VectorBase& operator-=(const VectorBase& rhs);
        VectorBase& operator*= (float rhs);
        VectorBase& operator*=(const VectorBase& rhs);
        VectorBase& operator/= (float rhs);
        VectorBase& operator/=(const VectorBase& rhs);

        void Swap(VectorBase& other);
        static void Swap(VectorBase& lhs, VectorBase& rhs);

        Float2 GetFloat2() const;
        Float3 GetFloat3() const;
        Float4 GetFloat4() const;

        float Length() const;
        float SquareLength() const;
        VectorBase LengthV() const;
        VectorBase SquareLengthV() const;

        void Normalize();
        VectorBase Normalized();

        float Dot(const VectorBase& rhs) const;
        static float Dot(const VectorBase& lhs, const VectorBase& rhs);
        VectorBase DotV(const VectorBase& rhs) const;
        static VectorBase DotV(const VectorBase& lhs, const VectorBase& rhs);

        VectorBase Cross(const VectorBase& rhs) const;
        static VectorBase Cross(const VectorBase& lhs, const VectorBase& rhs);
    
    private:
        template <int M>
        friend class VectorBase;
        friend class Matrix;
        friend class MatrixUtility;
    
        VectorData<N> mData;
    
        friend VectorBase operator*<> (float lhs, const VectorBase& rhs);
        friend VectorBase operator/<> (float lhs, const VectorBase& rhs);
        friend VectorBase<4> operator* (const VectorBase<4>& lhs, const Matrix& rhs);
    };

    using Vector2 = VectorBase<2>;
    using Vector3 = VectorBase<3>;
    using Vector4 = VectorBase<4>;
} // namespace cube

namespace fmt
{
    using namespace cube;

    // Float formatting
    template <typename Char>
    struct formatter<Float2, Char> : cube_formatter<Char>
    {
        template <typename FormatContext>
        auto format(const Float2& f2, FormatContext& ctx) const
        {
            return cube_formatter<Char>::cube_format(ctx, CUBE_T("({:.3f}, {:.3f})"), f2.x, f2.y);
        }
    };

    template <typename Char>
    struct formatter<Float3, Char> : cube_formatter<Char>
    {
        template <typename FormatContext>
        auto format(const Float3& f3, FormatContext& ctx) const
        {
            return cube_formatter<Char>::cube_format(ctx, CUBE_T("({:.3f}, {:.3f}, {:.3f})"), f3.x, f3.y, f3.z);
        }
    };

    template <typename Char>
    struct formatter<Float4, Char> : cube_formatter<Char>
    {
        template <typename FormatContext>
        auto format(const Float4& f4, FormatContext& ctx) const
        {
            return cube_formatter<Char>::cube_format(ctx, CUBE_T("({:.3f}, {:.3f}, {:.3f}, {:.3f})"), f4.x, f4.y, f4.z, f4.w);
        }
    };

    // Vector formatting
    template <typename Char>
    struct formatter<Vector2, Char> : cube_formatter<Char>
    {
        template <typename FormatContext>
        auto format(const Vector2& v2, FormatContext& ctx) const
        {
            Float2 f2 = v2.GetFloat2();
            return cube_formatter<Char>::cube_format(ctx, CUBE_T("({:.3f}, {:.3f})"), f2.x, f2.y);
        }
    };

    template <typename Char>
    struct formatter<Vector3, Char> : cube_formatter<Char>
    {
        template <typename FormatContext>
        auto format(const Vector3& v3, FormatContext& ctx) const
        {
            Float3 f3 = v3.GetFloat3();
            return cube_formatter<Char>::cube_format(ctx, CUBE_T("({:.3f}, {:.3f}, {:.3f})"), f3.x, f3.y, f3.z);
        }
    };

    template <typename Char>
    struct formatter<Vector4, Char> : cube_formatter<Char>
    {
        template <typename FormatContext>
        auto format(const Vector4& v4, FormatContext& ctx) const
        {
            Float4 f4 = v4.GetFloat4();
            return cube_formatter<Char>::cube_format(ctx, CUBE_T("({:.3f}, {:.3f}, {:.3f}, {:.3f})"), f4.x, f4.y, f4.z, f4.w);
        }
    };
} // namespace fmt

// Include inline function definition
#ifndef CUBE_VECTOR_IMPLEMENTATION

#if CUBE_VECTOR_USE_SSE
#include "VectorImpl/VectorSSE.inl"
#elif CUBE_VECTOR_USE_NEON
#include "VectorImpl/VectorNEON.inl"
#else
#include "VectorImpl/VectorArray.inl"
#endif

#endif // !CUBE_VECTOR_IMPLEMENTATION
