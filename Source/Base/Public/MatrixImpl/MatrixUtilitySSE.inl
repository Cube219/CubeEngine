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
        Matrix m;
        m[0] = Vector4(scaleX, 0.0f, 0.0f, 0.0f);
        m[1] = Vector4(0.0f, scaleY, 0.0f, 0.0f);
        m[2] = Vector4(0.0f, 0.0f, scaleZ, 0.0f);
        m[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        return m;
    }

    inline Matrix MatrixUtility::GetScale(Vector3 vec)
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
        Matrix m = Matrix::Identity();

        float sinA = Math::Sin(angle);
        float cosA = Math::Cos(angle);

        m[1] = Vector4(0.0f, cosA, sinA, 0);
        m[2] = Vector4(0.0f, -sinA, cosA, 0);

        return m;
    }

    inline Matrix MatrixUtility::GetRotationY(float angle)
    {
        /*
          cosA     0 -sinA     0
             0     1     0     0
          sinA     0  cosA     0
             0     0     0     1
        */
        Matrix m = Matrix::Identity();

        float sinA = Math::Sin(angle);
        float cosA = Math::Cos(angle);

        m[0] = Vector4(cosA, 0, -sinA, 0);
        m[2] = Vector4(sinA, 0, cosA, 0);

        return m;
    }

    inline Matrix MatrixUtility::GetRotationZ(float angle)
    {
        /*
          cosA  sinA     0     0
         -sinA  cosA     0     0
             0     0     1     0
             0     0     0     1
        */
        Matrix m = Matrix::Identity();

        float sinA = Math::Sin(angle);
        float cosA = Math::Cos(angle);

        m[0] = Vector4(cosA, sinA, 0, 0);
        m[1] = Vector4(-sinA, cosA, 0, 0);

        return m;
    }

    inline Matrix MatrixUtility::GetRotationXYZ(float xAngle, float yAngle, float zAngle)
    {
        /*
                        cosYcosZ                -cosYsinZ        sinY      0
           sinXsinYcosZ+cosXsinZ   -sinXsinYsinZ+cosXcosZ   -sinXcosY      0
          -cosXsinYcosZ+sinXsinZ    cosXsinYsinZ+sinXcosZ    cosXcosY      0
                               0                        0           0      1
        */
        Matrix x = GetRotationX(xAngle);
        Matrix y = GetRotationY(yAngle);
        Matrix z = GetRotationZ(zAngle);

        return x * y * z;
    }

    inline Matrix MatrixUtility::GetRotationXYZ(Vector3 vec)
    {
        Float3 vecF3 = vec.GetFloat3();

        Matrix x = GetRotationX(vecF3.x);
        Matrix y = GetRotationY(vecF3.y);
        Matrix z = GetRotationZ(vecF3.z);

        return x * y * z;
    }

    inline Matrix MatrixUtility::GetRotationAxis(Vector3 axis, float angle)
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

        VectorData r0 = _mm_mul_ps(axis.mData, revCosA.mData);
        VectorData r1 = r0;
        VectorData r2 = r0;

        // x / x / x / x
        VectorData t = _mm_shuffle_ps(axis.mData, axis.mData, _MM_SHUFFLE(0, 0, 0, 0));
        r0 = _mm_mul_ps(r0, t);

        // y / y / y / y
        t = _mm_shuffle_ps(axis.mData, axis.mData, _MM_SHUFFLE(1, 1, 1, 1));
        r1 = _mm_mul_ps(r1, t);

        // z / z / z / z
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
          0  0  0  0
          0  0  0  0
          0  0  0  0
          x  y  z  1
        */
        Matrix m = Matrix::Zero();

        m[3] = Vector4(x, y, z, 1.0f);

        return m;
    }

    inline Matrix MatrixUtility::GetTranslation(Vector3 vec)
    {
        Matrix m = Matrix::Identity();

        Vector4 one = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        // z / z / 1 / 1
        VectorData temp = _mm_shuffle_ps(vec.mData, one.mData, _MM_SHUFFLE(0, 0, 2, 2));
        // x / y / z / 1
        m[3].mData = _mm_shuffle_ps(vec.mData, temp, _MM_SHUFFLE(2, 0, 1, 0));

        return m;
    }

    inline Matrix MatrixUtility::GetLookAt(Vector3 eyePos, Vector3 targetPos, Vector3 upDir)
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

        m[0] = u;
        m[1] = v;
        m[2] = w;
        m[3] = Vector4(0, 0, 0, 1);
        m.Transpose();

        VectorData d0 = (-Vector3::Dot(eyePos, u)).mData;
        VectorData d1 = (-Vector3::Dot(eyePos, v)).mData;
        VectorData d2 = (-Vector3::Dot(eyePos, w)).mData;

        // d1 / d1 / d2 / d2
        VectorData t = _mm_shuffle_ps(d1, d2, _MM_SHUFFLE(0, 0, 0, 0));
        // 0 / d1 / d2 / 0
        t = _mm_and_ps(t, vectorMaskYZ);

        // d0 / 0 / 0 / 0
        d0 = _mm_and_ps(d0, vectorMaskX);
        // d0 / d1 / d2 / 0
        t = _mm_add_ps(t, d0);

        m[3] = Vector4(0, 0, 0, 1);
        m[3].mData = _mm_add_ps(m[3].mData, t);

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

        Matrix m;

        m[0] = Vector4(1 / (aspectRatio * tanA), 0, 0, 0);
        m[1] = Vector4(0, 1 / tanA, 0, 0);
        m[2] = Vector4(0, 0, t, 1);
        m[3] = Vector4(0, 0, -nearZ * t, 0);

        return m;
    }

    inline Matrix MatrixUtility::GetPerspectiveFovWithReverseY(float fovAngleY, float aspectRatio, float nearZ, float farZ)
    {
        float tanA = Math::Tan(fovAngleY / 2.0f);
        float t = farZ / (farZ - nearZ);

        Matrix m;

        m[0] = Vector4(1 / (aspectRatio * tanA), 0, 0, 0);
        m[1] = Vector4(0, -1 / tanA, 0, 0);
        m[2] = Vector4(0, 0, t, 1);
        m[3] = Vector4(0, 0, -nearZ * t, 0);

        return m;
    }
} // namespace cube
