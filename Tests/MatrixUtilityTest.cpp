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

    // Non-uniform scale with negative and fractional
    Matrix s2 = MatrixUtility::GetScale(-1.0f, 0.5f, 10.0f);
    Vector4 p2(3.0f, 4.0f, 5.0f, 1.0f);
    Float4 f2 = (p2 * s2).GetFloat4();
    EXPECT_NEAR(f2.x, -3.0f, kEps);
    EXPECT_NEAR(f2.y, 2.0f, kEps);
    EXPECT_NEAR(f2.z, 50.0f, kEps);
    EXPECT_NEAR(f2.w, 1.0f, kEps);

    // Very small scale
    Matrix s3 = MatrixUtility::GetScale(0.01f, 0.01f, 0.01f);
    Vector4 p3(100.0f, 200.0f, 300.0f, 1.0f);
    Float4 f3 = (p3 * s3).GetFloat4();
    EXPECT_NEAR(f3.x, 1.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
    EXPECT_NEAR(f3.z, 3.0f, kEps);
    EXPECT_NEAR(f3.w, 1.0f, kEps);
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

    Vector3 scaleVec2(-2.0f, 0.5f, 3.0f);
    Matrix s2 = MatrixUtility::GetScale(scaleVec2);
    Vector4 p2(5.0f, 8.0f, 2.0f, 1.0f);
    Float4 f2 = (p2 * s2).GetFloat4();
    EXPECT_NEAR(f2.x, -10.0f, kEps);
    EXPECT_NEAR(f2.y, 4.0f, kEps);
    EXPECT_NEAR(f2.z, 6.0f, kEps);
    EXPECT_NEAR(f2.w, 1.0f, kEps);
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

    // Rotating (0, 1, 0) around X by 90 should give (0, 0, 1)
    Vector4 p(0.0f, 1.0f, 0.0f, 1.0f);
    Vector4 result = p * r;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 1.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);

    // Rotating (0, 0, 1) around X by 90 should give (0, -1, 0)
    Vector4 p2(0.0f, 0.0f, 1.0f, 1.0f);
    Float4 f2 = (p2 * r).GetFloat4();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, -1.0f, kEps);
    EXPECT_NEAR(f2.z, 0.0f, kEps);
    EXPECT_NEAR(f2.w, 1.0f, kEps);

    // 180 degrees: (0, 1, 0) -> (0, -1, 0)
    Matrix r180 = MatrixUtility::GetRotationX(Math::Deg2Rad(180.0f));
    Float4 f3 = (p * r180).GetFloat4();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, -1.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
    EXPECT_NEAR(f3.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, RotationY_90Degrees)
{
    float angle = Math::Deg2Rad(90.0f);
    Matrix r = MatrixUtility::GetRotationY(angle);

    // Rotating (0, 0, 1) around Y by 90 should give (1, 0, 0)
    Vector4 p(0.0f, 0.0f, 1.0f, 1.0f);
    Vector4 result = p * r;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);

    // Rotating (1, 0, 0) around Y by 90 should give (0, 0, -1)
    Vector4 p2(1.0f, 0.0f, 0.0f, 1.0f);
    Float4 f2 = (p2 * r).GetFloat4();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, -1.0f, kEps);
    EXPECT_NEAR(f2.w, 1.0f, kEps);

    // 180 degrees: (1, 0, 0) -> (-1, 0, 0)
    Matrix r180 = MatrixUtility::GetRotationY(Math::Deg2Rad(180.0f));
    Float4 f3 = (p2 * r180).GetFloat4();
    EXPECT_NEAR(f3.x, -1.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
    EXPECT_NEAR(f3.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, RotationZ_90Degrees)
{
    float angle = Math::Deg2Rad(90.0f);
    Matrix r = MatrixUtility::GetRotationZ(angle);

    // Rotating (1, 0, 0) around Z by 90 should give (0, 1, 0)
    Vector4 p(1.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = p * r;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 1.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);

    // Rotating (0, 1, 0) around Z by 90 should give (-1, 0, 0)
    Vector4 p2(0.0f, 1.0f, 0.0f, 1.0f);
    Float4 f2 = (p2 * r).GetFloat4();
    EXPECT_NEAR(f2.x, -1.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 0.0f, kEps);
    EXPECT_NEAR(f2.w, 1.0f, kEps);

    // 270 degrees: (1, 0, 0) -> (0, -1, 0)
    Matrix r270 = MatrixUtility::GetRotationZ(Math::Deg2Rad(270.0f));
    Float4 f3 = (p * r270).GetFloat4();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, -1.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
    EXPECT_NEAR(f3.w, 1.0f, kEps);
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

    // Another set of angles
    Matrix combined2 = MatrixUtility::GetRotationXYZ(
        Math::Deg2Rad(73.0f), Math::Deg2Rad(-41.0f), Math::Deg2Rad(158.0f));
    Matrix product2 = combined2 * combined2.Transposed();
    ExpectMatrixNear(product2, Matrix::Identity());
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

    // Different vector and angles
    Matrix combined2 = MatrixUtility::GetRotationXYZ(
        Math::Deg2Rad(73.0f), Math::Deg2Rad(-41.0f), Math::Deg2Rad(158.0f));
    Vector4 p2(-5.0f, 7.0f, -2.0f, 0.0f);
    Vector4 result2 = p2 * combined2;
    EXPECT_NEAR(p2.Length(), result2.Length(), kEps);

    Vector4 p3(100.0f, 0.0f, 0.0f, 0.0f);
    Vector4 result3 = p3 * combined;
    EXPECT_NEAR(p3.Length(), result3.Length(), kEps);
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

    // Another set of angles
    float xAngle2 = Math::Deg2Rad(-15.0f);
    float yAngle2 = Math::Deg2Rad(73.0f);
    float zAngle2 = Math::Deg2Rad(-120.0f);
    Vector3 angles2(xAngle2, yAngle2, zAngle2);
    ExpectMatrixNear(
        MatrixUtility::GetRotationXYZ(angles2),
        MatrixUtility::GetRotationXYZ(xAngle2, yAngle2, zAngle2));
}

TEST(MatrixUtilityTest, RotationAxis_AroundY)
{
    float angle = Math::Deg2Rad(90.0f);
    Vector3 axis(0.0f, 1.0f, 0.0f);
    Matrix r = MatrixUtility::GetRotationAxis(axis, angle);
    Matrix ry = MatrixUtility::GetRotationY(angle);

    ExpectMatrixNear(r, ry);

    // Also test with 45 degrees
    float angle2 = Math::Deg2Rad(45.0f);
    Matrix r2 = MatrixUtility::GetRotationAxis(axis, angle2);
    Matrix ry2 = MatrixUtility::GetRotationY(angle2);
    ExpectMatrixNear(r2, ry2);
}

TEST(MatrixUtilityTest, RotationAxis_AroundX)
{
    float angle = Math::Deg2Rad(45.0f);
    Vector3 axis(1.0f, 0.0f, 0.0f);
    Matrix r = MatrixUtility::GetRotationAxis(axis, angle);
    Matrix rx = MatrixUtility::GetRotationX(angle);

    ExpectMatrixNear(r, rx);

    // Also test with 120 degrees
    float angle2 = Math::Deg2Rad(120.0f);
    Matrix r2 = MatrixUtility::GetRotationAxis(axis, angle2);
    Matrix rx2 = MatrixUtility::GetRotationX(angle2);
    ExpectMatrixNear(r2, rx2);
}

TEST(MatrixUtilityTest, RotationAxis_AroundZ)
{
    float angle = Math::Deg2Rad(60.0f);
    Vector3 axis(0.0f, 0.0f, 1.0f);
    Matrix r = MatrixUtility::GetRotationAxis(axis, angle);
    Matrix rz = MatrixUtility::GetRotationZ(angle);

    ExpectMatrixNear(r, rz);

    // Also test with 180 degrees
    float angle2 = Math::Deg2Rad(180.0f);
    Matrix r2 = MatrixUtility::GetRotationAxis(axis, angle2);
    Matrix rz2 = MatrixUtility::GetRotationZ(angle2);
    ExpectMatrixNear(r2, rz2);
}

// ===== Translation =====

TEST(MatrixUtilityTest, GetTranslation)
{
    Matrix t = MatrixUtility::GetTranslation(3.0f, 4.0f, 5.0f);
    // (1, 2, 3, 1) * T should give (1+3, 2+4, 3+5, 1) = (4, 6, 8, 1)
    Vector4 p(1.0f, 2.0f, 3.0f, 1.0f);
    Vector4 result = p * t;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
    EXPECT_NEAR(f.z, 8.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);

    // Negative translation
    Matrix t2 = MatrixUtility::GetTranslation(-10.0f, 5.0f, -3.0f);
    Vector4 p2(0.0f, 0.0f, 0.0f, 1.0f);
    Float4 f2 = (p2 * t2).GetFloat4();
    EXPECT_NEAR(f2.x, -10.0f, kEps);
    EXPECT_NEAR(f2.y, 5.0f, kEps);
    EXPECT_NEAR(f2.z, -3.0f, kEps);
    EXPECT_NEAR(f2.w, 1.0f, kEps);

    // Direction vector (w=0) should be unaffected by translation
    Vector4 dir(1.0f, 0.0f, 0.0f, 0.0f);
    Float4 f3 = (dir * t).GetFloat4();
    EXPECT_NEAR(f3.x, 1.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
    EXPECT_NEAR(f3.w, 0.0f, kEps);
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

    Vector3 tv2(-100.0f, 200.0f, -300.0f);
    Matrix t2 = MatrixUtility::GetTranslation(tv2);
    Vector4 p2(50.0f, -50.0f, 150.0f, 1.0f);
    Float4 f2 = (p2 * t2).GetFloat4();
    EXPECT_NEAR(f2.x, -50.0f, kEps);
    EXPECT_NEAR(f2.y, 150.0f, kEps);
    EXPECT_NEAR(f2.z, -150.0f, kEps);
    EXPECT_NEAR(f2.w, 1.0f, kEps);
}

TEST(MatrixUtilityTest, GetTranslation_Add)
{
    Matrix t = MatrixUtility::GetTranslation_Add(3.0f, 4.0f, 5.0f);
    Matrix m = Matrix::Identity();
    Matrix result = m + t;
    Float4 row3 = result.GetRow(3).GetFloat4();
    EXPECT_NEAR(row3.x, 3.0f, kEps);
    EXPECT_NEAR(row3.y, 4.0f, kEps);
    EXPECT_NEAR(row3.z, 5.0f, kEps);
    EXPECT_NEAR(row3.w, 1.0f, kEps);

    Matrix t2 = MatrixUtility::GetTranslation_Add(-10.0f, 20.0f, -30.0f);
    Matrix result2 = m + t2;
    Float4 row3b = result2.GetRow(3).GetFloat4();
    EXPECT_NEAR(row3b.x, -10.0f, kEps);
    EXPECT_NEAR(row3b.y, 20.0f, kEps);
    EXPECT_NEAR(row3b.z, -30.0f, kEps);
    EXPECT_NEAR(row3b.w, 1.0f, kEps);
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

    Vector3 tv2(-7.0f, 0.0f, 42.0f);
    Matrix t2 = MatrixUtility::GetTranslation_Add(tv2);
    Matrix result2 = m + t2;
    Float4 row3b = result2.GetRow(3).GetFloat4();
    EXPECT_NEAR(row3b.x, -7.0f, kEps);
    EXPECT_NEAR(row3b.y, 0.0f, kEps);
    EXPECT_NEAR(row3b.z, 42.0f, kEps);
    EXPECT_NEAR(row3b.w, 1.0f, kEps);
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
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);

    // Different eye position
    Vector3 eye2(10.0f, 5.0f, 0.0f);
    Vector3 target2(0.0f, 0.0f, 0.0f);
    Matrix view2 = MatrixUtility::GetLookAt(eye2, target2, up);
    Vector4 eyeH2(10.0f, 5.0f, 0.0f, 1.0f);
    Float4 f2 = (eyeH2 * view2).GetFloat4();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 0.0f, kEps);
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

    // Target on negative Z axis
    Vector3 target2(0.0f, 0.0f, -1.0f);
    Matrix view2 = MatrixUtility::GetLookAt(eye, target2, up);
    Float4 f2 = (origin * view2).GetFloat4();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 0.0f, kEps);
}

