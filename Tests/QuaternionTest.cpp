#include <gtest/gtest.h>

#include "CubeMath.h"
#include "Matrix.h"
#include "MatrixUtility.h"
#include "Quaternion.h"
#include "Vector.h"

using namespace cube;

constexpr float kEps = 1e-4f;

// Helper to compare two quaternions component-wise.
static void ExpectQuatNear(const Quaternion& a, const Quaternion& b, float eps = kEps)
{
    Float4 fa = a.GetFloat4();
    Float4 fb = b.GetFloat4();
    EXPECT_NEAR(fa.x, fb.x, eps);
    EXPECT_NEAR(fa.y, fb.y, eps);
    EXPECT_NEAR(fa.z, fb.z, eps);
    EXPECT_NEAR(fa.w, fb.w, eps);
}

// Helper to compare two matrices element-wise.
static void ExpectMatrixNear(const Matrix& a, const Matrix& b, float eps = kEps)
{
    for (int r = 0; r < 4; ++r)
    {
        Float4 fa = const_cast<Matrix&>(a)[r].GetFloat4();
        Float4 fb = const_cast<Matrix&>(b)[r].GetFloat4();
        EXPECT_NEAR(fa.x, fb.x, eps) << "row=" << r << " col=0";
        EXPECT_NEAR(fa.y, fb.y, eps) << "row=" << r << " col=1";
        EXPECT_NEAR(fa.z, fb.z, eps) << "row=" << r << " col=2";
        EXPECT_NEAR(fa.w, fb.w, eps) << "row=" << r << " col=3";
    }
}

// ===== Construction =====

TEST(QuaternionTest, ComponentConstruction)
{
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    Float4 f = q.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
    EXPECT_NEAR(f.w, 4.0f, kEps);
}

TEST(QuaternionTest, Identity)
{
    Quaternion q = Quaternion::Identity();
    Float4 f = q.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
    EXPECT_NEAR(f.w, 1.0f, kEps);
}

TEST(QuaternionTest, Equality)
{
    Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion b(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion c(1.0f, 2.0f, 3.0f, 5.0f);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST(QuaternionTest, CopyAndAssign)
{
    Quaternion a(1.0f, -2.0f, 3.0f, -4.0f);
    Quaternion b(a);
    Quaternion c;
    c = a;
    ExpectQuatNear(b, a);
    ExpectQuatNear(c, a);
}

// ===== Arithmetic =====

TEST(QuaternionTest, AddSub)
{
    Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion b(5.0f, 6.0f, 7.0f, 8.0f);
    ExpectQuatNear(a + b, Quaternion(6.0f, 8.0f, 10.0f, 12.0f));
    ExpectQuatNear(b - a, Quaternion(4.0f, 4.0f, 4.0f, 4.0f));
}

TEST(QuaternionTest, ScalarMultiply)
{
    Quaternion a(1.0f, -2.0f, 3.0f, -4.0f);
    ExpectQuatNear(a * 2.0f, Quaternion(2.0f, -4.0f, 6.0f, -8.0f));
    ExpectQuatNear(2.0f * a, Quaternion(2.0f, -4.0f, 6.0f, -8.0f));
}

TEST(QuaternionTest, UnaryMinus)
{
    Quaternion a(1.0f, -2.0f, 3.0f, -4.0f);
    ExpectQuatNear(-a, Quaternion(-1.0f, 2.0f, -3.0f, 4.0f));
}

TEST(QuaternionTest, HamiltonProduct)
{
    // Hand-computed: q1 = (1,2,3,4) (x,y,z,w), q2 = (5,6,7,8).
    // x = w1*x2 + x1*w2 + y1*z2 - z1*y2 = 4*5 + 1*8 + 2*7 - 3*6 = 20+8+14-18 = 24
    // y = w1*y2 - x1*z2 + y1*w2 + z1*x2 = 4*6 - 1*7 + 2*8 + 3*5 = 24-7+16+15 = 48
    // z = w1*z2 + x1*y2 - y1*x2 + z1*w2 = 4*7 + 1*6 - 2*5 + 3*8 = 28+6-10+24 = 48
    // w = w1*w2 - x1*x2 - y1*y2 - z1*z2 = 4*8 - 1*5 - 2*6 - 3*7 = 32-5-12-21 = -6
    Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion q2(5.0f, 6.0f, 7.0f, 8.0f);
    ExpectQuatNear(q1 * q2, Quaternion(24.0f, 48.0f, 48.0f, -6.0f));
}

TEST(QuaternionTest, IdentityIsMultiplicativeIdentity)
{
    Quaternion q = Quaternion::FromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), 0.7f);
    ExpectQuatNear(q * Quaternion::Identity(), q);
    ExpectQuatNear(Quaternion::Identity() * q, q);
}

