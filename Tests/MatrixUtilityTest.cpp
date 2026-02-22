#include <gtest/gtest.h>

#include "CubeMath.h"
#include "Matrix.h"
#include "MatrixUtility.h"

using namespace cube;

constexpr float kEps = 1e-4f;

// Helper to compare two matrices element-wise
static void ExpectMatrixNear(const Matrix& a, const Matrix& b, float eps = kEps)
{
    for (int r = 0; r < 4; ++r)
    {
        Vector4 ra = const_cast<Matrix&>(a)[r];
        Vector4 rb = const_cast<Matrix&>(b)[r];
        Float4 fa = ra.GetFloat4();
        Float4 fb = rb.GetFloat4();
        EXPECT_NEAR(fa.x, fb.x, eps) << "row=" << r << " col=0";
        EXPECT_NEAR(fa.y, fb.y, eps) << "row=" << r << " col=1";
        EXPECT_NEAR(fa.z, fb.z, eps) << "row=" << r << " col=2";
        EXPECT_NEAR(fa.w, fb.w, eps) << "row=" << r << " col=3";
    }
}

static void ExpectMatrixNear(const Matrix& m, const float expected[4][4], float eps = kEps)
{
    for (int r = 0; r < 4; ++r)
    {
        Vector4 row = const_cast<Matrix&>(m)[r];
        Float4 f = row.GetFloat4();
        EXPECT_NEAR(f.x, expected[r][0], eps) << "row=" << r << " col=0";
        EXPECT_NEAR(f.y, expected[r][1], eps) << "row=" << r << " col=1";
        EXPECT_NEAR(f.z, expected[r][2], eps) << "row=" << r << " col=2";
        EXPECT_NEAR(f.w, expected[r][3], eps) << "row=" << r << " col=3";
    }
}

// ===== Scale =====

