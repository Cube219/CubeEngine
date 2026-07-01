#pragma once

#include "Matrix.h"
#include "Vector.h"

#include "Format.h"

namespace cube
{
    // Imaginary: xyz / Scalar: w
    class Quaternion
    {
    public:
        static Quaternion Zero();
        static Quaternion Identity();

        static Quaternion FromAxisAngle(const Vector3& axis, float angle);
        static Quaternion FromEulerXYZ(float xAngle, float yAngle, float zAngle);
        static Quaternion FromEulerXYZ(const Vector3& angles);
        static Quaternion FromRotationMatrix(const Matrix& matrix);

        Quaternion() = default;
        ~Quaternion() = default;

        Quaternion(float x, float y, float z, float w);
        Quaternion(const Quaternion& other);
        Quaternion& operator=(const Quaternion& rhs);

        bool operator==(const Quaternion& rhs) const;
        bool operator!=(const Quaternion& rhs) const;

        Quaternion operator+(const Quaternion& rhs) const;
        Quaternion operator-(const Quaternion& rhs) const;
        Quaternion operator*(float rhs) const;
        Quaternion operator*(const Quaternion& rhs) const;

        const Quaternion& operator+() const;
        Quaternion operator-() const;

        Quaternion& operator+=(const Quaternion& rhs);
        Quaternion& operator-=(const Quaternion& rhs);
        Quaternion& operator*=(float rhs);
        Quaternion& operator*=(const Quaternion& rhs);

        Float4 GetFloat4() const;

        float Length() const;
        float SquareLength() const;

        void Normalize();
        Quaternion Normalized() const;

        float Dot(const Quaternion& rhs) const;
        static float Dot(const Quaternion& lhs, const Quaternion& rhs);

        void Conjugate();
        Quaternion Conjugated() const;

        void Inverse();
        Quaternion Inversed() const;

        Vector3 RotateVector(const Vector3& vec) const;

        Matrix ToRotationMatrix() const;

        static Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t);
        static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);

    private:
        VectorData<4> mData;

        friend Quaternion operator*(float lhs, const Quaternion& rhs);
    };
} // namespace cube

namespace fmt
{
    using namespace cube;

    template <typename Char>
    struct formatter<Quaternion, Char> : cube_formatter<Char>
    {
        template <typename FormatContext>
        auto format(const Quaternion& q, FormatContext& ctx) const
        {
            Float4 f4 = q.GetFloat4();
            return cube_formatter<Char>::cube_format(ctx, CUBE_T("({:.3f}, {:.3f}, {:.3f}, {:.3f})"), f4.x, f4.y, f4.z, f4.w);
        }
    };
} // namespace fmt

// Include inline function definition
#ifndef CUBE_QUATERNION_IMPLEMENTATION

#if CUBE_VECTOR_USE_AVX2
#include "QuaternionImpl/QuaternionAVX2.inl"
#elif CUBE_VECTOR_USE_SSE
#include "QuaternionImpl/QuaternionSSE.inl"
#elif CUBE_VECTOR_USE_NEON
#include "QuaternionImpl/QuaternionNEON.inl"
#else
#include "QuaternionImpl/QuaternionArray.inl"
#endif

#endif // !CUBE_QUATERNION_IMPLEMENTATION
