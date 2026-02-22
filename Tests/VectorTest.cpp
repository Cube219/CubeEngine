#include <gtest/gtest.h>

#include "Vector.h"

using namespace cube;

// Tolerance for floating-point comparison
constexpr float kEps = 1e-5f;

// ===== Vector2 Tests =====

TEST(Vector2Test, DefaultConstruction)
{
    Vector2 v;
    Float2 f = v.GetFloat2();
    // Default constructor - values are uninitialized, just verify it doesn't crash
    (void)f;
}

TEST(Vector2Test, ScalarConstruction)
{
    Vector2 v(3.0f);
    Float2 f = v.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 3.0f, kEps);
}

TEST(Vector2Test, ComponentConstruction)
{
    Vector2 v(1.0f, 2.0f);
    Float2 f = v.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

TEST(Vector2Test, CopyConstruction)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(a);
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

TEST(Vector2Test, Zero)
{
    Vector2 v = Vector2::Zero();
    Float2 f = v.GetFloat2();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
}

TEST(Vector2Test, Equality)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(1.0f, 2.0f);
    Vector2 c(3.0f, 4.0f);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST(Vector2Test, Addition)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    Vector2 c = a + b;
    Float2 f = c.GetFloat2();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
}

TEST(Vector2Test, Subtraction)
{
    Vector2 a(5.0f, 7.0f);
    Vector2 b(2.0f, 3.0f);
    Vector2 c = a - b;
    Float2 f = c.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
}

TEST(Vector2Test, ScalarMultiply)
{
    Vector2 a(2.0f, 3.0f);
    Vector2 b = a * 2.0f;
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);

    Vector2 c = a * 3.0f;
    Float2 g = c.GetFloat2();
    EXPECT_NEAR(g.x, 6.0f, kEps);
    EXPECT_NEAR(g.y, 9.0f, kEps);
}

TEST(Vector2Test, ElementWiseMultiply)
{
    Vector2 a(2.0f, 3.0f);
    Vector2 b(4.0f, 5.0f);
    Vector2 c = a * b;
    Float2 f = c.GetFloat2();
    EXPECT_NEAR(f.x, 8.0f, kEps);
    EXPECT_NEAR(f.y, 15.0f, kEps);
}

TEST(Vector2Test, ScalarDivide)
{
    Vector2 a(6.0f, 8.0f);
    Vector2 b = a / 2.0f;
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
}

TEST(Vector2Test, ElementWiseDivide)
{
    Vector2 a(6.0f, 10.0f);
    Vector2 b(2.0f, 5.0f);
    Vector2 c = a / b;
    Float2 f = c.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

TEST(Vector2Test, UnaryPlus)
{
    Vector2 a(1.0f, -2.0f);
    Vector2 b = +a;
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, -2.0f, kEps);
}

TEST(Vector2Test, UnaryMinus)
{
    Vector2 a(1.0f, -2.0f);
    Vector2 b = -a;
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, -1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

TEST(Vector2Test, CompoundAddition)
{
    Vector2 a(1.0f, 2.0f);
    a += Vector2(3.0f, 4.0f);
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
}

TEST(Vector2Test, CompoundSubtraction)
{
    Vector2 a(5.0f, 7.0f);
    a -= Vector2(2.0f, 3.0f);
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
}

TEST(Vector2Test, CompoundScalarMultiply)
{
    Vector2 a(2.0f, 3.0f);
    a *= 2.0f;
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
}

TEST(Vector2Test, CompoundElementWiseMultiply)
{
    Vector2 a(2.0f, 3.0f);
    a *= Vector2(4.0f, 5.0f);
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 8.0f, kEps);
    EXPECT_NEAR(f.y, 15.0f, kEps);
}

TEST(Vector2Test, CompoundScalarDivide)
{
    Vector2 a(6.0f, 8.0f);
    a /= 2.0f;
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
}

TEST(Vector2Test, CompoundElementWiseDivide)
{
    Vector2 a(6.0f, 10.0f);
    a /= Vector2(2.0f, 5.0f);
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

TEST(Vector2Test, Length)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_NEAR(v.Length(), 5.0f, kEps);
}

TEST(Vector2Test, SquareLength)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_NEAR(v.SquareLength(), 25.0f, kEps);
}