TEST(MatrixUtilityTest, LookAt_TargetHasNegativeZ)
{
    // In right-handed, target should be at negative Z in view space
    Vector3 eye(0.0f, 0.0f, -5.0f);
    Vector3 target(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Matrix view = MatrixUtility::GetLookAt(eye, target, up);
    Vector4 targetH(0.0f, 0.0f, 0.0f, 1.0f);
    Float4 f = (targetH * view).GetFloat4();
    EXPECT_LT(f.z, 0.0f);
}

// ===== Perspective =====

TEST(MatrixUtilityTest, PerspectiveFov_BasicProperties)
{
    float fov = Math::Deg2Rad(90.0f);
    float aspect = 16.0f / 9.0f;
    float nearZ = 0.1f;
    float farZ = 100.0f;

    Matrix proj = MatrixUtility::GetPerspectiveFov(fov, aspect, nearZ, farZ);

    // A point at center of near plane (right-handed: -Z is forward)
    Vector4 nearCenter(0.0f, 0.0f, -nearZ, 1.0f);
    Vector4 result = nearCenter * proj;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    // ndc_z should be 0 at near plane
    float ndcZ = f.z / f.w;
    EXPECT_NEAR(ndcZ, 0.0f, kEps);

    // ndc_z should be 1 at far plane
    Vector4 farCenter(0.0f, 0.0f, -farZ, 1.0f);
    Float4 f2 = (farCenter * proj).GetFloat4();
    float ndcZFar = f2.z / f2.w;
    EXPECT_NEAR(ndcZFar, 1.0f, kEps);
}

TEST(MatrixUtilityTest, PerspectiveFovWithReverseY_BasicProperties)
{
    float fov = Math::Deg2Rad(90.0f);
    float aspect = 16.0f / 9.0f;
    float nearZ = 0.1f;
    float farZ = 100.0f;

    Matrix proj = MatrixUtility::GetPerspectiveFovWithReverseY(fov, aspect, nearZ, farZ);

    // A point at center of near plane (right-handed: -Z is forward)
    Vector4 nearCenter(0.0f, 0.0f, -nearZ, 1.0f);
    Vector4 result = nearCenter * proj;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);

    // Different parameters
    Matrix proj2 = MatrixUtility::GetPerspectiveFovWithReverseY(
        Math::Deg2Rad(60.0f), 1.0f, 0.5f, 500.0f);
    Vector4 nearCenter2(0.0f, 0.0f, -0.5f, 1.0f);
    Float4 f2 = (nearCenter2 * proj2).GetFloat4();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
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
    // RotZ 90: (0, 2, 0, 1)
    // Translate: (10, 2, 0, 1)
    Vector4 p(1.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = p * srt;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 10.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);

    // Different SRT: Scale(3,1,1) * RotX(90) * Translate(0,5,0)
    Matrix s2 = MatrixUtility::GetScale(3.0f, 1.0f, 1.0f);
    Matrix r2 = MatrixUtility::GetRotationX(Math::Deg2Rad(90.0f));
    Matrix t2 = MatrixUtility::GetTranslation(0.0f, 5.0f, 0.0f);
    Matrix srt2 = s2 * r2 * t2;

    // Transform (1, 0, 0, 1):
    // Scale: (3, 0, 0, 1)
    // RotX 90: (3, 0, 0, 1) (X axis unaffected by RotX)
    // Translate: (3, 5, 0, 1)
    Vector4 p2(1.0f, 0.0f, 0.0f, 1.0f);
    Float4 f2 = (p2 * srt2).GetFloat4();
    EXPECT_NEAR(f2.x, 3.0f, kEps);
    EXPECT_NEAR(f2.y, 5.0f, kEps);
    EXPECT_NEAR(f2.z, 0.0f, kEps);
    EXPECT_NEAR(f2.w, 1.0f, kEps);
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

    // Different vector
    Vector4 p2(-5.0f, 7.0f, -2.0f, 0.0f);
    Vector4 rotated2 = p2 * r;
    EXPECT_NEAR(p2.Length(), rotated2.Length(), kEps);

    // Unit vector should remain unit
    Vector4 p3(1.0f, 0.0f, 0.0f, 0.0f);
    Vector4 rotated3 = p3 * r;
    EXPECT_NEAR(rotated3.Length(), 1.0f, kEps);
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

    Matrix r2 = MatrixUtility::GetRotationXYZ(
        Math::Deg2Rad(-73.0f),
        Math::Deg2Rad(158.0f),
        Math::Deg2Rad(-41.0f)
    );
    Matrix product2 = r2 * r2.Transposed();
    ExpectMatrixNear(product2, Matrix::Identity(), 1e-4f);
}

