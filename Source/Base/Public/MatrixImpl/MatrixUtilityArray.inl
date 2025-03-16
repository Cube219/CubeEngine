#define CUBE_MATRIX_UTILITY_IMPLEMENTATION

#include "../MatrixUtility.h"

#include "../Math.h"

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

        m[0] = Vector4(vec.mData[0], 0.0f, 0.0f, 0.0f);
        m[1] = Vector4(0.0f, vec.mData[1], 0.0f, 0.0f);
        m[2] = Vector4(0.0f, 0.0f, vec.mData[2], 0.0f);
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

        float sA = Math::Sin(angle);
        float cA = Math::Cos(angle);
        float revCA = 1 - cA;

        float x = axis.mData[0];
        float y = axis.mData[0];
        float z = axis.mData[0];

        m[0] = Vector4(cA + revCA * x * x, revCA * x * y + z * sA, revCA * x * z - y * sA, 0);
        m[1] = Vector4(revCA * x * y - z * sA, cA + revCA * y * y, revCA * y * z + x * sA, 0);
        m[2] = Vector4(revCA * x * z + y * sA, revCA * y * z - x * sA, cA + revCA * z * z, 0);
        m[3] = Vector4(0, 0, 0, 1);

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

        m[3] = Vector4(vec.mData[0], vec.mData[1], vec.mData[2], 1.0f);

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

        VectorBase d0 = -Vector3::Dot(eyePos, u);
        VectorBase d1 = -Vector3::Dot(eyePos, v);
        VectorBase d2 = -Vector3::Dot(eyePos, w);

        m[3] = Vector4(d0.mData[0], d1.mData[0], d2.mData[0], 1);

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