TEST(QuaternionTest, ProductIsNonCommutative)
{
    Quaternion qx = Quaternion::FromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), Math::Deg2Rad(90.0f));
    Quaternion qy = Quaternion::FromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), Math::Deg2Rad(90.0f));
    EXPECT_TRUE((qx * qy) != (qy * qx));
}

// ===== Geometry =====

TEST(QuaternionTest, LengthAndNormalize)
{
    Quaternion q(1.0f, 2.0f, 2.0f, 4.0f); // length = sqrt(1+4+4+16) = 5
    EXPECT_NEAR(q.SquareLength(), 25.0f, kEps);
    EXPECT_NEAR(q.Length(), 5.0f, kEps);

    Quaternion n = q.Normalized();
    EXPECT_NEAR(n.Length(), 1.0f, kEps);
    // Original unchanged.
    EXPECT_NEAR(q.Length(), 5.0f, kEps);

    Quaternion m = q;
    m.Normalize();
    EXPECT_NEAR(m.Length(), 1.0f, kEps);
}

TEST(QuaternionTest, Dot)
{
    Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion b(5.0f, 6.0f, 7.0f, 8.0f);
    // 5+12+21+32 = 70
    EXPECT_NEAR(a.Dot(b), 70.0f, kEps);
    EXPECT_NEAR(Quaternion::Dot(a, b), 70.0f, kEps);
}

TEST(QuaternionTest, Conjugate)
{
    Quaternion q(1.0f, -2.0f, 3.0f, 4.0f);
    ExpectQuatNear(q.Conjugate(), Quaternion(-1.0f, 2.0f, -3.0f, 4.0f));
}

TEST(QuaternionTest, InverseRoundTrip)
{
    Quaternion q = Quaternion::FromAxisAngle(Vector3(1.0f, 2.0f, 3.0f), 1.1f);
    ExpectQuatNear(q * q.Inverse(), Quaternion::Identity());
    ExpectQuatNear(q.Inverse() * q, Quaternion::Identity());
}

TEST(QuaternionTest, FromAxisAngleIsUnit)
{
    Quaternion q = Quaternion::FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), Math::Deg2Rad(45.0f));
    EXPECT_NEAR(q.Length(), 1.0f, kEps);
    // Z rotation by 45deg: (0, 0, sin(22.5), cos(22.5))
    Float4 f = q.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, Math::Sin(Math::Deg2Rad(22.5f)), kEps);
    EXPECT_NEAR(f.w, Math::Cos(Math::Deg2Rad(22.5f)), kEps);
}

// ===== Matrix interop (validates row-vector convention) =====

TEST(QuaternionTest, ToRotationMatrixMatchesAxisAngle)
{
    Vector3 axis(0.3f, -0.5f, 0.8f);
    float angle = Math::Deg2Rad(57.0f);

    Vector3 normAxis = axis;
    normAxis.Normalize();

    Quaternion q = Quaternion::FromAxisAngle(normAxis, angle);
    Matrix fromQuat = q.ToRotationMatrix();
    Matrix fromUtil = MatrixUtility::GetRotationAxis(normAxis, angle);

    ExpectMatrixNear(fromQuat, fromUtil);
}