// ===== Complex Tests =====

TEST(MatrixUtilityTest, RotationXYZ_ThreeRotationsUndo)
{
    // Applying an XYZ rotation then its inverse should restore the original vector.
    float xAngle = Math::Deg2Rad(25.0f);
    float yAngle = Math::Deg2Rad(53.0f);
    float zAngle = Math::Deg2Rad(78.0f);

    Matrix r = MatrixUtility::GetRotationXYZ(xAngle, yAngle, zAngle);
    Matrix rInv = r.Inversed();

    Vector4 p(3.0f, -7.0f, 2.0f, 0.0f);
    Vector4 rotated = p * r;
    Vector4 restored = rotated * rInv;

    Float4 fp = p.GetFloat4();
    Float4 fr = restored.GetFloat4();
    EXPECT_NEAR(fr.x, fp.x, kEps);
    EXPECT_NEAR(fr.y, fp.y, kEps);
    EXPECT_NEAR(fr.z, fp.z, kEps);
}

TEST(MatrixUtilityTest, RotationXYZ_HandVerifiedResult)
{
    // Verify GetRotationXYZ against a hand-computed result.
    // For x=90, y=0, z=0: cosX=0, sinX=1, cosY=1, sinY=0, cosZ=1, sinZ=0
    Matrix r = MatrixUtility::GetRotationXYZ(Math::Deg2Rad(90.0f), 0.0f, 0.0f);

    float expected[4][4] = {
        {1.0f,  0.0f,  0.0f, 0.0f},
        {0.0f,  0.0f, -1.0f, 0.0f},
        {0.0f,  1.0f,  0.0f, 0.0f},
        {0.0f,  0.0f,  0.0f, 1.0f}
    };
    ExpectMatrixNear(r, expected);
}

