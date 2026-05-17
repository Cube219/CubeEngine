#pragma once

#include "Defines.h"

#include "Format.h"

#ifndef CUBE_VECTOR_USE_AVX2
#define CUBE_VECTOR_USE_AVX2 0
#endif

#ifndef CUBE_VECTOR_USE_SSE
#define CUBE_VECTOR_USE_SSE 0
#endif

#ifndef CUBE_VECTOR_USE_NEON
#define CUBE_VECTOR_USE_NEON 0
#endif

#if CUBE_VECTOR_USE_AVX2 || CUBE_VECTOR_USE_SSE
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
    template <typename T>
    struct PrimitiveVector2
    {
        T x;
        T y;

        PrimitiveVector2 operator+(const PrimitiveVector2& rhs) const { return PrimitiveVector2{ x + rhs.x, y + rhs.y }; }
        PrimitiveVector2 operator-(const PrimitiveVector2& rhs) const { return PrimitiveVector2{ x - rhs.x, y - rhs.y }; }
        PrimitiveVector2 operator*(T rhs) const { return PrimitiveVector2{ x * rhs, y * rhs }; }
        PrimitiveVector2 operator*(const PrimitiveVector2& rhs) const { return PrimitiveVector2{ x * rhs.x, y * rhs.y }; }
        PrimitiveVector2 operator/(T rhs) const { return PrimitiveVector2{ x / rhs, y / rhs }; }
        PrimitiveVector2 operator/(const PrimitiveVector2& rhs) const { return PrimitiveVector2{ x / rhs.x, y / rhs.y }; }

        PrimitiveVector2 operator-() const { return PrimitiveVector2{ -x, -y }; }
        const PrimitiveVector2& operator+() const { return *this; }

        PrimitiveVector2& operator+=(const PrimitiveVector2& rhs) { x += rhs.x; y += rhs.y; return *this; }
        PrimitiveVector2& operator-=(const PrimitiveVector2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
        PrimitiveVector2& operator*=(T rhs) { x *= rhs; y *= rhs; return *this; }
        PrimitiveVector2& operator*=(const PrimitiveVector2& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
        PrimitiveVector2& operator/=(T rhs) { x /= rhs; y /= rhs; return *this; }
        PrimitiveVector2& operator/=(const PrimitiveVector2& rhs) { x /= rhs.x; y /= rhs.y; return *this; }

        bool operator==(const PrimitiveVector2& rhs) const { return x == rhs.x && y == rhs.y; }
        bool operator!=(const PrimitiveVector2& rhs) const { return !(*this == rhs); }
    };

    template <typename T>
    inline PrimitiveVector2<T> operator*(T lhs, const PrimitiveVector2<T>& rhs) { return PrimitiveVector2<T>{ lhs * rhs.x, lhs * rhs.y }; }
    template <typename T>
    inline PrimitiveVector2<T> operator/(T lhs, const PrimitiveVector2<T>& rhs) { return PrimitiveVector2<T>{ lhs / rhs.x, lhs / rhs.y }; }

    template <typename T>
    struct PrimitiveVector3
    {
        T x;
        T y;
        T z;

        PrimitiveVector3 operator+(const PrimitiveVector3& rhs) const { return PrimitiveVector3{ x + rhs.x, y + rhs.y, z + rhs.z }; }
        PrimitiveVector3 operator-(const PrimitiveVector3& rhs) const { return PrimitiveVector3{ x - rhs.x, y - rhs.y, z - rhs.z }; }
        PrimitiveVector3 operator*(T rhs) const { return PrimitiveVector3{ x * rhs, y * rhs, z * rhs }; }
        PrimitiveVector3 operator*(const PrimitiveVector3& rhs) const { return PrimitiveVector3{ x * rhs.x, y * rhs.y, z * rhs.z }; }
        PrimitiveVector3 operator/(T rhs) const { return PrimitiveVector3{ x / rhs, y / rhs, z / rhs }; }
        PrimitiveVector3 operator/(const PrimitiveVector3& rhs) const { return PrimitiveVector3{ x / rhs.x, y / rhs.y, z / rhs.z }; }

        PrimitiveVector3 operator-() const { return PrimitiveVector3{ -x, -y, -z }; }
        const PrimitiveVector3& operator+() const { return *this; }

        PrimitiveVector3& operator+=(const PrimitiveVector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        PrimitiveVector3& operator-=(const PrimitiveVector3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        PrimitiveVector3& operator*=(T rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
        PrimitiveVector3& operator*=(const PrimitiveVector3& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
        PrimitiveVector3& operator/=(T rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }
        PrimitiveVector3& operator/=(const PrimitiveVector3& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

        bool operator==(const PrimitiveVector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
        bool operator!=(const PrimitiveVector3& rhs) const { return !(*this == rhs); }
    };

    template <typename T>
    inline PrimitiveVector3<T> operator*(T lhs, const PrimitiveVector3<T>& rhs) { return PrimitiveVector3<T>{ lhs * rhs.x, lhs * rhs.y, lhs * rhs.z }; }
    template <typename T>
    inline PrimitiveVector3<T> operator/(T lhs, const PrimitiveVector3<T>& rhs) { return PrimitiveVector3<T>{ lhs / rhs.x, lhs / rhs.y, lhs / rhs.z }; }

    template <typename T>
    struct PrimitiveVector4
    {
        T x;
        T y;
        T z;
        T w;

        PrimitiveVector4 operator+(const PrimitiveVector4& rhs) const { return PrimitiveVector4{ x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w }; }
        PrimitiveVector4 operator-(const PrimitiveVector4& rhs) const { return PrimitiveVector4{ x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w }; }
        PrimitiveVector4 operator*(T rhs) const { return PrimitiveVector4{ x * rhs, y * rhs, z * rhs, w * rhs }; }
        PrimitiveVector4 operator*(const PrimitiveVector4& rhs) const { return PrimitiveVector4{ x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w }; }
        PrimitiveVector4 operator/(T rhs) const { return PrimitiveVector4{ x / rhs, y / rhs, z / rhs, w / rhs }; }
        PrimitiveVector4 operator/(const PrimitiveVector4& rhs) const { return PrimitiveVector4{ x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w }; }

        PrimitiveVector4 operator-() const { return PrimitiveVector4{ -x, -y, -z, -w }; }
        const PrimitiveVector4& operator+() const { return *this; }

        PrimitiveVector4& operator+=(const PrimitiveVector4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
        PrimitiveVector4& operator-=(const PrimitiveVector4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
        PrimitiveVector4& operator*=(T rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
        PrimitiveVector4& operator*=(const PrimitiveVector4& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
        PrimitiveVector4& operator/=(T rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }
        PrimitiveVector4& operator/=(const PrimitiveVector4& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

        bool operator==(const PrimitiveVector4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        bool operator!=(const PrimitiveVector4& rhs) const { return !(*this == rhs); }
    };

    template <typename T>
    inline PrimitiveVector4<T> operator*(T lhs, const PrimitiveVector4<T>& rhs) { return PrimitiveVector4<T>{ lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w }; }
    template <typename T>
    inline PrimitiveVector4<T> operator/(T lhs, const PrimitiveVector4<T>& rhs) { return PrimitiveVector4<T>{ lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w }; }

    using Float2 = PrimitiveVector2<float>;
    using Float3 = PrimitiveVector3<float>;
    using Float4 = PrimitiveVector4<float>;

    using Int2 = PrimitiveVector2<Int32>;
    using Int3 = PrimitiveVector3<Int32>;
    using Int4 = PrimitiveVector4<Int32>;

    using Uint2 = PrimitiveVector2<Uint32>;
    using Uint3 = PrimitiveVector3<Uint32>;
    using Uint4 = PrimitiveVector4<Uint32>;

    class Matrix;

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
    
        template <int M>
        friend VectorBase<M> operator* (float lhs, const VectorBase<M>& rhs);
        template <int M>
        friend VectorBase<M> operator/ (float lhs, const VectorBase<M>& rhs);
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

#if CUBE_VECTOR_USE_AVX2
#include "VectorImpl/VectorAVX2.inl"
#elif CUBE_VECTOR_USE_SSE
#include "VectorImpl/VectorSSE.inl"
#elif CUBE_VECTOR_USE_NEON
#include "VectorImpl/VectorNEON.inl"
#else
#include "VectorImpl/VectorArray.inl"
#endif

#endif // !CUBE_VECTOR_IMPLEMENTATION
