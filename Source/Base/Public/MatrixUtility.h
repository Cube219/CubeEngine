#pragma once

#include "Matrix.h"

namespace cube
{
    class MatrixUtility
    {
    public:
        static Matrix GetScale(float scaleX, float scaleY, float scaleZ);
        static Matrix GetScale(const Vector3& vec);

        static Matrix GetRotationX(float angle);
        static Matrix GetRotationY(float angle);
        static Matrix GetRotationZ(float angle);
        static Matrix GetRotationXYZ(float xAngle, float yAngle, float zAngle);
        static Matrix GetRotationXYZ(const Vector3& vec);
        static Matrix GetRotationAxis(const Vector3& axis, float angle);

        static Matrix GetTranslation(float x, float y, float z);
        static Matrix GetTranslation(const Vector3& vec);

        static Matrix GetLookAt(const Vector3& eyePos, const Vector3& targetPos, const Vector3& upDir);
        static Matrix GetPerspectiveFov(float fovAngleY, float aspectRatio, float nearZ, float farZ);
        static Matrix GetPerspectiveFovWithReverseY(float fovAngleY, float aspectRatio, float nearZ, float farZ);
    };
} // namespace cube

#ifndef CUBE_MATRIX_UTILITY_IMPLEMENTATION

#if CUBE_VECTOR_USE_SSE
#include "MatrixImpl/MatrixUtilitySSE.inl"
#else
#include "MatrixImpl/MatrixUtilityArray.inl"
#endif

#endif // !CUBE_MATRIX_UTILITY_IMPLEMENTATION
