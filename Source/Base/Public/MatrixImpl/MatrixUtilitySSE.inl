#define CUBE_MATRIX_UTILITY_IMPLEMENTATION

#include "../MatrixUtility.h"

#include <stdint.h>

#include "../CubeMath.h"

namespace cube
{
    struct VectorU32
    {
        union
        {
            uint32_t u[4];
            __m128 v;
        };

        inline operator __m128() const { return v; }
    };

    static constexpr VectorU32 vectorMaskX = { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 };
    static constexpr VectorU32 vectorMaskY = { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 };
    static constexpr VectorU32 vectorMaskZ = { 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 };
    static constexpr VectorU32 vectorMaskW = { 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF };
    static constexpr VectorU32 vectorMaskYZ = { 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };

    inline Matrix MatrixUtility::GetScale(float scaleX, float scaleY, float scaleZ)
    {
        /*
          x  0  0  0
          0  y  0  0
          0  0  z  0
          0  0  0  1
        */
        return Matrix{
            scaleX, 0.0f, 0.0f, 0.0f,
            0.0f, scaleY, 0.0f, 0.0f,
            0.0f, 0.0f, scaleZ, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    inline Matrix MatrixUtility::GetScale(const Vector3& vec)
    {
        Matrix m;

        m[0].mData = _mm_and_ps(vec.mData, vectorMaskX);
        m[1].mData = _mm_and_ps(vec.mData, vectorMaskY);
        m[2].mData = _mm_and_ps(vec.mData, vectorMaskZ);
        m[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        return m;
    }

    inline Matrix MatrixUtility::GetRotationX(float angle)
    {
        /*
             1     0     0     0
             0  cosA  sinA     0
             0 -sinA  cosA     0
             0     0     0     1
        */
        float sinA = Math::Sin(angle);
        float cosA = Math::Cos(angle);

        return Matrix{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, cosA, sinA, 0.0f,
            0.0f, -sinA, cosA, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    inline Matrix MatrixUtility::GetRotationY(float angle)
    {
        /*
          cosA     0 -sinA     0
             0     1     0     0
          sinA     0  cosA     0
             0     0     0     1
        */
        float sinA = Math::Sin(angle);
        float cosA = Math::Cos(angle);

        return Matrix{
            cosA, 0.0f, -sinA, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            sinA, 0.0f, cosA, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    inline Matrix MatrixUtility::GetRotationZ(float angle)
    {
        /*
          cosA  sinA     0     0
         -sinA  cosA     0     0
             0     0     1     0
             0     0     0     1
        */
        float sinA = Math::Sin(angle);
        float cosA = Math::Cos(angle);

        return Matrix{
            cosA, sinA, 0.0f, 0.0f,
            -sinA, cosA, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    inline Matrix MatrixUtility::GetRotationXYZ(float xAngle, float yAngle, float zAngle)
    {
        /*
                        cosYcosZ                -cosYsinZ        sinY      0
           sinXsinYcosZ+cosXsinZ   -sinXsinYsinZ+cosXcosZ   -sinXcosY      0
          -cosXsinYcosZ+sinXsinZ    cosXsinYsinZ+sinXcosZ    cosXcosY      0
                               0                        0           0      1
        */
        float sinX = Math::Sin(xAngle);
        float cosX = Math::Cos(xAngle);
        float sinY = Math::Sin(yAngle);
        float cosY = Math::Cos(yAngle);
        float sinZ = Math::Sin(zAngle);
        float cosZ = Math::Cos(zAngle);

        return Matrix{
            cosY * cosZ, -cosY * sinZ, sinY, 0.0f,
            sinX * sinY * cosZ + cosX * sinZ, -sinX * sinY * sinZ + cosX * cosZ, -sinX * cosY, 0.0f,
            -cosX * sinY * cosZ + sinX * sinZ, cosX * sinY * sinZ + sinX * cosZ, cosX * cosY, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    inline Matrix MatrixUtility::GetRotationXYZ(const Vector3& vec)
    {
        Float3 vecF3 = vec.GetFloat3();
        return GetRotationXYZ(vecF3.x, vecF3.y, vecF3.z);
    }

    inline Matrix MatrixUtility::GetRotationAxis(const Vector3& axis, float angle)
    {
        /*
           cosA+(1-cosA)x^2  (1-cosA)xy+z*sinA  (1-cosA)xz-y*sinA     0
          (1-cosA)xy-z*sinA   cosA+(1-cosA)y^2  (1-cosA)yz+x*sinA     0
          (1-cosA)xz+y*sinA  (1-cosA)yz-x*sinA   cosA+(1-cosA)z^2     0
                          0                  0                  0     1
        */
        Matrix m;

        // Calculate the part of (1-cosA)
        float cA = Math::Cos(angle);
        Vector4 revCosA(1 - cA, 1 - cA, 1 - cA, 0.0f);

        __m128 r0 = _mm_mul_ps(axis.mData, revCosA.mData);
        __m128 r1 = r0;
        __m128 r2 = r0;

        // (x, x, x, x)
        __m128 t = _mm_shuffle_ps(axis.mData, axis.mData, _MM_SHUFFLE(0, 0, 0, 0));
        r0 = _mm_mul_ps(r0, t);

        // (y, y, y, y)
        t = _mm_shuffle_ps(axis.mData, axis.mData, _MM_SHUFFLE(1, 1, 1, 1));
        r1 = _mm_mul_ps(r1, t);

        // (z, z, z, z)
        t = _mm_shuffle_ps(axis.mData, axis.mData, _MM_SHUFFLE(2, 2, 2, 2));
        r2 = _mm_mul_ps(r2, t);

        // Calculate the other part
        float sA = Math::Sin(angle);
        Float3 f3 = axis.GetFloat3();

        m[0] = Vector4(cA, f3.z * sA, -f3.y * sA, 0.0f);
        m[1] = Vector4(-f3.z * sA, cA, f3.x * sA, 0.0f);
        m[2] = Vector4(f3.y * sA, -f3.x * sA, cA, 0.0f);
        m[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        m[0].mData = _mm_add_ps(m[0].mData, r0);
        m[1].mData = _mm_add_ps(m[1].mData, r1);
        m[2].mData = _mm_add_ps(m[2].mData, r2);

        return m;
    }

    inline Matrix MatrixUtility::GetTranslation(float x, float y, float z)
    {
        /*
          1  0  0  0
          0  1  0  0
          0  0  1  0
          x  y  z  1
        */
        return Matrix{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            x, y, z, 1.0f
        };
    }

    inline Matrix MatrixUtility::GetTranslation(const Vector3& vec)
    {
        Matrix m = Matrix::Identity();

        Vector4 one = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        // (z, z, 1, 1)
        __m128 temp = _mm_shuffle_ps(vec.mData, one.mData, _MM_SHUFFLE(0, 0, 2, 2));
        // (x, y, z, 1)
        m[3].mData = _mm_shuffle_ps(vec.mData, temp, _MM_SHUFFLE(2, 0, 1, 0));

        return m;
    }

    inline Matrix MatrixUtility::GetTranslation_Add(float x, float y, float z)
    {
        /*
          0  0  0  0
          0  0  0  0
          0  0  0  0
          x  y  z  0
        */
        return Matrix{
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            x, y, z, 0.0f
        };
    }

    inline Matrix MatrixUtility::GetTranslation_Add(const Vector3& vec)
    {
        Matrix m = Matrix::Zero();

        // Convert to Vector4 to set w component 0.
        m[3].mData = Vector4(vec).mData;

        return m;
    }

    inline Matrix MatrixUtility::GetLookAt(const Vector3& eyePos, const Vector3& targetPos, const Vector3& upDir)
    {
        /*
               Ux       Vx       Wx        0
               Uy       Vy       Wy        0
               Uz       Vz       Wz        0
          -ePos*U  -ePos*V  -ePos*w        1
          (U, V, W: basis vectors in eye space)
          (ePos: eye's position)
        */
        Matrix m;

        Vector3 w = targetPos - eyePos;
        w.Normalize();
        Vector3 u = Vector3::Cross(upDir, w);
        u.Normalize();
        Vector3 v = Vector3::Cross(w, u);

        m[0] = Vector4(u);
        m[1] = Vector4(v);
        m[2] = Vector4(w);
        m[3] = Vector4(0, 0, 0, 1);
        m.Transpose();

        __m128 d0 = Vector3::DotV(eyePos, u).mData;
        __m128 d1 = Vector3::DotV(eyePos, v).mData;
        __m128 d2 = Vector3::DotV(eyePos, w).mData;

        // (d1, d1, d2, d2)
        __m128 t = _mm_shuffle_ps(d1, d2, _MM_SHUFFLE(0, 0, 0, 0));
        // (0, d1, d2, 0)
        t = _mm_and_ps(t, vectorMaskYZ);

        // (d0, 0, 0, 0)
        d0 = _mm_and_ps(d0, vectorMaskX);
        // (d0, d1, d2, 0)
        t = _mm_add_ps(t, d0);

        m[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        m[3].mData = _mm_sub_ps(m[3].mData, t);

        return m;
    }

    inline Matrix MatrixUtility::GetPerspectiveFov(float fovAngleY, float aspectRatio, float nearZ, float farZ)
    {
        /*
          1/(rtan(a/2))             0             0             0
                      0  1/(tan(a/2))             0             0
                      0             0       f/(f-n)             1
                      0             0     -nf/(f-n)             0
        */

        float tanA = Math::Tan(fovAngleY / 2.0f);
        float t = farZ / (farZ - nearZ);

        return Matrix{
            Vector4(1.0f / (aspectRatio * tanA), 0.0f, 0.0f, 0.0f),
            Vector4(0.0f, 1.0f / tanA, 0.0f, 0.0f),
            Vector4(0.0f, 0.0f, t, 1.0f),
            Vector4(0.0f, 0.0f, -nearZ * t, 0.0f)
        };
    }

    inline Matrix MatrixUtility::GetPerspectiveFovWithReverseY(float fovAngleY, float aspectRatio, float nearZ, float farZ)
    {
        float tanA = Math::Tan(fovAngleY / 2.0f);
        float t = farZ / (farZ - nearZ);

        return Matrix{
            Vector4(1.0f / (aspectRatio * tanA), 0.0f, 0.0f, 0.0f),
            Vector4(0.0f, -1.0f / tanA, 0.0f, 0.0f),
            Vector4(0.0f, 0.0f, t, 1.0f),
            Vector4(0.0f, 0.0f, -nearZ * t, 0.0f)
        };
    }
} // namespace cube