TEST(MatrixUtilityTest, GetScaleComponents)
{
    Matrix s = MatrixUtility::GetScale(2.0f, 3.0f, 4.0f);
    // Scaling a point (1, 1, 1, 1)
    Vector4 p(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 result = p * s;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 2.0f, kEps);
    EXPECT_NEAR(f.y, 3.0f, kEps);
    EXPECT_NEAR(f.z, 4.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, GetScaleVector)
{
    Vector3 scaleVec(2.0f, 3.0f, 4.0f);
    Matrix s = MatrixUtility::GetScale(scaleVec);
    Vector4 p(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 result = p * s;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 2.0f, kEps);
    EXPECT_NEAR(f.y, 3.0f, kEps);
    EXPECT_NEAR(f.z, 4.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, GetScaleIdentity)
{
    Matrix s = MatrixUtility::GetScale(1.0f, 1.0f, 1.0f);
    ExpectMatrixNear(s, Matrix::Identity());
}

// ===== Rotation =====

TEST(MatrixUtilityTest, RotationX_90Degrees)
{
    float angle = Math::Deg2Rad(90.0f);
    Matrix r = MatrixUtility::GetRotationX(angle);

    // Rotating (0, 1, 0) around X by 90° should give (0, 0, 1)
    Vector4 p(0.0f, 1.0f, 0.0f, 1.0f);
    Vector4 result = p * r;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 1.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, RotationY_90Degrees)
{
    float angle = Math::Deg2Rad(90.0f);
    Matrix r = MatrixUtility::GetRotationY(angle);

    // Rotating (0, 0, 1) around Y by 90° should give (1, 0, 0)
    Vector4 p(0.0f, 0.0f, 1.0f, 1.0f);
    Vector4 result = p * r;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, RotationZ_90Degrees)
{
    float angle = Math::Deg2Rad(90.0f);
    Matrix r = MatrixUtility::GetRotationZ(angle);

    // Rotating (1, 0, 0) around Z by 90° should give (0, 1, 0)
    Vector4 p(1.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = p * r;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 1.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, RotationX_ZeroAngle)
{
    Matrix r = MatrixUtility::GetRotationX(0.0f);
    ExpectMatrixNear(r, Matrix::Identity());
}

TEST(MatrixUtilityTest, RotationY_ZeroAngle)
{
    Matrix r = MatrixUtility::GetRotationY(0.0f);
    ExpectMatrixNear(r, Matrix::Identity());
}

TEST(MatrixUtilityTest, RotationZ_ZeroAngle)
{
    Matrix r = MatrixUtility::GetRotationZ(0.0f);
    ExpectMatrixNear(r, Matrix::Identity());
}

TEST(MatrixUtilityTest, RotationXYZ_IsOrthogonal)
{
    float xAngle = Math::Deg2Rad(30.0f);
    float yAngle = Math::Deg2Rad(45.0f);
    float zAngle = Math::Deg2Rad(60.0f);

    Matrix combined = MatrixUtility::GetRotationXYZ(xAngle, yAngle, zAngle);

    // A combined rotation matrix should be orthogonal: R * R^T = Identity
    Matrix rt = combined.Transposed();
    Matrix product = combined * rt;
    ExpectMatrixNear(product, Matrix::Identity());
}

TEST(MatrixUtilityTest, RotationXYZ_PreservesLength)
{
    float xAngle = Math::Deg2Rad(30.0f);
    float yAngle = Math::Deg2Rad(45.0f);
    float zAngle = Math::Deg2Rad(60.0f);

    Matrix combined = MatrixUtility::GetRotationXYZ(xAngle, yAngle, zAngle);

    // Rotation should preserve vector length
    Vector4 p(1.0f, 2.0f, 3.0f, 0.0f);
    Vector4 result = p * combined;
    EXPECT_NEAR(p.Length(), result.Length(), kEps);
}

TEST(MatrixUtilityTest, RotationXYZ_ZeroAnglesIsIdentity)
{
    Matrix combined = MatrixUtility::GetRotationXYZ(0.0f, 0.0f, 0.0f);
    ExpectMatrixNear(combined, Matrix::Identity());
}

TEST(MatrixUtilityTest, RotationXYZ_VectorOverload)
{
    float xAngle = Math::Deg2Rad(30.0f);
    float yAngle = Math::Deg2Rad(45.0f);
    float zAngle = Math::Deg2Rad(60.0f);

    Vector3 angles(xAngle, yAngle, zAngle);
    Matrix fromVec = MatrixUtility::GetRotationXYZ(angles);
    Matrix fromFloats = MatrixUtility::GetRotationXYZ(xAngle, yAngle, zAngle);

    ExpectMatrixNear(fromVec, fromFloats);
}

TEST(MatrixUtilityTest, RotationAxis_AroundY)
{
    float angle = Math::Deg2Rad(90.0f);
    Vector3 axis(0.0f, 1.0f, 0.0f);
    Matrix r = MatrixUtility::GetRotationAxis(axis, angle);
    Matrix ry = MatrixUtility::GetRotationY(angle);

    ExpectMatrixNear(r, ry);
}

TEST(MatrixUtilityTest, RotationAxis_AroundX)
{
    float angle = Math::Deg2Rad(45.0f);
    Vector3 axis(1.0f, 0.0f, 0.0f);
    Matrix r = MatrixUtility::GetRotationAxis(axis, angle);
    Matrix rx = MatrixUtility::GetRotationX(angle);

    ExpectMatrixNear(r, rx);
}

TEST(MatrixUtilityTest, RotationAxis_AroundZ)
{
    float angle = Math::Deg2Rad(60.0f);
    Vector3 axis(0.0f, 0.0f, 1.0f);
    Matrix r = MatrixUtility::GetRotationAxis(axis, angle);
    Matrix rz = MatrixUtility::GetRotationZ(angle);

    ExpectMatrixNear(r, rz);
}

// ===== Translation =====

TEST(MatrixUtilityTest, GetTranslation)
{
    Matrix t = MatrixUtility::GetTranslation(3.0f, 4.0f, 5.0f);
    // Translation matrix: identity with (x,y,z,1) in row 3
    // (1, 2, 3, 1) * T should give (1+3, 2+4, 3+5, 1) = (4, 6, 8, 1)
    Vector4 p(1.0f, 2.0f, 3.0f, 1.0f);
    Vector4 result = p * t;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
    EXPECT_NEAR(f.z, 8.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, GetTranslationVector)
{
    Vector3 tv(3.0f, 4.0f, 5.0f);
    Matrix t = MatrixUtility::GetTranslation(tv);
    Vector4 p(1.0f, 2.0f, 3.0f, 1.0f);
    Vector4 result = p * t;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
    EXPECT_NEAR(f.z, 8.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, GetTranslation_Add)
{
    // GetTranslation_Add returns a zero matrix with (x,y,z,0) in row 3
    // It is meant to be added to an existing matrix
    Matrix t = MatrixUtility::GetTranslation_Add(3.0f, 4.0f, 5.0f);
    Matrix m = Matrix::Identity();
    Matrix result = m + t;
    // After adding, row 3 should be (3, 4, 5, 1)
    Float4 row3 = result.GetRow(3).GetFloat4();
    EXPECT_NEAR(row3.x, 3.0f, kEps);
    EXPECT_NEAR(row3.y, 4.0f, kEps);
    EXPECT_NEAR(row3.z, 5.0f, kEps);
    EXPECT_NEAR(row3.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, GetTranslation_AddVector)
{
    Vector3 tv(3.0f, 4.0f, 5.0f);
    Matrix t = MatrixUtility::GetTranslation_Add(tv);
    Matrix m = Matrix::Identity();
    Matrix result = m + t;
    Float4 row3 = result.GetRow(3).GetFloat4();
    EXPECT_NEAR(row3.x, 3.0f, kEps);
    EXPECT_NEAR(row3.y, 4.0f, kEps);
    EXPECT_NEAR(row3.z, 5.0f, kEps);
    EXPECT_NEAR(row3.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, TranslationZero)
{
    Matrix t = MatrixUtility::GetTranslation(0.0f, 0.0f, 0.0f);
    ExpectMatrixNear(t, Matrix::Identity());
}

// ===== LookAt =====

TEST(MatrixUtilityTest, LookAt_BasicProperties)
{
    Vector3 eye(0.0f, 0.0f, -5.0f);
    Vector3 target(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Matrix view = MatrixUtility::GetLookAt(eye, target, up);

    // The view matrix should transform eye position to origin
    Vector4 eyeH(0.0f, 0.0f, -5.0f, 1.0f);
    Vector4 transformedEye = eyeH * view;
    Float4 f = transformedEye.GetFloat4();
    // After view transform, eye should be at or near origin
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
}

TEST(MatrixUtilityTest, LookAt_TargetOnZAxis)
{
    Vector3 eye(0.0f, 0.0f, 0.0f);
    Vector3 target(0.0f, 0.0f, 1.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Matrix view = MatrixUtility::GetLookAt(eye, target, up);

    // Origin should map to origin
    Vector4 origin(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = origin * view;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
}

// ===== Perspective =====

TEST(MatrixUtilityTest, PerspectiveFov_BasicProperties)
{
    float fov = Math::Deg2Rad(90.0f);
    float aspect = 16.0f / 9.0f;
    float nearZ = 0.1f;
    float farZ = 100.0f;

    Matrix proj = MatrixUtility::GetPerspectiveFov(fov, aspect, nearZ, farZ);

    // A point at center of near plane should map to (0, 0, ?, ?)
    Vector4 nearCenter(0.0f, 0.0f, nearZ, 1.0f);
    Vector4 result = nearCenter * proj;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
}

TEST(MatrixUtilityTest, PerspectiveFovWithReverseY_BasicProperties)
{
    float fov = Math::Deg2Rad(90.0f);
    float aspect = 16.0f / 9.0f;
    float nearZ = 0.1f;
    float farZ = 100.0f;

    Matrix proj = MatrixUtility::GetPerspectiveFovWithReverseY(fov, aspect, nearZ, farZ);

    // A point at center of near plane
    Vector4 nearCenter(0.0f, 0.0f, nearZ, 1.0f);
    Vector4 result = nearCenter * proj;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
}

TEST(MatrixUtilityTest, PerspectiveFovWithReverseY_YIsFlipped)
{
    float fov = Math::Deg2Rad(90.0f);
    float aspect = 1.0f;
    float nearZ = 0.1f;
    float farZ = 100.0f;

    Matrix proj = MatrixUtility::GetPerspectiveFov(fov, aspect, nearZ, farZ);
    Matrix projRev = MatrixUtility::GetPerspectiveFovWithReverseY(fov, aspect, nearZ, farZ);

    // Row 1 (Y scaling) should be negated in ReverseY
    Float4 r1 = proj.GetRow(1).GetFloat4();
    Float4 r1Rev = projRev.GetRow(1).GetFloat4();
    EXPECT_NEAR(r1.y, -r1Rev.y, kEps);
}

// ===== Combined Transforms =====

TEST(MatrixUtilityTest, ScaleRotateTranslate)
{
    // Build a SRT matrix and verify it transforms correctly
    Matrix s = MatrixUtility::GetScale(2.0f, 2.0f, 2.0f);
    Matrix r = MatrixUtility::GetRotationZ(Math::Deg2Rad(90.0f));
    Matrix t = MatrixUtility::GetTranslation(10.0f, 0.0f, 0.0f);

    Matrix srt = s * r * t;

    // Transform (1, 0, 0, 1):
    // Scale: (2, 0, 0, 1)
    // RotZ 90°: (0, 2, 0, 1)
    // Translate: (10, 2, 0, 1)
    Vector4 p(1.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = p * srt;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 10.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, RotationPreservesLength)
{
    float angle = Math::Deg2Rad(37.0f);
    Matrix r = MatrixUtility::GetRotationXYZ(
        Math::Deg2Rad(30.0f),
        Math::Deg2Rad(45.0f),
        Math::Deg2Rad(60.0f)
    );

    Vector4 p(1.0f, 2.0f, 3.0f, 0.0f);
    Vector4 rotated = p * r;

    EXPECT_NEAR(p.Length(), rotated.Length(), kEps);
}

TEST(MatrixUtilityTest, RotationIsOrthogonal)
{
    Matrix r = MatrixUtility::GetRotationXYZ(
        Math::Deg2Rad(30.0f),
        Math::Deg2Rad(45.0f),
        Math::Deg2Rad(60.0f)
    );

    // R * R^T should be Identity for rotation matrices
    Matrix rt = r.Transposed();
    Matrix product = r * rt;
    ExpectMatrixNear(product, Matrix::Identity(), 1e-4f);
}
