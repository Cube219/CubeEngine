#pragma once

#include "fmt/format.h"

#include "CubeString.h"
#include "Format.h"

#ifndef CUBE_VECTOR_USE_SSE
#define CUBE_VECTOR_USE_SSE 1
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

        // void Swap(VectorBase& other); // TODO

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
#else
#include "VectorImpl/VectorArray.inl"
#endif

#endif // !CUBE_VECTOR_IMPLEMENTATION