TEST(Vector2Test, LengthV)
{
    Vector2 v(3.0f, 4.0f);
    Vector2 lv = v.LengthV();
    Float2 f = lv.GetFloat2();
    EXPECT_NEAR(f.x, 5.0f, kEps);
    EXPECT_NEAR(f.y, 5.0f, kEps);
}

TEST(Vector2Test, SquareLengthV)
{
    Vector2 v(3.0f, 4.0f);
    Vector2 slv = v.SquareLengthV();
    Float2 f = slv.GetFloat2();
    EXPECT_NEAR(f.x, 25.0f, kEps);
    EXPECT_NEAR(f.y, 25.0f, kEps);
}

TEST(Vector2Test, Normalize)
{
    Vector2 v(3.0f, 4.0f);
    v.Normalize();
    Float2 f = v.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f / 5.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f / 5.0f, kEps);
    EXPECT_NEAR(v.Length(), 1.0f, kEps);
}

TEST(Vector2Test, Normalized)
{
    Vector2 v(3.0f, 4.0f);
    Vector2 n = v.Normalized();
    Float2 f = n.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f / 5.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f / 5.0f, kEps);
    // Original should be unchanged
    Float2 orig = v.GetFloat2();
    EXPECT_NEAR(orig.x, 3.0f, kEps);
    EXPECT_NEAR(orig.y, 4.0f, kEps);
}

TEST(Vector2Test, DotProduct)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    EXPECT_NEAR(a.Dot(b), 11.0f, kEps);
    EXPECT_NEAR(Vector2::Dot(a, b), 11.0f, kEps);
}

TEST(Vector2Test, DotProductV)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    Vector2 dv = a.DotV(b);
    Float2 f = dv.GetFloat2();
    EXPECT_NEAR(f.x, 11.0f, kEps);
    EXPECT_NEAR(f.y, 11.0f, kEps);

    Vector2 dv2 = Vector2::DotV(a, b);
    Float2 g = dv2.GetFloat2();
    EXPECT_NEAR(g.x, 11.0f, kEps);
    EXPECT_NEAR(g.y, 11.0f, kEps);
}

TEST(Vector2Test, Swap)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    a.Swap(b);
    Float2 fa = a.GetFloat2();
    Float2 fb = b.GetFloat2();
    EXPECT_NEAR(fa.x, 3.0f, kEps);
    EXPECT_NEAR(fa.y, 4.0f, kEps);
    EXPECT_NEAR(fb.x, 1.0f, kEps);
    EXPECT_NEAR(fb.y, 2.0f, kEps);
}

TEST(Vector2Test, StaticSwap)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    Vector2::Swap(a, b);
    Float2 fa = a.GetFloat2();
    Float2 fb = b.GetFloat2();
    EXPECT_NEAR(fa.x, 3.0f, kEps);
    EXPECT_NEAR(fa.y, 4.0f, kEps);
    EXPECT_NEAR(fb.x, 1.0f, kEps);
    EXPECT_NEAR(fb.y, 2.0f, kEps);
}

TEST(Vector2Test, CopyAssignment)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b;
    b = a;
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

TEST(Vector2Test, ScalarAssignment)
{
    Vector2 a;
    a = 5.0f;
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 5.0f, kEps);
    EXPECT_NEAR(f.y, 5.0f, kEps);
}