TEST(MatrixUtilityTest, RotationXYZ_InverseIsNegatedAngles)
{
    // For any rotation matrix, inverse == transpose
    float xAngle = Math::Deg2Rad(25.0f);
    float yAngle = Math::Deg2Rad(53.0f);
    float zAngle = Math::Deg2Rad(78.0f);

    Matrix r = MatrixUtility::GetRotationXYZ(xAngle, yAngle, zAngle);
    Matrix rInv = r.Inversed();
    Matrix rT = r.Transposed();

    ExpectMatrixNear(rInv, rT);
}

TEST(MatrixUtilityTest, RotationAxis_ArbitraryAxis)
{
    // Rotate by 120 around the axis (1, 1, 1)/sqrt(3).
    // This cyclically permutes the axes: x -> y -> z -> x.
    Vector3 axis(1.0f, 1.0f, 1.0f);
    axis.Normalize();
    float angle = Math::Deg2Rad(120.0f);

    Matrix r = MatrixUtility::GetRotationAxis(axis, angle);

    // (1, 0, 0) -> (0, 1, 0)
    Vector4 px(1.0f, 0.0f, 0.0f, 0.0f);
    Float4 fx = (px * r).GetFloat4();
    EXPECT_NEAR(fx.x, 0.0f, kEps);
    EXPECT_NEAR(fx.y, 1.0f, kEps);
    EXPECT_NEAR(fx.z, 0.0f, kEps);

    // (0, 1, 0) -> (0, 0, 1)
    Vector4 py(0.0f, 1.0f, 0.0f, 0.0f);
    Float4 fy = (py * r).GetFloat4();
    EXPECT_NEAR(fy.x, 0.0f, kEps);
    EXPECT_NEAR(fy.y, 0.0f, kEps);
    EXPECT_NEAR(fy.z, 1.0f, kEps);

    // (0, 0, 1) -> (1, 0, 0)
    Vector4 pz(0.0f, 0.0f, 1.0f, 0.0f);
    Float4 fz = (pz * r).GetFloat4();
    EXPECT_NEAR(fz.x, 1.0f, kEps);
    EXPECT_NEAR(fz.y, 0.0f, kEps);
    EXPECT_NEAR(fz.z, 0.0f, kEps);
}