TEST(QuaternionTest, ToRotationMatrixMatchesEulerXYZ)
{
    float ax = Math::Deg2Rad(20.0f);
    float ay = Math::Deg2Rad(-35.0f);
    float az = Math::Deg2Rad(50.0f);

    Quaternion q = Quaternion::FromEulerXYZ(ax, ay, az);
    Matrix fromQuat = q.ToRotationMatrix();
    Matrix fromUtil = MatrixUtility::GetRotationXYZ(ax, ay, az);

    ExpectMatrixNear(fromQuat, fromUtil);
}

TEST(QuaternionTest, FromRotationMatrixRoundTrip)
{
    Vector3 axis(0.2f, 0.7f, -0.4f);
    float angle = Math::Deg2Rad(80.0f);
    Vector3 normAxis = axis;
    normAxis.Normalize();

    Quaternion q = Quaternion::FromAxisAngle(normAxis, angle);
    Matrix m = q.ToRotationMatrix();
    Quaternion recovered = Quaternion::FromRotationMatrix(m);

    // q and -q represent the same rotation; align the sign before comparing.
    if (q.Dot(recovered) < 0.0f)
    {
        recovered = -recovered;
    }
    ExpectQuatNear(recovered, q);
}

TEST(QuaternionTest, RotateVectorMatchesMatrix)
{
    Vector3 axis(0.0f, 1.0f, 0.0f);
    float angle = Math::Deg2Rad(90.0f);
    Quaternion q = Quaternion::FromAxisAngle(axis, angle);

    Vector3 v(1.0f, 0.0f, 0.0f);
    Vector3 rotated = q.RotateVector(v);

    // Compare against transforming through the equivalent rotation matrix.
    Vector4 viaMatrix = Vector4(v) * q.ToRotationMatrix();
    Float3 fq = rotated.GetFloat3();
    Float4 fm = viaMatrix.GetFloat4();
    EXPECT_NEAR(fq.x, fm.x, kEps);
    EXPECT_NEAR(fq.y, fm.y, kEps);
    EXPECT_NEAR(fq.z, fm.z, kEps);
}

// ===== Interpolation =====

TEST(QuaternionTest, SlerpEndpoints)
{
    Quaternion a = Quaternion::FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), Math::Deg2Rad(10.0f));
    Quaternion b = Quaternion::FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), Math::Deg2Rad(80.0f));

    Quaternion s0 = Quaternion::Slerp(a, b, 0.0f);
    Quaternion s1 = Quaternion::Slerp(a, b, 1.0f);
    ExpectQuatNear(s0, a);
    ExpectQuatNear(s1, b);
}

TEST(QuaternionTest, SlerpMidpointIsUnitAndHalfway)
{
    Quaternion a = Quaternion::FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), Math::Deg2Rad(0.0f));
    Quaternion b = Quaternion::FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), Math::Deg2Rad(90.0f));

    Quaternion mid = Quaternion::Slerp(a, b, 0.5f);
    EXPECT_NEAR(mid.Length(), 1.0f, kEps);

    Quaternion expected = Quaternion::FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), Math::Deg2Rad(45.0f));
    if (mid.Dot(expected) < 0.0f)
    {
        expected = -expected;
    }
    ExpectQuatNear(mid, expected);
}

TEST(QuaternionTest, LerpEndpoints)
{
    Quaternion a = Quaternion::FromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), Math::Deg2Rad(10.0f));
    Quaternion b = Quaternion::FromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), Math::Deg2Rad(70.0f));

    ExpectQuatNear(Quaternion::Lerp(a, b, 0.0f), a);
    ExpectQuatNear(Quaternion::Lerp(a, b, 1.0f), b);
    EXPECT_NEAR(Quaternion::Lerp(a, b, 0.5f).Length(), 1.0f, kEps);
}