TEST(Vector2Test, GetFloat3FromVector2)
{
    Vector2 v(1.0f, 2.0f);
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

TEST(Vector2Test, GetFloat4FromVector2)
{
    Vector2 v(1.0f, 2.0f);
    Float4 f = v.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

// ===== Vector3 Tests =====

TEST(Vector3Test, ScalarConstruction)
{
    Vector3 v(3.0f);
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 3.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
}

TEST(Vector3Test, ComponentConstruction)
{
    Vector3 v(1.0f, 2.0f, 3.0f);
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
}

TEST(Vector3Test, CopyConstruction)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(a);
    Float3 f = b.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
}

TEST(Vector3Test, CrossDimensionConstruction)
{
    Vector2 v2(1.0f, 2.0f);
    Vector3 v3(v2);
    Float3 f = v3.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

TEST(Vector3Test, Zero)
{
    Vector3 v = Vector3::Zero();
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
}

TEST(Vector3Test, Equality)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(1.0f, 2.0f, 3.0f);
    Vector3 c(4.0f, 5.0f, 6.0f);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST(Vector3Test, Addition)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    Vector3 c = a + b;
    Float3 f = c.GetFloat3();
    EXPECT_NEAR(f.x, 5.0f, kEps);
    EXPECT_NEAR(f.y, 7.0f, kEps);
    EXPECT_NEAR(f.z, 9.0f, kEps);
}

TEST(Vector3Test, Subtraction)
{
    Vector3 a(5.0f, 7.0f, 9.0f);
    Vector3 b(2.0f, 3.0f, 4.0f);
    Vector3 c = a - b;
    Float3 f = c.GetFloat3();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
    EXPECT_NEAR(f.z, 5.0f, kEps);
}

TEST(Vector3Test, ScalarMultiply)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b = a * 2.0f;
    Float3 f = b.GetFloat3();
    EXPECT_NEAR(f.x, 2.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
    EXPECT_NEAR(f.z, 6.0f, kEps);

    Vector3 c = a * 3.0f;
    Float3 g = c.GetFloat3();
    EXPECT_NEAR(g.x, 3.0f, kEps);
    EXPECT_NEAR(g.y, 6.0f, kEps);
    EXPECT_NEAR(g.z, 9.0f, kEps);
}

TEST(Vector3Test, ElementWiseMultiply)
{
    Vector3 a(2.0f, 3.0f, 4.0f);
    Vector3 b(5.0f, 6.0f, 7.0f);
    Vector3 c = a * b;
    Float3 f = c.GetFloat3();
    EXPECT_NEAR(f.x, 10.0f, kEps);
    EXPECT_NEAR(f.y, 18.0f, kEps);
    EXPECT_NEAR(f.z, 28.0f, kEps);
}

TEST(Vector3Test, ScalarDivide)
{
    Vector3 a(6.0f, 8.0f, 10.0f);
    Vector3 b = a / 2.0f;
    Float3 f = b.GetFloat3();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
    EXPECT_NEAR(f.z, 5.0f, kEps);
}

TEST(Vector3Test, ElementWiseDivide)
{
    Vector3 a(6.0f, 12.0f, 20.0f);
    Vector3 b(2.0f, 3.0f, 4.0f);
    Vector3 c = a / b;
    Float3 f = c.GetFloat3();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
    EXPECT_NEAR(f.z, 5.0f, kEps);
}

TEST(Vector3Test, UnaryMinus)
{
    Vector3 a(1.0f, -2.0f, 3.0f);
    Vector3 b = -a;
    Float3 f = b.GetFloat3();
    EXPECT_NEAR(f.x, -1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, -3.0f, kEps);
}

TEST(Vector3Test, CompoundOperations)
{
    Vector3 a(1.0f, 2.0f, 3.0f);

    a += Vector3(1.0f, 1.0f, 1.0f);
    Float3 f1 = a.GetFloat3();
    EXPECT_NEAR(f1.x, 2.0f, kEps);
    EXPECT_NEAR(f1.y, 3.0f, kEps);
    EXPECT_NEAR(f1.z, 4.0f, kEps);

    a -= Vector3(1.0f, 1.0f, 1.0f);
    Float3 f2 = a.GetFloat3();
    EXPECT_NEAR(f2.x, 1.0f, kEps);
    EXPECT_NEAR(f2.y, 2.0f, kEps);
    EXPECT_NEAR(f2.z, 3.0f, kEps);

    a *= 2.0f;
    Float3 f3 = a.GetFloat3();
    EXPECT_NEAR(f3.x, 2.0f, kEps);
    EXPECT_NEAR(f3.y, 4.0f, kEps);
    EXPECT_NEAR(f3.z, 6.0f, kEps);

    a /= 2.0f;
    Float3 f4 = a.GetFloat3();
    EXPECT_NEAR(f4.x, 1.0f, kEps);
    EXPECT_NEAR(f4.y, 2.0f, kEps);
    EXPECT_NEAR(f4.z, 3.0f, kEps);

    a *= Vector3(2.0f, 3.0f, 4.0f);
    Float3 f5 = a.GetFloat3();
    EXPECT_NEAR(f5.x, 2.0f, kEps);
    EXPECT_NEAR(f5.y, 6.0f, kEps);
    EXPECT_NEAR(f5.z, 12.0f, kEps);

    a /= Vector3(2.0f, 3.0f, 4.0f);
    Float3 f6 = a.GetFloat3();
    EXPECT_NEAR(f6.x, 1.0f, kEps);
    EXPECT_NEAR(f6.y, 2.0f, kEps);
    EXPECT_NEAR(f6.z, 3.0f, kEps);
}

TEST(Vector3Test, Length)
{
    Vector3 v(1.0f, 2.0f, 2.0f);
    EXPECT_NEAR(v.Length(), 3.0f, kEps);
}

TEST(Vector3Test, SquareLength)
{
    Vector3 v(1.0f, 2.0f, 2.0f);
    EXPECT_NEAR(v.SquareLength(), 9.0f, kEps);
}

TEST(Vector3Test, LengthV)
{
    Vector3 v(1.0f, 2.0f, 2.0f);
    Vector3 lv = v.LengthV();
    Float3 f = lv.GetFloat3();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 3.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
}

TEST(Vector3Test, Normalize)
{
    Vector3 v(1.0f, 2.0f, 2.0f);
    v.Normalize();
    EXPECT_NEAR(v.Length(), 1.0f, kEps);
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f / 3.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f / 3.0f, kEps);
    EXPECT_NEAR(f.z, 2.0f / 3.0f, kEps);
}

