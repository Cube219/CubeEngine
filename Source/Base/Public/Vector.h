#pragma once

#include "fmt/format.h"

#ifndef CUBE_VECTOR_USE_SSE
#define CUBE_VECTOR_USE_SSE 0 // TODO
#endif


#if CUBE_VECTOR_USE_SSE
#include <xmmintrin.h>
namespace cube
{
    using VectorData = __m128;
} // namespace cube
#else
namespace cube
{
    using VectorData = float[4];
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

    // Forward declarations
    class VectorBase;
    class Vector2;
    class Vector3;
    class Vector4;
    class Matrix;
    class MatrixUtility;
    VectorBase operator*(const Vector4& lhs, const Matrix& rhs);

    class VectorBase
    {
    public:
        static VectorBase Zero();

        VectorBase();
        ~VectorBase();

        //void Swap(Vector& other); // TODO: 구현하기

        VectorBase& operator= (const VectorBase& rhs);

        VectorBase& operator= (float rhs);

        bool operator== (const VectorBase& rhs) const; // TODO: 개별 Vector구현으로

        bool operator!= (const VectorBase& rhs) const; // TODO: 개별 Vector구현으로

        VectorBase operator+ (const VectorBase& rhs) const;
        VectorBase operator- (const VectorBase& rhs) const;
        VectorBase operator* (float rhs) const;
        VectorBase operator* (const VectorBase& rhs) const;
        VectorBase operator/ (float rhs) const;
        VectorBase operator/ (const VectorBase& rhs) const;

        const VectorBase& operator+() const;
        VectorBase operator-() const;

        VectorBase& operator+= (const VectorBase& rhs);
        VectorBase& operator-= (const VectorBase& rhs);
        VectorBase& operator*= (float rhs);
        VectorBase& operator*= (const VectorBase& rhs);
        VectorBase& operator/= (float rhs);
        VectorBase& operator/= (const VectorBase& rhs);

    protected:
        friend class Vector2;
        friend class Vector3;
        friend class Vector4;
        friend class Matrix;
        friend class MatrixUtility;

        explicit VectorBase(float x, float y, float z, float w);

        VectorData mData;

        friend VectorBase operator* (float lhs, const VectorBase& rhs);
        friend VectorBase operator/ (float lhs, const VectorBase& rhs);
        friend VectorBase operator* (const Vector4& lhs, const Matrix& rhs);
    };

    VectorBase operator* (float lhs, const VectorBase& rhs);
    VectorBase operator/ (float lhs, const VectorBase& rhs);

    class Vector2 : public VectorBase
    {
    public:
        Vector2();
        Vector2(float x, float y);

        Vector2(const VectorBase& vec);
        Vector2& operator=(const VectorBase& vec);

        operator Vector3() const;
        operator Vector4() const;

        Float2 GetFloat2() const;

        VectorBase Length() const;
        VectorBase SquareLength() const;

        VectorBase Dot(const Vector2& rhs) const;
        static VectorBase Dot(const Vector2& lhs, const Vector2& rhs);

        void Normalize();
        VectorBase Normalized() const;

    private:
        friend class Vector3;
        friend class Vector4;
        Vector2(const VectorData vData);
    };

    class Vector3 : public VectorBase
    {
    public:
        Vector3();
        Vector3(float x, float y, float z);

        Vector3(const VectorBase& vec);
        Vector3& operator=(const VectorBase& vec);

        operator Vector2() const;
        operator Vector4() const;

        Float3 GetFloat3() const;

        VectorBase Length() const;
        VectorBase SquareLength() const;

        VectorBase Dot(const Vector3& rhs) const;
        static VectorBase Dot(const Vector3& lhs, const Vector3& rhs);

        void Normalize();
        VectorBase Normalized() const;

        VectorBase Cross(const Vector3& rhs) const;
        static VectorBase Cross(const Vector3& lhs, const Vector3& rhs);

    private:
        friend class Vector2;
        friend class Vector4;
        Vector3(const VectorData vData);
    };

    class Vector4 : public VectorBase
    {
    public:
        Vector4();
        Vector4(float x, float y, float z, float w);

        Vector4(const VectorBase& vec);
        Vector4& operator=(const VectorBase& vec);

        operator Vector2() const;
        operator Vector3() const;

        Float4 GetFloat4() const;

        VectorBase Length() const;
        VectorBase SquareLength() const;

        VectorBase Dot(const Vector4& rhs) const;
        static VectorBase Dot(const Vector4& lhs, const Vector4& rhs);

        void Normalize();
        VectorBase Normalized() const;

    private:
        friend class Vector2;
        friend class Vector3;
        Vector4(const VectorData vData);
    };
} // namespace cube

namespace fmt
{
    // Float formatting
    template <>
    struct formatter<cube::Float2>
    {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const cube::Float2& f2, FormatContext& ctx)
        {
            return format_to(ctx.begin(), "({:.3f}, {:.3f})", f2.x, f2.y);
        }
    };

    template <>
    struct formatter<cube::Float3>
    {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const cube::Float3& f3, FormatContext& ctx)
        {
            return format_to(ctx.begin(), "({:.3f}, {:.3f}, {:3.f})", f3.x, f3.y, f3.z);
        }
    };

    template <>
    struct formatter<cube::Float4>
    {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const cube::Float4& f4, FormatContext& ctx)
        {
            return format_to(ctx.begin(), "({:.3f}, {:.3f}, {:.3f}, {:.3f})", f4.x, f4.y, f4.z, f4.w);
        }
    };

    // Vector formatting
    template <>
    struct formatter<cube::Vector2>
    {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const cube::Vector2& v2, FormatContext& ctx)
        {
            cube::Float2 f2 = v2.GetFloat2();
            return format_to(ctx.begin(), "({:.3f}, {:.3f})", f2.x, f2.y);
        }
    };

    template <>
    struct formatter<cube::Vector3>
    {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const cube::Vector3& v3, FormatContext& ctx)
        {
            cube::Float3 f3 = v3.GetFloat3();
            return format_to(ctx.begin(), "({:.3f}, {:.3f}, {:3.f})", f3.x, f3.y, f3.z);
        }
    };

    template <>
    struct formatter<cube::Vector4>
    {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const cube::Vector4& v4, FormatContext& ctx)
        {
            cube::Float4 f4 = v4.GetFloat4();
            return format_to(ctx.begin(), "({:.3f}, {:.3f}, {:.3f}, {:.3f})", f4.x, f4.y, f4.z, f4.w);
        }
    };
} // namespace fmt

// Include inline function definition
#ifndef CUBE_VECTOR_IMPLEMENTATION

#if CUBE_VECTOR_USE_SSE
#include "VectorImpl/VectorSSE.inl"
#else
#include "VectorImpl/VectorArray.inl"
#endif

#endif // !CUBE_VECTOR_IMPLEMENTATION
