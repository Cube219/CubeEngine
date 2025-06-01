#define CUBE_MATRIX_UTILITY_IMPLEMENTATION

#include "../MatrixUtility.h"

#include "../CubeMath.h"

namespace cube
{
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
        return GetScale(vec.mData[0], vec.mData[1], vec.mData[2]);
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
        float sA = Math::Sin(angle);
        float cA = Math::Cos(angle);
        float revCA = 1 - cA;

        float x = axis.mData[0];
        float y = axis.mData[0];
        float z = axis.mData[0];

        return Matrix{
            cA + revCA * x * x, revCA * x * y + z * sA, revCA * x * z - y * sA, 0.0f,
            revCA * x * y - z * sA, cA + revCA * y * y, revCA * y * z + x * sA, 0.0f,
            revCA * x * z + y * sA, revCA * y * z - x * sA, cA + revCA * z * z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    inline Matrix MatrixUtility::GetTranslation(float x, float y, float z)
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

    inline Matrix MatrixUtility::GetTranslation(const Vector3& vec)
    {
        return GetTranslation(vec.mData[0], vec.mData[1], vec.mData[2]);
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
        Vector3 w = targetPos - eyePos;
        w.Normalize();
        Vector3 u = Vector3::Cross(upDir, w);
        u.Normalize();
        Vector3 v = Vector3::Cross(w, u);

        Matrix m(
            Vector4(u),
            Vector4(v),
            Vector4(w),
            Vector4(0.0f, 0.0f, 0.0f, 1.0f)
        );
        m.Transpose();

        float d0 = -Vector3::Dot(eyePos, u);
        float d1 = -Vector3::Dot(eyePos, v);
        float d2 = -Vector3::Dot(eyePos, w);
        m[3] = Vector4(d0, d1, d2, 1.0f);

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
            1.0f / (aspectRatio * tanA), 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / tanA, 0.0f, 0.0f,
            0.0f, 0.0f, t, 1.0f,
            0.0f, 0.0f, -nearZ * t, 0.0f
        };
    }

    inline Matrix MatrixUtility::GetPerspectiveFovWithReverseY(float fovAngleY, float aspectRatio, float nearZ, float farZ)
    {
        float tanA = Math::Tan(fovAngleY / 2.0f);
        float t = farZ / (farZ - nearZ);

        return Matrix{
            1.0f / (aspectRatio * tanA), 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f / tanA, 0.0f, 0.0f,
            0.0f, 0.0f, t, 1.0f,
            0.0f, 0.0f, -nearZ * t, 0.0f
        };
    }
} // namespace cube
