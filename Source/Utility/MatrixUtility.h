#pragma once

#include "Matrix.h"

namespace cube
{
    class MatrixUtility
    {
    public:
        static Matrix GetScale(float scaleX, float scaleY, float scaleZ);
        static Matrix GetScale(Vector3 vec);

        static Matrix GetRotationX(float angle);
        static Matrix GetRotationY(float angle);
        static Matrix GetRotationZ(float angle);
        static Matrix GetRotationXYZ(float xAngle, float yAngle, float zAngle);
        static Matrix GetRotationXYZ(Vector3 vec);
        static Matrix GetRotationAxis(Vector3 axis, float angle);

        static Matrix GetTranslation(float x, float y, float z);
        static Matrix GetTranslation(Vector3 vec);

        static Matrix GetLookAt(Vector3 eyePos, Vector3 targetPos, Vector3 upDir);
        static Matrix GetPerspectiveFov(float fovAngleY, float aspectRatio, float nearZ, float farZ);
        static Matrix GetPerspectiveFovWithReverseY(float fovAngleY, float aspectRatio, float nearZ, float farZ);
    };
} // namespace cube

#ifndef MATRIX_UTILITY_IMPLEMENTATION

#if defined(VECTOR_SIMD_SSE)

#include "MatrixImpl/MatrixUtilitySSE.h"

#elif defined(VECTOR_SIMD_NONE)

#include "MatrixImpl/MatrixUtilityArray.h"

#endif

#endif // !MATRIX_UTILITY_IMPLEMENTATION