TEST(Vector3Test, Normalized)
{
    Vector3 v(1.0f, 2.0f, 2.0f);
    Vector3 n = v.Normalized();
    EXPECT_NEAR(n.Length(), 1.0f, kEps);
    // Original unchanged
    EXPECT_NEAR(v.Length(), 3.0f, kEps);
}

TEST(Vector3Test, DotProduct)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    // 1*4 + 2*5 + 3*6 = 32
    EXPECT_NEAR(a.Dot(b), 32.0f, kEps);
    EXPECT_NEAR(Vector3::Dot(a, b), 32.0f, kEps);
}

TEST(Vector3Test, DotProductV)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    Vector3 dv = a.DotV(b);
    Float3 f = dv.GetFloat3();
    EXPECT_NEAR(f.x, 32.0f, kEps);
    EXPECT_NEAR(f.y, 32.0f, kEps);
    EXPECT_NEAR(f.z, 32.0f, kEps);
}

TEST(Vector3Test, CrossProduct)
{
    // i x j = k
    Vector3 i(1.0f, 0.0f, 0.0f);
    Vector3 j(0.0f, 1.0f, 0.0f);
    Vector3 k = i.Cross(j);
    Float3 f = k.GetFloat3();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 1.0f, kEps);

    // j x i = -k
    Vector3 nk = j.Cross(i);
    Float3 g = nk.GetFloat3();
    EXPECT_NEAR(g.x, 0.0f, kEps);
    EXPECT_NEAR(g.y, 0.0f, kEps);
    EXPECT_NEAR(g.z, -1.0f, kEps);
}

TEST(Vector3Test, StaticCrossProduct)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    // cross = (2*6-3*5, 3*4-1*6, 1*5-2*4) = (-3, 6, -3)
    Vector3 c = Vector3::Cross(a, b);
    Float3 f = c.GetFloat3();
    EXPECT_NEAR(f.x, -3.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
    EXPECT_NEAR(f.z, -3.0f, kEps);
}

TEST(Vector3Test, CrossProductPerpendicular)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    Vector3 c = a.Cross(b);
    // Cross product should be perpendicular to both inputs
    EXPECT_NEAR(c.Dot(a), 0.0f, kEps);
    EXPECT_NEAR(c.Dot(b), 0.0f, kEps);
}

TEST(Vector3Test, Swap)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    a.Swap(b);
    Float3 fa = a.GetFloat3();
    Float3 fb = b.GetFloat3();
    EXPECT_NEAR(fa.x, 4.0f, kEps);
    EXPECT_NEAR(fa.y, 5.0f, kEps);
    EXPECT_NEAR(fa.z, 6.0f, kEps);
    EXPECT_NEAR(fb.x, 1.0f, kEps);
    EXPECT_NEAR(fb.y, 2.0f, kEps);
    EXPECT_NEAR(fb.z, 3.0f, kEps);
}

TEST(Vector3Test, ScalarAssignment)
{
    Vector3 a;
    a = 7.0f;
    Float3 f = a.GetFloat3();
    EXPECT_NEAR(f.x, 7.0f, kEps);
    EXPECT_NEAR(f.y, 7.0f, kEps);
    EXPECT_NEAR(f.z, 7.0f, kEps);
}

// ===== Vector4 Tests =====