TEST(MatrixUtilityTest, RotationAxis_DoubleRotationEqualsSquare)
{
    // Rotating by angle twice via two multiplications should equal rotating by 2*angle
    Vector3 axis(1.0f, 2.0f, 3.0f);
    axis.Normalize();
    float angle = Math::Deg2Rad(37.0f);

    Matrix rSingle = MatrixUtility::GetRotationAxis(axis, angle);
    Matrix rDouble = MatrixUtility::GetRotationAxis(axis, angle * 2.0f);
    Matrix rSquared = rSingle * rSingle;

    ExpectMatrixNear(rDouble, rSquared);
}

TEST(MatrixUtilityTest, ViewProjectionPipeline)
{
    // Full view-projection pipeline: world point -> view -> clip -> NDC
    Vector3 eye(0.0f, 5.0f, -10.0f);
    Vector3 target(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Matrix view = MatrixUtility::GetLookAt(eye, target, up);

    float fov = Math::Deg2Rad(60.0f);
    float aspect = 16.0f / 9.0f;
    float nearZ = 0.1f;
    float farZ = 1000.0f;

    Matrix proj = MatrixUtility::GetPerspectiveFov(fov, aspect, nearZ, farZ);
    Matrix viewProj = view * proj;

    // The eye position itself should map to origin in view space
    Vector4 eyeH(eye.GetFloat3().x, eye.GetFloat3().y, eye.GetFloat3().z, 1.0f);
    Vector4 eyeView = eyeH * view;
    Float4 ev = eyeView.GetFloat4();
    EXPECT_NEAR(ev.x, 0.0f, kEps);
    EXPECT_NEAR(ev.y, 0.0f, kEps);
    EXPECT_NEAR(ev.z, 0.0f, kEps);

    // The target should be in front of camera (negative z in right-handed view space)
    Vector4 targetH(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 targetView = targetH * view;
    Float4 tv = targetView.GetFloat4();
    EXPECT_LT(tv.z, 0.0f);

    // Target is at center of view, so after view-proj its x,y should be 0
    Vector4 targetClip = targetH * viewProj;
    Float4 tc = targetClip.GetFloat4();
    float ndcX = tc.x / tc.w;
    float ndcY = tc.y / tc.w;
    EXPECT_NEAR(ndcX, 0.0f, kEps);
    EXPECT_NEAR(ndcY, 0.0f, kEps);

    // clip.w should be positive for visible objects
    EXPECT_GT(tc.w, 0.0f);
}

TEST(MatrixUtilityTest, LookAt_OrthonormalBasis)
{
    // The upper-left 3x3 of the view matrix should be an orthonormal basis
    Vector3 eye(3.0f, 7.0f, -2.0f);
    Vector3 target(-1.0f, 2.0f, 5.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Matrix view = MatrixUtility::GetLookAt(eye, target, up);

    // Extract the 3x3 rotation part (rows 0-2, cols 0-2)
    Vector3 r0(view.GetRow(0).GetFloat4().x, view.GetRow(0).GetFloat4().y, view.GetRow(0).GetFloat4().z);
    Vector3 r1(view.GetRow(1).GetFloat4().x, view.GetRow(1).GetFloat4().y, view.GetRow(1).GetFloat4().z);
    Vector3 r2(view.GetRow(2).GetFloat4().x, view.GetRow(2).GetFloat4().y, view.GetRow(2).GetFloat4().z);

    // Each row should be unit length
    EXPECT_NEAR(r0.Length(), 1.0f, kEps);
    EXPECT_NEAR(r1.Length(), 1.0f, kEps);
    EXPECT_NEAR(r2.Length(), 1.0f, kEps);

    // Rows should be mutually orthogonal
    EXPECT_NEAR(r0.Dot(r1), 0.0f, kEps);
    EXPECT_NEAR(r0.Dot(r2), 0.0f, kEps);
    EXPECT_NEAR(r1.Dot(r2), 0.0f, kEps);
}

TEST(MatrixUtilityTest, SRT_InverseReconstruction)
{
    // Build SRT, invert it, and verify it undoes the transform
    Matrix s = MatrixUtility::GetScale(2.0f, 0.5f, 3.0f);
    Matrix r = MatrixUtility::GetRotationXYZ(
        Math::Deg2Rad(15.0f), Math::Deg2Rad(30.0f), Math::Deg2Rad(45.0f));
    Matrix t = MatrixUtility::GetTranslation(10.0f, -5.0f, 3.0f);

    Matrix srt = s * r * t;
    Matrix srtInv = srt.Inversed();

    // Transforming a point through SRT then SRT^-1 should return the original
    Vector4 p(7.0f, -3.0f, 2.0f, 1.0f);
    Vector4 transformed = p * srt;
    Vector4 restored = transformed * srtInv;
    Float4 fp = p.GetFloat4();
    Float4 fr = restored.GetFloat4();
    EXPECT_NEAR(fr.x, fp.x, kEps);
    EXPECT_NEAR(fr.y, fp.y, kEps);
    EXPECT_NEAR(fr.z, fp.z, kEps);
    EXPECT_NEAR(fr.w, fp.w, kEps);
}
