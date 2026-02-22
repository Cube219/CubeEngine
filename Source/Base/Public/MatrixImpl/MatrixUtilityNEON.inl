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
            float32x4_t v;
        };

        inline operator float32x4_t() const { return v; }
    };

    static constexpr VectorU32 vectorMaskX = { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 };
    static constexpr VectorU32 vectorMaskY = { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 };
    static constexpr VectorU32 vectorMaskZ = { 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 };
    static constexpr VectorU32 vectorMaskW = { 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF };
    static constexpr VectorU32 vectorMaskYZ = { 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };

    namespace NEONUtility
    {
        inline float32x4_t Mask(float32x4_t a, float32x4_t b)
        {
            return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_f32(b)));
        }
    } // namespace NEONUtility

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

        m[0].mData = NEONUtility::Mask(vec.mData, vectorMaskX);
        m[1].mData = NEONUtility::Mask(vec.mData, vectorMaskY);
        m[2].mData = NEONUtility::Mask(vec.mData, vectorMaskZ);
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

        float cosA = Math::Cos(angle);
        Vector4 revCosA(1 - cosA, 1 - cosA, 1 - cosA, 0.0f);

        // ((1-cosA)x, (1-cosA)y, (1-cosA)z, 0)
        float32x4_t revCosAxis = vmulq_f32(axis.mData, revCosA.mData);

        // Calculate the other part
        float sinA = Math::Sin(angle);
        Float3 f3 = axis.GetFloat3();

        m[0] = Vector4(cosA, f3.z * sinA, -f3.y * sinA, 0.0f);
        m[1] = Vector4(-f3.z * sinA, cosA, f3.x * sinA, 0.0f);
        m[2] = Vector4(f3.y * sinA, -f3.x * sinA, cosA, 0.0f);
        m[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        m[0].mData = vfmaq_laneq_f32(m[0].mData, revCosAxis, revCosAxis, 0);
        m[1].mData = vfmaq_laneq_f32(m[1].mData, revCosAxis, revCosAxis, 1);
        m[2].mData = vfmaq_laneq_f32(m[2].mData, revCosAxis, revCosAxis, 2);

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
        return Matrix{
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            x, y, z, 1.0f
        };
    }

    inline Matrix MatrixUtility::GetTranslation(const Vector3& vec)
    {
        Matrix m = Matrix::Zero();

        // Build {x, y, z, 1} from vec
        m[3].mData = vsetq_lane_f32(1.0f, vec.mData, 3);

        return m;
    }

    inline Matrix MatrixUtility::GetLookAt(const Vector3& eyePos, const Vector3& targetPos, const Vector3& upDir)
    {
        /*
               Ux       Vx       Wx        0
               Uy       Vy       Wy        0
               Uz       Vz       Wz        0
          -ePos*U  -ePos*V  -ePos*W        1
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

        float32x4_t d0 = Vector3::DotV(eyePos, u).mData;
        float32x4_t d1 = Vector3::DotV(eyePos, v).mData;
        float32x4_t d2 = Vector3::DotV(eyePos, w).mData;

        float dotU = vgetq_lane_f32(d0, 0);
        float dotV = vgetq_lane_f32(d1, 0);
        float dotW = vgetq_lane_f32(d2, 0);

        m[3] = Vector4(-dotU, -dotV, -dotW, 1.0f);

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