TEST(Vector4Test, ScalarConstruction)
{
    Vector4 v(3.0f);
    Float4 f = v.GetFloat4();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 3.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
    EXPECT_NEAR(f.w, 3.0f, kEps);
}

TEST(Vector4Test, ComponentConstruction)
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Float4 f = v.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
    EXPECT_NEAR(f.w, 4.0f, kEps);
}

TEST(Vector4Test, CopyConstruction)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(a);
    Float4 f = b.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
    EXPECT_NEAR(f.w, 4.0f, kEps);
}

TEST(Vector4Test, CrossDimensionConstruction)
{
    Vector3 v3(1.0f, 2.0f, 3.0f);
    Vector4 v4(v3);
    Float4 f = v4.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
}

TEST(Vector4Test, Zero)
{
    Vector4 v = Vector4::Zero();
    Float4 f = v.GetFloat4();
    EXPECT_NEAR(f.x, 0.0f, kEps);
    EXPECT_NEAR(f.y, 0.0f, kEps);
    EXPECT_NEAR(f.z, 0.0f, kEps);
    EXPECT_NEAR(f.w, 0.0f, kEps);
}

TEST(Vector4Test, Equality)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 c(5.0f, 6.0f, 7.0f, 8.0f);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST(Vector4Test, Addition)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4 c = a + b;
    Float4 f = c.GetFloat4();
    EXPECT_NEAR(f.x, 6.0f, kEps);
    EXPECT_NEAR(f.y, 8.0f, kEps);
    EXPECT_NEAR(f.z, 10.0f, kEps);
    EXPECT_NEAR(f.w, 12.0f, kEps);
}

TEST(Vector4Test, Subtraction)
{
    Vector4 a(5.0f, 7.0f, 9.0f, 11.0f);
    Vector4 b(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 c = a - b;
    Float4 f = c.GetFloat4();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 5.0f, kEps);
    EXPECT_NEAR(f.z, 6.0f, kEps);
    EXPECT_NEAR(f.w, 7.0f, kEps);
}

TEST(Vector4Test, ScalarMultiply)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b = a * 2.0f;
    Float4 f = b.GetFloat4();
    EXPECT_NEAR(f.x, 2.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
    EXPECT_NEAR(f.z, 6.0f, kEps);
    EXPECT_NEAR(f.w, 8.0f, kEps);

    Vector4 c = a * 3.0f;
    Float4 g = c.GetFloat4();
    EXPECT_NEAR(g.x, 3.0f, kEps);
    EXPECT_NEAR(g.y, 6.0f, kEps);
    EXPECT_NEAR(g.z, 9.0f, kEps);
    EXPECT_NEAR(g.w, 12.0f, kEps);
}

TEST(Vector4Test, ElementWiseMultiply)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4 c = a * b;
    Float4 f = c.GetFloat4();
    EXPECT_NEAR(f.x, 5.0f, kEps);
    EXPECT_NEAR(f.y, 12.0f, kEps);
    EXPECT_NEAR(f.z, 21.0f, kEps);
    EXPECT_NEAR(f.w, 32.0f, kEps);
}

TEST(Vector4Test, ScalarDivide)
{
    Vector4 a(2.0f, 4.0f, 6.0f, 8.0f);
    Vector4 b = a / 2.0f;
    Float4 f = b.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
    EXPECT_NEAR(f.w, 4.0f, kEps);
}

TEST(Vector4Test, ElementWiseDivide)
{
    Vector4 a(10.0f, 12.0f, 21.0f, 32.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4 c = a / b;
    Float4 f = c.GetFloat4();
    EXPECT_NEAR(f.x, 2.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
    EXPECT_NEAR(f.w, 4.0f, kEps);
}

TEST(Vector4Test, UnaryMinus)
{
    Vector4 a(1.0f, -2.0f, 3.0f, -4.0f);
    Vector4 b = -a;
    Float4 f = b.GetFloat4();
    EXPECT_NEAR(f.x, -1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, -3.0f, kEps);
    EXPECT_NEAR(f.w, 4.0f, kEps);
}

TEST(Vector4Test, CompoundOperations)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);

    a += Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    Float4 f1 = a.GetFloat4();
    EXPECT_NEAR(f1.x, 2.0f, kEps);
    EXPECT_NEAR(f1.y, 3.0f, kEps);
    EXPECT_NEAR(f1.z, 4.0f, kEps);
    EXPECT_NEAR(f1.w, 5.0f, kEps);

    a -= Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    Float4 f2 = a.GetFloat4();
    EXPECT_NEAR(f2.x, 1.0f, kEps);
    EXPECT_NEAR(f2.y, 2.0f, kEps);
    EXPECT_NEAR(f2.z, 3.0f, kEps);
    EXPECT_NEAR(f2.w, 4.0f, kEps);

    a *= 2.0f;
    Float4 f3 = a.GetFloat4();
    EXPECT_NEAR(f3.x, 2.0f, kEps);
    EXPECT_NEAR(f3.y, 4.0f, kEps);
    EXPECT_NEAR(f3.z, 6.0f, kEps);
    EXPECT_NEAR(f3.w, 8.0f, kEps);

    a /= 2.0f;
    Float4 f4 = a.GetFloat4();
    EXPECT_NEAR(f4.x, 1.0f, kEps);
    EXPECT_NEAR(f4.y, 2.0f, kEps);
    EXPECT_NEAR(f4.z, 3.0f, kEps);
    EXPECT_NEAR(f4.w, 4.0f, kEps);
}

TEST(Vector4Test, Length)
{
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    // sqrt(1+4+4+16) = sqrt(25) = 5
    EXPECT_NEAR(v.Length(), 5.0f, kEps);
}

TEST(Vector4Test, SquareLength)
{
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    EXPECT_NEAR(v.SquareLength(), 25.0f, kEps);
}

TEST(Vector4Test, LengthV)
{
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    Vector4 lv = v.LengthV();
    Float4 f = lv.GetFloat4();
    EXPECT_NEAR(f.x, 5.0f, kEps);
    EXPECT_NEAR(f.y, 5.0f, kEps);
    EXPECT_NEAR(f.z, 5.0f, kEps);
    EXPECT_NEAR(f.w, 5.0f, kEps);
}

TEST(Vector4Test, Normalize)
{
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    v.Normalize();
    EXPECT_NEAR(v.Length(), 1.0f, kEps);
}

TEST(Vector4Test, Normalized)
{
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    Vector4 n = v.Normalized();
    EXPECT_NEAR(n.Length(), 1.0f, kEps);
    // Original unchanged
    EXPECT_NEAR(v.Length(), 5.0f, kEps);
}

TEST(Vector4Test, DotProduct)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    // 1*5 + 2*6 + 3*7 + 4*8 = 5+12+21+32 = 70
    EXPECT_NEAR(a.Dot(b), 70.0f, kEps);
    EXPECT_NEAR(Vector4::Dot(a, b), 70.0f, kEps);
}

TEST(Vector4Test, DotProductV)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4 dv = a.DotV(b);
    Float4 f = dv.GetFloat4();
    EXPECT_NEAR(f.x, 70.0f, kEps);
    EXPECT_NEAR(f.y, 70.0f, kEps);
    EXPECT_NEAR(f.z, 70.0f, kEps);
    EXPECT_NEAR(f.w, 70.0f, kEps);
}

TEST(Vector4Test, Swap)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4::Swap(a, b);
    Float4 fa = a.GetFloat4();
    Float4 fb = b.GetFloat4();
    EXPECT_NEAR(fa.x, 5.0f, kEps);
    EXPECT_NEAR(fa.y, 6.0f, kEps);
    EXPECT_NEAR(fa.z, 7.0f, kEps);
    EXPECT_NEAR(fa.w, 8.0f, kEps);
    EXPECT_NEAR(fb.x, 1.0f, kEps);
    EXPECT_NEAR(fb.y, 2.0f, kEps);
    EXPECT_NEAR(fb.z, 3.0f, kEps);
    EXPECT_NEAR(fb.w, 4.0f, kEps);
}

TEST(Vector4Test, ScalarAssignment)
{
    Vector4 a;
    a = 9.0f;
    Float4 f = a.GetFloat4();
    EXPECT_NEAR(f.x, 9.0f, kEps);
    EXPECT_NEAR(f.y, 9.0f, kEps);
    EXPECT_NEAR(f.z, 9.0f, kEps);
    EXPECT_NEAR(f.w, 9.0f, kEps);
}

TEST(Vector4Test, GetFloat2FromVector4)
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Float2 f = v.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
}

TEST(Vector4Test, GetFloat3FromVector4)
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
}
