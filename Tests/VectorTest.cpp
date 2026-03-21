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

    Vector2 v2(-7.5f);
    Float2 f2 = v2.GetFloat2();
    EXPECT_NEAR(f2.x, -7.5f, kEps);
    EXPECT_NEAR(f2.y, -7.5f, kEps);

    Vector2 v3(0.001f);
    Float2 f3 = v3.GetFloat2();
    EXPECT_NEAR(f3.x, 0.001f, kEps);
    EXPECT_NEAR(f3.y, 0.001f, kEps);
}

TEST(Vector2Test, ComponentConstruction)
{
    Vector2 v(1.0f, 2.0f);
    Float2 f = v.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector2 v2(-3.5f, 100.0f);
    Float2 f2 = v2.GetFloat2();
    EXPECT_NEAR(f2.x, -3.5f, kEps);
    EXPECT_NEAR(f2.y, 100.0f, kEps);

    Vector2 v3(0.123f, -456.789f);
    Float2 f3 = v3.GetFloat2();
    EXPECT_NEAR(f3.x, 0.123f, kEps);
    EXPECT_NEAR(f3.y, -456.789f, kEps);
}

TEST(Vector2Test, CopyConstruction)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(a);
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector2 a2(-5.0f, 12.0f);
    Vector2 b2(a2);
    Float2 f2 = b2.GetFloat2();
    EXPECT_NEAR(f2.x, -5.0f, kEps);
    EXPECT_NEAR(f2.y, 12.0f, kEps);

    Vector2 a3(999.0f, -0.25f);
    Vector2 b3(a3);
    Float2 f3 = b3.GetFloat2();
    EXPECT_NEAR(f3.x, 999.0f, kEps);
    EXPECT_NEAR(f3.y, -0.25f, kEps);
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

    Vector2 d(-5.0f, 0.0f);
    Vector2 e(-5.0f, 0.0f);
    Vector2 g(-5.0f, 0.1f);
    EXPECT_TRUE(d == e);
    EXPECT_TRUE(d != g);

    Vector2 h(100.5f, -200.25f);
    Vector2 i(100.5f, -200.25f);
    EXPECT_TRUE(h == i);
    EXPECT_FALSE(h != i);
}

TEST(Vector2Test, Addition)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    Vector2 c = a + b;
    Float2 f = c.GetFloat2();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);

    Vector2 d(-10.0f, 5.0f);
    Vector2 e(10.0f, -5.0f);
    Float2 f2 = (d + e).GetFloat2();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);

    Vector2 g(0.1f, 0.2f);
    Vector2 h(0.3f, 0.4f);
    Float2 f3 = (g + h).GetFloat2();
    EXPECT_NEAR(f3.x, 0.4f, kEps);
    EXPECT_NEAR(f3.y, 0.6f, kEps);
}

TEST(Vector2Test, Subtraction)
{
    Vector2 a(5.0f, 7.0f);
    Vector2 b(2.0f, 3.0f);
    Vector2 c = a - b;
    Float2 f = c.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);

    Vector2 d(-3.0f, -7.0f);
    Vector2 e(-1.0f, -2.0f);
    Float2 f2 = (d - e).GetFloat2();
    EXPECT_NEAR(f2.x, -2.0f, kEps);
    EXPECT_NEAR(f2.y, -5.0f, kEps);

    Vector2 g(100.0f, 200.0f);
    Vector2 h(100.0f, 200.0f);
    Float2 f3 = (g - h).GetFloat2();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
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

    Vector2 d(-4.0f, 5.0f);
    Float2 f2 = (d * -3.0f).GetFloat2();
    EXPECT_NEAR(f2.x, 12.0f, kEps);
    EXPECT_NEAR(f2.y, -15.0f, kEps);

    Float2 f3 = (0.5f * d).GetFloat2();
    EXPECT_NEAR(f3.x, -2.0f, kEps);
    EXPECT_NEAR(f3.y, 2.5f, kEps);
}

TEST(Vector2Test, ElementWiseMultiply)
{
    Vector2 a(2.0f, 3.0f);
    Vector2 b(4.0f, 5.0f);
    Vector2 c = a * b;
    Float2 f = c.GetFloat2();
    EXPECT_NEAR(f.x, 8.0f, kEps);
    EXPECT_NEAR(f.y, 15.0f, kEps);

    Vector2 d(-2.0f, 3.0f);
    Vector2 e(5.0f, -4.0f);
    Float2 f2 = (d * e).GetFloat2();
    EXPECT_NEAR(f2.x, -10.0f, kEps);
    EXPECT_NEAR(f2.y, -12.0f, kEps);

    Vector2 g(0.5f, 0.25f);
    Vector2 h(4.0f, 8.0f);
    Float2 f3 = (g * h).GetFloat2();
    EXPECT_NEAR(f3.x, 2.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
}

TEST(Vector2Test, ScalarDivide)
{
    Vector2 a(6.0f, 8.0f);
    Vector2 b = a / 2.0f;
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);

    Vector2 c(-15.0f, 9.0f);
    Float2 f2 = (c / 3.0f).GetFloat2();
    EXPECT_NEAR(f2.x, -5.0f, kEps);
    EXPECT_NEAR(f2.y, 3.0f, kEps);

    Vector2 d(7.0f, -14.0f);
    Float2 f3 = (d / -7.0f).GetFloat2();
    EXPECT_NEAR(f3.x, -1.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
}

TEST(Vector2Test, ElementWiseDivide)
{
    Vector2 a(6.0f, 10.0f);
    Vector2 b(2.0f, 5.0f);
    Vector2 c = a / b;
    Float2 f = c.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector2 d(-12.0f, 15.0f);
    Vector2 e(4.0f, -3.0f);
    Float2 f2 = (d / e).GetFloat2();
    EXPECT_NEAR(f2.x, -3.0f, kEps);
    EXPECT_NEAR(f2.y, -5.0f, kEps);

    Vector2 g(1.0f, 1.0f);
    Vector2 h(0.5f, 0.25f);
    Float2 f3 = (g / h).GetFloat2();
    EXPECT_NEAR(f3.x, 2.0f, kEps);
    EXPECT_NEAR(f3.y, 4.0f, kEps);
}

TEST(Vector2Test, UnaryPlus)
{
    Vector2 a(1.0f, -2.0f);
    Vector2 b = +a;
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, -2.0f, kEps);

    Vector2 c(-100.0f, 50.5f);
    Float2 f2 = (+c).GetFloat2();
    EXPECT_NEAR(f2.x, -100.0f, kEps);
    EXPECT_NEAR(f2.y, 50.5f, kEps);
}

TEST(Vector2Test, UnaryMinus)
{
    Vector2 a(1.0f, -2.0f);
    Vector2 b = -a;
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, -1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector2 c(-100.0f, 50.5f);
    Float2 f2 = (-c).GetFloat2();
    EXPECT_NEAR(f2.x, 100.0f, kEps);
    EXPECT_NEAR(f2.y, -50.5f, kEps);

    Vector2 d(0.0f, 0.0f);
    Float2 f3 = (-d).GetFloat2();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
}

TEST(Vector2Test, CompoundAddition)
{
    Vector2 a(1.0f, 2.0f);
    a += Vector2(3.0f, 4.0f);
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);

    Vector2 b(-10.0f, 20.0f);
    b += Vector2(10.0f, -20.0f);
    Float2 f2 = b.GetFloat2();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);

    Vector2 c(0.5f, 0.25f);
    c += Vector2(0.5f, 0.75f);
    Float2 f3 = c.GetFloat2();
    EXPECT_NEAR(f3.x, 1.0f, kEps);
    EXPECT_NEAR(f3.y, 1.0f, kEps);
}

TEST(Vector2Test, CompoundSubtraction)
{
    Vector2 a(5.0f, 7.0f);
    a -= Vector2(2.0f, 3.0f);
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);

    Vector2 b(100.0f, -50.0f);
    b -= Vector2(100.0f, -50.0f);
    Float2 f2 = b.GetFloat2();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);

    Vector2 c(-3.0f, 7.0f);
    c -= Vector2(4.0f, -3.0f);
    Float2 f3 = c.GetFloat2();
    EXPECT_NEAR(f3.x, -7.0f, kEps);
    EXPECT_NEAR(f3.y, 10.0f, kEps);
}

TEST(Vector2Test, CompoundScalarMultiply)
{
    Vector2 a(2.0f, 3.0f);
    a *= 2.0f;
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 4.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);

    Vector2 b(-4.0f, 5.0f);
    b *= -3.0f;
    Float2 f2 = b.GetFloat2();
    EXPECT_NEAR(f2.x, 12.0f, kEps);
    EXPECT_NEAR(f2.y, -15.0f, kEps);

    Vector2 c(10.0f, 20.0f);
    c *= 0.1f;
    Float2 f3 = c.GetFloat2();
    EXPECT_NEAR(f3.x, 1.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
}

TEST(Vector2Test, CompoundElementWiseMultiply)
{
    Vector2 a(2.0f, 3.0f);
    a *= Vector2(4.0f, 5.0f);
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 8.0f, kEps);
    EXPECT_NEAR(f.y, 15.0f, kEps);

    Vector2 b(-3.0f, 7.0f);
    b *= Vector2(-2.0f, -1.0f);
    Float2 f2 = b.GetFloat2();
    EXPECT_NEAR(f2.x, 6.0f, kEps);
    EXPECT_NEAR(f2.y, -7.0f, kEps);

    Vector2 c(0.5f, 4.0f);
    c *= Vector2(6.0f, 0.25f);
    Float2 f3 = c.GetFloat2();
    EXPECT_NEAR(f3.x, 3.0f, kEps);
    EXPECT_NEAR(f3.y, 1.0f, kEps);
}

TEST(Vector2Test, CompoundScalarDivide)
{
    Vector2 a(6.0f, 8.0f);
    a /= 2.0f;
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);

    Vector2 b(-15.0f, 9.0f);
    b /= -3.0f;
    Float2 f2 = b.GetFloat2();
    EXPECT_NEAR(f2.x, 5.0f, kEps);
    EXPECT_NEAR(f2.y, -3.0f, kEps);

    Vector2 c(100.0f, 200.0f);
    c /= 0.5f;
    Float2 f3 = c.GetFloat2();
    EXPECT_NEAR(f3.x, 200.0f, kEps);
    EXPECT_NEAR(f3.y, 400.0f, kEps);
}

TEST(Vector2Test, CompoundElementWiseDivide)
{
    Vector2 a(6.0f, 10.0f);
    a /= Vector2(2.0f, 5.0f);
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector2 b(-12.0f, 15.0f);
    b /= Vector2(-4.0f, 3.0f);
    Float2 f2 = b.GetFloat2();
    EXPECT_NEAR(f2.x, 3.0f, kEps);
    EXPECT_NEAR(f2.y, 5.0f, kEps);

    Vector2 c(1.0f, 1.0f);
    c /= Vector2(0.25f, 0.5f);
    Float2 f3 = c.GetFloat2();
    EXPECT_NEAR(f3.x, 4.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
}

TEST(Vector2Test, Length)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_NEAR(v.Length(), 5.0f, kEps);

    // sqrt(5^2 + 12^2) = sqrt(169) = 13
    Vector2 v2(5.0f, 12.0f);
    EXPECT_NEAR(v2.Length(), 13.0f, kEps);

    // sqrt(8^2 + 6^2) = sqrt(100) = 10
    Vector2 v3(-8.0f, 6.0f);
    EXPECT_NEAR(v3.Length(), 10.0f, kEps);
}

TEST(Vector2Test, SquareLength)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_NEAR(v.SquareLength(), 25.0f, kEps);

    Vector2 v2(5.0f, 12.0f);
    EXPECT_NEAR(v2.SquareLength(), 169.0f, kEps);

    Vector2 v3(-2.0f, -3.0f);
    EXPECT_NEAR(v3.SquareLength(), 13.0f, kEps);
}

TEST(Vector2Test, LengthV)
{
    Vector2 v(3.0f, 4.0f);
    Vector2 lv = v.LengthV();
    Float2 f = lv.GetFloat2();
    EXPECT_NEAR(f.x, 5.0f, kEps);
    EXPECT_NEAR(f.y, 5.0f, kEps);

    Vector2 v2(5.0f, 12.0f);
    Float2 f2 = v2.LengthV().GetFloat2();
    EXPECT_NEAR(f2.x, 13.0f, kEps);
    EXPECT_NEAR(f2.y, 13.0f, kEps);
}

TEST(Vector2Test, SquareLengthV)
{
    Vector2 v(3.0f, 4.0f);
    Vector2 slv = v.SquareLengthV();
    Float2 f = slv.GetFloat2();
    EXPECT_NEAR(f.x, 25.0f, kEps);
    EXPECT_NEAR(f.y, 25.0f, kEps);

    Vector2 v2(-2.0f, -3.0f);
    Float2 f2 = v2.SquareLengthV().GetFloat2();
    EXPECT_NEAR(f2.x, 13.0f, kEps);
    EXPECT_NEAR(f2.y, 13.0f, kEps);
}

TEST(Vector2Test, Normalize)
{
    Vector2 v(3.0f, 4.0f);
    v.Normalize();
    Float2 f = v.GetFloat2();
    EXPECT_NEAR(f.x, 3.0f / 5.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f / 5.0f, kEps);
    EXPECT_NEAR(v.Length(), 1.0f, kEps);

    // (5, 12) -> len=13, normalized=(5/13, 12/13)
    Vector2 v2(5.0f, 12.0f);
    v2.Normalize();
    Float2 f2 = v2.GetFloat2();
    EXPECT_NEAR(f2.x, 5.0f / 13.0f, kEps);
    EXPECT_NEAR(f2.y, 12.0f / 13.0f, kEps);
    EXPECT_NEAR(v2.Length(), 1.0f, kEps);

    // (-8, 6) -> len=10
    Vector2 v3(-8.0f, 6.0f);
    v3.Normalize();
    EXPECT_NEAR(v3.Length(), 1.0f, kEps);
    Float2 f3 = v3.GetFloat2();
    EXPECT_NEAR(f3.x, -0.8f, kEps);
    EXPECT_NEAR(f3.y, 0.6f, kEps);
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

    Vector2 v2(-5.0f, 12.0f);
    Vector2 n2 = v2.Normalized();
    EXPECT_NEAR(n2.Length(), 1.0f, kEps);
    Float2 orig2 = v2.GetFloat2();
    EXPECT_NEAR(orig2.x, -5.0f, kEps);
    EXPECT_NEAR(orig2.y, 12.0f, kEps);
}

TEST(Vector2Test, DotProduct)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    EXPECT_NEAR(a.Dot(b), 11.0f, kEps);
    EXPECT_NEAR(Vector2::Dot(a, b), 11.0f, kEps);

    // (-3)*4 + 5*(-2) = -12 + -10 = -22
    Vector2 c(-3.0f, 5.0f);
    Vector2 d(4.0f, -2.0f);
    EXPECT_NEAR(c.Dot(d), -22.0f, kEps);
    EXPECT_NEAR(Vector2::Dot(c, d), -22.0f, kEps);

    // Perpendicular: (1,0) . (0,1) = 0
    Vector2 e(1.0f, 0.0f);
    Vector2 g(0.0f, 1.0f);
    EXPECT_NEAR(e.Dot(g), 0.0f, kEps);
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

    Vector2 c(-3.0f, 5.0f);
    Vector2 d(4.0f, -2.0f);
    Float2 f2 = c.DotV(d).GetFloat2();
    EXPECT_NEAR(f2.x, -22.0f, kEps);
    EXPECT_NEAR(f2.y, -22.0f, kEps);
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

    Vector2 c(-10.0f, 20.0f);
    Vector2 d(0.5f, -0.5f);
    c.Swap(d);
    Float2 fc = c.GetFloat2();
    Float2 fd = d.GetFloat2();
    EXPECT_NEAR(fc.x, 0.5f, kEps);
    EXPECT_NEAR(fc.y, -0.5f, kEps);
    EXPECT_NEAR(fd.x, -10.0f, kEps);
    EXPECT_NEAR(fd.y, 20.0f, kEps);
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

    Vector2 c(100.0f, -200.0f);
    Vector2 d(-300.0f, 400.0f);
    Vector2::Swap(c, d);
    Float2 fc = c.GetFloat2();
    Float2 fd = d.GetFloat2();
    EXPECT_NEAR(fc.x, -300.0f, kEps);
    EXPECT_NEAR(fc.y, 400.0f, kEps);
    EXPECT_NEAR(fd.x, 100.0f, kEps);
    EXPECT_NEAR(fd.y, -200.0f, kEps);
}

TEST(Vector2Test, CopyAssignment)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b;
    b = a;
    Float2 f = b.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector2 c(-99.0f, 0.5f);
    Vector2 d;
    d = c;
    Float2 f2 = d.GetFloat2();
    EXPECT_NEAR(f2.x, -99.0f, kEps);
    EXPECT_NEAR(f2.y, 0.5f, kEps);
}

TEST(Vector2Test, ScalarAssignment)
{
    Vector2 a;
    a = 5.0f;
    Float2 f = a.GetFloat2();
    EXPECT_NEAR(f.x, 5.0f, kEps);
    EXPECT_NEAR(f.y, 5.0f, kEps);

    Vector2 b;
    b = -42.0f;
    Float2 f2 = b.GetFloat2();
    EXPECT_NEAR(f2.x, -42.0f, kEps);
    EXPECT_NEAR(f2.y, -42.0f, kEps);

    Vector2 c;
    c = 0.0f;
    Float2 f3 = c.GetFloat2();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
}

TEST(Vector2Test, GetFloat3FromVector2)
{
    Vector2 v(1.0f, 2.0f);
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector2 v2(-7.5f, 100.0f);
    Float3 f2 = v2.GetFloat3();
    EXPECT_NEAR(f2.x, -7.5f, kEps);
    EXPECT_NEAR(f2.y, 100.0f, kEps);
}

TEST(Vector2Test, GetFloat4FromVector2)
{
    Vector2 v(1.0f, 2.0f);
    Float4 f = v.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector2 v2(-50.0f, 0.25f);
    Float4 f2 = v2.GetFloat4();
    EXPECT_NEAR(f2.x, -50.0f, kEps);
    EXPECT_NEAR(f2.y, 0.25f, kEps);
}

// ===== Vector3 Tests =====

TEST(Vector3Test, ScalarConstruction)
{
    Vector3 v(3.0f);
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 3.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);

    Vector3 v2(-12.5f);
    Float3 f2 = v2.GetFloat3();
    EXPECT_NEAR(f2.x, -12.5f, kEps);
    EXPECT_NEAR(f2.y, -12.5f, kEps);
    EXPECT_NEAR(f2.z, -12.5f, kEps);

    Vector3 v3(0.001f);
    Float3 f3 = v3.GetFloat3();
    EXPECT_NEAR(f3.x, 0.001f, kEps);
    EXPECT_NEAR(f3.y, 0.001f, kEps);
    EXPECT_NEAR(f3.z, 0.001f, kEps);
}

TEST(Vector3Test, ComponentConstruction)
{
    Vector3 v(1.0f, 2.0f, 3.0f);
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);

    Vector3 v2(-5.0f, 0.0f, 100.0f);
    Float3 f2 = v2.GetFloat3();
    EXPECT_NEAR(f2.x, -5.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 100.0f, kEps);

    Vector3 v3(0.1f, -0.2f, 0.3f);
    Float3 f3 = v3.GetFloat3();
    EXPECT_NEAR(f3.x, 0.1f, kEps);
    EXPECT_NEAR(f3.y, -0.2f, kEps);
    EXPECT_NEAR(f3.z, 0.3f, kEps);
}

TEST(Vector3Test, CopyConstruction)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(a);
    Float3 f = b.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);

    Vector3 a2(-7.0f, 0.0f, 42.0f);
    Vector3 b2(a2);
    Float3 f2 = b2.GetFloat3();
    EXPECT_NEAR(f2.x, -7.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 42.0f, kEps);
}

TEST(Vector3Test, CrossDimensionConstruction)
{
    Vector2 v2(1.0f, 2.0f);
    Vector3 v3(v2);
    Float3 f = v3.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector2 v2b(-5.0f, 10.0f);
    Vector3 v3b(v2b);
    Float3 f2 = v3b.GetFloat3();
    EXPECT_NEAR(f2.x, -5.0f, kEps);
    EXPECT_NEAR(f2.y, 10.0f, kEps);
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

    Vector3 d(-1.0f, 0.0f, 100.0f);
    Vector3 e(-1.0f, 0.0f, 100.0f);
    Vector3 g(-1.0f, 0.0f, 100.1f);
    EXPECT_TRUE(d == e);
    EXPECT_TRUE(d != g);
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

    Vector3 d(-10.0f, 5.0f, -3.0f);
    Vector3 e(10.0f, -5.0f, 3.0f);
    Float3 f2 = (d + e).GetFloat3();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 0.0f, kEps);

    Vector3 g(0.1f, 0.2f, 0.3f);
    Vector3 h(0.4f, 0.5f, 0.6f);
    Float3 f3 = (g + h).GetFloat3();
    EXPECT_NEAR(f3.x, 0.5f, kEps);
    EXPECT_NEAR(f3.y, 0.7f, kEps);
    EXPECT_NEAR(f3.z, 0.9f, kEps);
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

    Vector3 d(-3.0f, -7.0f, 10.0f);
    Vector3 e(-1.0f, -2.0f, 4.0f);
    Float3 f2 = (d - e).GetFloat3();
    EXPECT_NEAR(f2.x, -2.0f, kEps);
    EXPECT_NEAR(f2.y, -5.0f, kEps);
    EXPECT_NEAR(f2.z, 6.0f, kEps);

    Vector3 g(50.0f, 50.0f, 50.0f);
    Float3 f3 = (g - g).GetFloat3();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
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

    Vector3 d(-4.0f, 5.0f, -6.0f);
    Float3 f2 = (d * -2.0f).GetFloat3();
    EXPECT_NEAR(f2.x, 8.0f, kEps);
    EXPECT_NEAR(f2.y, -10.0f, kEps);
    EXPECT_NEAR(f2.z, 12.0f, kEps);

    Float3 f3 = (0.5f * d).GetFloat3();
    EXPECT_NEAR(f3.x, -2.0f, kEps);
    EXPECT_NEAR(f3.y, 2.5f, kEps);
    EXPECT_NEAR(f3.z, -3.0f, kEps);
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

    Vector3 d(-3.0f, 4.0f, -5.0f);
    Vector3 e(2.0f, -3.0f, -2.0f);
    Float3 f2 = (d * e).GetFloat3();
    EXPECT_NEAR(f2.x, -6.0f, kEps);
    EXPECT_NEAR(f2.y, -12.0f, kEps);
    EXPECT_NEAR(f2.z, 10.0f, kEps);

    Vector3 g(0.5f, 0.25f, 0.125f);
    Vector3 h(4.0f, 8.0f, 16.0f);
    Float3 f3 = (g * h).GetFloat3();
    EXPECT_NEAR(f3.x, 2.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
    EXPECT_NEAR(f3.z, 2.0f, kEps);
}

TEST(Vector3Test, ScalarDivide)
{
    Vector3 a(6.0f, 8.0f, 10.0f);
    Vector3 b = a / 2.0f;
    Float3 f = b.GetFloat3();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 4.0f, kEps);
    EXPECT_NEAR(f.z, 5.0f, kEps);

    Vector3 c(-15.0f, 9.0f, -21.0f);
    Float3 f2 = (c / 3.0f).GetFloat3();
    EXPECT_NEAR(f2.x, -5.0f, kEps);
    EXPECT_NEAR(f2.y, 3.0f, kEps);
    EXPECT_NEAR(f2.z, -7.0f, kEps);

    Vector3 d(7.0f, -14.0f, 28.0f);
    Float3 f3 = (d / -7.0f).GetFloat3();
    EXPECT_NEAR(f3.x, -1.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
    EXPECT_NEAR(f3.z, -4.0f, kEps);
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

    Vector3 d(-12.0f, 15.0f, -30.0f);
    Vector3 e(4.0f, -3.0f, 6.0f);
    Float3 f2 = (d / e).GetFloat3();
    EXPECT_NEAR(f2.x, -3.0f, kEps);
    EXPECT_NEAR(f2.y, -5.0f, kEps);
    EXPECT_NEAR(f2.z, -5.0f, kEps);

    Vector3 g(1.0f, 1.0f, 1.0f);
    Vector3 h(0.5f, 0.25f, 0.125f);
    Float3 f3 = (g / h).GetFloat3();
    EXPECT_NEAR(f3.x, 2.0f, kEps);
    EXPECT_NEAR(f3.y, 4.0f, kEps);
    EXPECT_NEAR(f3.z, 8.0f, kEps);
}

TEST(Vector3Test, UnaryMinus)
{
    Vector3 a(1.0f, -2.0f, 3.0f);
    Vector3 b = -a;
    Float3 f = b.GetFloat3();
    EXPECT_NEAR(f.x, -1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, -3.0f, kEps);

    Vector3 c(-100.0f, 0.0f, 50.5f);
    Float3 f2 = (-c).GetFloat3();
    EXPECT_NEAR(f2.x, 100.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, -50.5f, kEps);

    Vector3 d(0.0f, 0.0f, 0.0f);
    Float3 f3 = (-d).GetFloat3();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
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

    // Additional compound chain with different values
    Vector3 b(-5.0f, 10.0f, -15.0f);
    b += Vector3(5.0f, -10.0f, 15.0f);
    Float3 g1 = b.GetFloat3();
    EXPECT_NEAR(g1.x, 0.0f, kEps);
    EXPECT_NEAR(g1.y, 0.0f, kEps);
    EXPECT_NEAR(g1.z, 0.0f, kEps);

    Vector3 c(2.0f, -4.0f, 8.0f);
    c *= -0.5f;
    Float3 g2 = c.GetFloat3();
    EXPECT_NEAR(g2.x, -1.0f, kEps);
    EXPECT_NEAR(g2.y, 2.0f, kEps);
    EXPECT_NEAR(g2.z, -4.0f, kEps);
}

TEST(Vector3Test, Length)
{
    Vector3 v(1.0f, 2.0f, 2.0f);
    EXPECT_NEAR(v.Length(), 3.0f, kEps);

    // sqrt(2^2 + 3^2 + 6^2) = sqrt(49) = 7
    Vector3 v2(2.0f, 3.0f, 6.0f);
    EXPECT_NEAR(v2.Length(), 7.0f, kEps);

    // sqrt(3^2 + 4^2 + 12^2) = sqrt(169) = 13
    Vector3 v3(-3.0f, 4.0f, -12.0f);
    EXPECT_NEAR(v3.Length(), 13.0f, kEps);
}

TEST(Vector3Test, SquareLength)
{
    Vector3 v(1.0f, 2.0f, 2.0f);
    EXPECT_NEAR(v.SquareLength(), 9.0f, kEps);

    Vector3 v2(2.0f, 3.0f, 6.0f);
    EXPECT_NEAR(v2.SquareLength(), 49.0f, kEps);

    Vector3 v3(-3.0f, 4.0f, -12.0f);
    EXPECT_NEAR(v3.SquareLength(), 169.0f, kEps);
}

TEST(Vector3Test, LengthV)
{
    Vector3 v(1.0f, 2.0f, 2.0f);
    Vector3 lv = v.LengthV();
    Float3 f = lv.GetFloat3();
    EXPECT_NEAR(f.x, 3.0f, kEps);
    EXPECT_NEAR(f.y, 3.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);

    Vector3 v2(2.0f, 3.0f, 6.0f);
    Float3 f2 = v2.LengthV().GetFloat3();
    EXPECT_NEAR(f2.x, 7.0f, kEps);
    EXPECT_NEAR(f2.y, 7.0f, kEps);
    EXPECT_NEAR(f2.z, 7.0f, kEps);
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

    // (2, 3, 6) -> len=7
    Vector3 v2(2.0f, 3.0f, 6.0f);
    v2.Normalize();
    EXPECT_NEAR(v2.Length(), 1.0f, kEps);
    Float3 f2 = v2.GetFloat3();
    EXPECT_NEAR(f2.x, 2.0f / 7.0f, kEps);
    EXPECT_NEAR(f2.y, 3.0f / 7.0f, kEps);
    EXPECT_NEAR(f2.z, 6.0f / 7.0f, kEps);

    // (-3, 4, -12) -> len=13
    Vector3 v3(-3.0f, 4.0f, -12.0f);
    v3.Normalize();
    EXPECT_NEAR(v3.Length(), 1.0f, kEps);
    Float3 f3 = v3.GetFloat3();
    EXPECT_NEAR(f3.x, -3.0f / 13.0f, kEps);
    EXPECT_NEAR(f3.y, 4.0f / 13.0f, kEps);
    EXPECT_NEAR(f3.z, -12.0f / 13.0f, kEps);
}

TEST(Vector3Test, Normalized)
{
    Vector3 v(1.0f, 2.0f, 2.0f);
    Vector3 n = v.Normalized();
    EXPECT_NEAR(n.Length(), 1.0f, kEps);
    // Original unchanged
    EXPECT_NEAR(v.Length(), 3.0f, kEps);

    Vector3 v2(2.0f, 3.0f, 6.0f);
    Vector3 n2 = v2.Normalized();
    EXPECT_NEAR(n2.Length(), 1.0f, kEps);
    EXPECT_NEAR(v2.Length(), 7.0f, kEps);
}

TEST(Vector3Test, DotProduct)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    // 1*4 + 2*5 + 3*6 = 32
    EXPECT_NEAR(a.Dot(b), 32.0f, kEps);
    EXPECT_NEAR(Vector3::Dot(a, b), 32.0f, kEps);

    // (-2)*3 + 5*(-1) + 4*2 = -6 + -5 + 8 = -3
    Vector3 c(-2.0f, 5.0f, 4.0f);
    Vector3 d(3.0f, -1.0f, 2.0f);
    EXPECT_NEAR(c.Dot(d), -3.0f, kEps);
    EXPECT_NEAR(Vector3::Dot(c, d), -3.0f, kEps);

    // Perpendicular: (1,0,0) . (0,1,0) = 0
    Vector3 e(1.0f, 0.0f, 0.0f);
    Vector3 g(0.0f, 1.0f, 0.0f);
    EXPECT_NEAR(e.Dot(g), 0.0f, kEps);
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

    Vector3 c(-2.0f, 5.0f, 4.0f);
    Vector3 d(3.0f, -1.0f, 2.0f);
    Float3 f2 = c.DotV(d).GetFloat3();
    EXPECT_NEAR(f2.x, -3.0f, kEps);
    EXPECT_NEAR(f2.y, -3.0f, kEps);
    EXPECT_NEAR(f2.z, -3.0f, kEps);
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

    // j x k = i
    Vector3 kv(0.0f, 0.0f, 1.0f);
    Float3 f2 = j.Cross(kv).GetFloat3();
    EXPECT_NEAR(f2.x, 1.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 0.0f, kEps);

    // k x i = j
    Float3 f3 = kv.Cross(i).GetFloat3();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 1.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
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

    // (2, -1, 3) x (1, 4, -2) = ((-1)*(-2)-3*4, 3*1-2*(-2), 2*4-(-1)*1) = (2-12, 3+4, 8+1) = (-10, 7, 9)
    Vector3 d(2.0f, -1.0f, 3.0f);
    Vector3 e(1.0f, 4.0f, -2.0f);
    Float3 f2 = Vector3::Cross(d, e).GetFloat3();
    EXPECT_NEAR(f2.x, -10.0f, kEps);
    EXPECT_NEAR(f2.y, 7.0f, kEps);
    EXPECT_NEAR(f2.z, 9.0f, kEps);

    // Parallel vectors: cross product should be zero
    Vector3 g(1.0f, 2.0f, 3.0f);
    Vector3 h(2.0f, 4.0f, 6.0f);
    Float3 f3 = Vector3::Cross(g, h).GetFloat3();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
}

TEST(Vector3Test, CrossProductPerpendicular)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    Vector3 c = a.Cross(b);
    // Cross product should be perpendicular to both inputs
    EXPECT_NEAR(c.Dot(a), 0.0f, kEps);
    EXPECT_NEAR(c.Dot(b), 0.0f, kEps);

    Vector3 d(2.0f, -1.0f, 3.0f);
    Vector3 e(1.0f, 4.0f, -2.0f);
    Vector3 g = d.Cross(e);
    EXPECT_NEAR(g.Dot(d), 0.0f, kEps);
    EXPECT_NEAR(g.Dot(e), 0.0f, kEps);

    Vector3 h(-5.0f, 7.0f, 2.0f);
    Vector3 i(3.0f, -4.0f, 8.0f);
    Vector3 j = h.Cross(i);
    EXPECT_NEAR(j.Dot(h), 0.0f, kEps);
    EXPECT_NEAR(j.Dot(i), 0.0f, kEps);
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

    Vector3 c(-10.0f, 20.0f, -30.0f);
    Vector3 d(0.5f, -0.5f, 0.25f);
    c.Swap(d);
    Float3 fc = c.GetFloat3();
    Float3 fd = d.GetFloat3();
    EXPECT_NEAR(fc.x, 0.5f, kEps);
    EXPECT_NEAR(fc.y, -0.5f, kEps);
    EXPECT_NEAR(fc.z, 0.25f, kEps);
    EXPECT_NEAR(fd.x, -10.0f, kEps);
    EXPECT_NEAR(fd.y, 20.0f, kEps);
    EXPECT_NEAR(fd.z, -30.0f, kEps);
}

TEST(Vector3Test, ScalarAssignment)
{
    Vector3 a;
    a = 7.0f;
    Float3 f = a.GetFloat3();
    EXPECT_NEAR(f.x, 7.0f, kEps);
    EXPECT_NEAR(f.y, 7.0f, kEps);
    EXPECT_NEAR(f.z, 7.0f, kEps);

    Vector3 b;
    b = -42.0f;
    Float3 f2 = b.GetFloat3();
    EXPECT_NEAR(f2.x, -42.0f, kEps);
    EXPECT_NEAR(f2.y, -42.0f, kEps);
    EXPECT_NEAR(f2.z, -42.0f, kEps);

    Vector3 c;
    c = 0.0f;
    Float3 f3 = c.GetFloat3();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
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

    Vector4 v2(-12.5f);
    Float4 f2 = v2.GetFloat4();
    EXPECT_NEAR(f2.x, -12.5f, kEps);
    EXPECT_NEAR(f2.y, -12.5f, kEps);
    EXPECT_NEAR(f2.z, -12.5f, kEps);
    EXPECT_NEAR(f2.w, -12.5f, kEps);

    Vector4 v3(0.001f);
    Float4 f3 = v3.GetFloat4();
    EXPECT_NEAR(f3.x, 0.001f, kEps);
    EXPECT_NEAR(f3.y, 0.001f, kEps);
    EXPECT_NEAR(f3.z, 0.001f, kEps);
    EXPECT_NEAR(f3.w, 0.001f, kEps);
}

TEST(Vector4Test, ComponentConstruction)
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Float4 f = v.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
    EXPECT_NEAR(f.w, 4.0f, kEps);

    Vector4 v2(-5.0f, 0.0f, 100.0f, -0.5f);
    Float4 f2 = v2.GetFloat4();
    EXPECT_NEAR(f2.x, -5.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 100.0f, kEps);
    EXPECT_NEAR(f2.w, -0.5f, kEps);

    Vector4 v3(0.1f, -0.2f, 0.3f, -0.4f);
    Float4 f3 = v3.GetFloat4();
    EXPECT_NEAR(f3.x, 0.1f, kEps);
    EXPECT_NEAR(f3.y, -0.2f, kEps);
    EXPECT_NEAR(f3.z, 0.3f, kEps);
    EXPECT_NEAR(f3.w, -0.4f, kEps);
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

    Vector4 a2(-7.0f, 0.0f, 42.0f, -0.25f);
    Vector4 b2(a2);
    Float4 f2 = b2.GetFloat4();
    EXPECT_NEAR(f2.x, -7.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 42.0f, kEps);
    EXPECT_NEAR(f2.w, -0.25f, kEps);
}

TEST(Vector4Test, CrossDimensionConstruction)
{
    Vector3 v3(1.0f, 2.0f, 3.0f);
    Vector4 v4(v3);
    Float4 f = v4.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);

    Vector3 v3b(-5.0f, 10.0f, -15.0f);
    Vector4 v4b(v3b);
    Float4 f2 = v4b.GetFloat4();
    EXPECT_NEAR(f2.x, -5.0f, kEps);
    EXPECT_NEAR(f2.y, 10.0f, kEps);
    EXPECT_NEAR(f2.z, -15.0f, kEps);
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

    Vector4 d(-1.0f, 0.0f, 100.0f, -50.0f);
    Vector4 e(-1.0f, 0.0f, 100.0f, -50.0f);
    Vector4 g(-1.0f, 0.0f, 100.0f, -50.1f);
    EXPECT_TRUE(d == e);
    EXPECT_TRUE(d != g);
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

    Vector4 d(-10.0f, 5.0f, -3.0f, 7.0f);
    Vector4 e(10.0f, -5.0f, 3.0f, -7.0f);
    Float4 f2 = (d + e).GetFloat4();
    EXPECT_NEAR(f2.x, 0.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, 0.0f, kEps);
    EXPECT_NEAR(f2.w, 0.0f, kEps);

    Vector4 g(0.1f, 0.2f, 0.3f, 0.4f);
    Vector4 h(0.9f, 0.8f, 0.7f, 0.6f);
    Float4 f3 = (g + h).GetFloat4();
    EXPECT_NEAR(f3.x, 1.0f, kEps);
    EXPECT_NEAR(f3.y, 1.0f, kEps);
    EXPECT_NEAR(f3.z, 1.0f, kEps);
    EXPECT_NEAR(f3.w, 1.0f, kEps);
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

    Vector4 d(-3.0f, -7.0f, 10.0f, 0.0f);
    Vector4 e(-1.0f, -2.0f, 4.0f, 5.0f);
    Float4 f2 = (d - e).GetFloat4();
    EXPECT_NEAR(f2.x, -2.0f, kEps);
    EXPECT_NEAR(f2.y, -5.0f, kEps);
    EXPECT_NEAR(f2.z, 6.0f, kEps);
    EXPECT_NEAR(f2.w, -5.0f, kEps);

    Vector4 g(50.0f, 50.0f, 50.0f, 50.0f);
    Float4 f3 = (g - g).GetFloat4();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
    EXPECT_NEAR(f3.w, 0.0f, kEps);
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

    Vector4 d(-4.0f, 5.0f, -6.0f, 7.0f);
    Float4 f2 = (d * -3.0f).GetFloat4();
    EXPECT_NEAR(f2.x, 12.0f, kEps);
    EXPECT_NEAR(f2.y, -15.0f, kEps);
    EXPECT_NEAR(f2.z, 18.0f, kEps);
    EXPECT_NEAR(f2.w, -21.0f, kEps);

    Float4 f3 = (0.5f * d).GetFloat4();
    EXPECT_NEAR(f3.x, -2.0f, kEps);
    EXPECT_NEAR(f3.y, 2.5f, kEps);
    EXPECT_NEAR(f3.z, -3.0f, kEps);
    EXPECT_NEAR(f3.w, 3.5f, kEps);
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

    Vector4 d(-3.0f, 4.0f, -5.0f, 6.0f);
    Vector4 e(2.0f, -3.0f, -2.0f, 0.5f);
    Float4 f2 = (d * e).GetFloat4();
    EXPECT_NEAR(f2.x, -6.0f, kEps);
    EXPECT_NEAR(f2.y, -12.0f, kEps);
    EXPECT_NEAR(f2.z, 10.0f, kEps);
    EXPECT_NEAR(f2.w, 3.0f, kEps);

    Vector4 g(0.5f, 0.25f, 0.125f, 2.0f);
    Vector4 h(4.0f, 8.0f, 16.0f, 0.5f);
    Float4 f3 = (g * h).GetFloat4();
    EXPECT_NEAR(f3.x, 2.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
    EXPECT_NEAR(f3.z, 2.0f, kEps);
    EXPECT_NEAR(f3.w, 1.0f, kEps);
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

    Vector4 c(-15.0f, 9.0f, -21.0f, 3.0f);
    Float4 f2 = (c / 3.0f).GetFloat4();
    EXPECT_NEAR(f2.x, -5.0f, kEps);
    EXPECT_NEAR(f2.y, 3.0f, kEps);
    EXPECT_NEAR(f2.z, -7.0f, kEps);
    EXPECT_NEAR(f2.w, 1.0f, kEps);

    Vector4 d(7.0f, -14.0f, 28.0f, -35.0f);
    Float4 f3 = (d / -7.0f).GetFloat4();
    EXPECT_NEAR(f3.x, -1.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
    EXPECT_NEAR(f3.z, -4.0f, kEps);
    EXPECT_NEAR(f3.w, 5.0f, kEps);
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

    Vector4 d(-12.0f, 15.0f, -30.0f, 6.0f);
    Vector4 e(4.0f, -3.0f, 6.0f, -2.0f);
    Float4 f2 = (d / e).GetFloat4();
    EXPECT_NEAR(f2.x, -3.0f, kEps);
    EXPECT_NEAR(f2.y, -5.0f, kEps);
    EXPECT_NEAR(f2.z, -5.0f, kEps);
    EXPECT_NEAR(f2.w, -3.0f, kEps);

    Vector4 g(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 h(0.5f, 0.25f, 0.125f, 4.0f);
    Float4 f3 = (g / h).GetFloat4();
    EXPECT_NEAR(f3.x, 2.0f, kEps);
    EXPECT_NEAR(f3.y, 4.0f, kEps);
    EXPECT_NEAR(f3.z, 8.0f, kEps);
    EXPECT_NEAR(f3.w, 0.25f, kEps);
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

    Vector4 c(-100.0f, 0.0f, 50.5f, -0.25f);
    Float4 f2 = (-c).GetFloat4();
    EXPECT_NEAR(f2.x, 100.0f, kEps);
    EXPECT_NEAR(f2.y, 0.0f, kEps);
    EXPECT_NEAR(f2.z, -50.5f, kEps);
    EXPECT_NEAR(f2.w, 0.25f, kEps);

    Vector4 d(0.0f, 0.0f, 0.0f, 0.0f);
    Float4 f3 = (-d).GetFloat4();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
    EXPECT_NEAR(f3.w, 0.0f, kEps);
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

    // Additional compound chain with different values
    Vector4 b(-5.0f, 10.0f, -15.0f, 20.0f);
    b += Vector4(5.0f, -10.0f, 15.0f, -20.0f);
    Float4 g1 = b.GetFloat4();
    EXPECT_NEAR(g1.x, 0.0f, kEps);
    EXPECT_NEAR(g1.y, 0.0f, kEps);
    EXPECT_NEAR(g1.z, 0.0f, kEps);
    EXPECT_NEAR(g1.w, 0.0f, kEps);

    Vector4 c(2.0f, -4.0f, 8.0f, -16.0f);
    c *= -0.5f;
    Float4 g2 = c.GetFloat4();
    EXPECT_NEAR(g2.x, -1.0f, kEps);
    EXPECT_NEAR(g2.y, 2.0f, kEps);
    EXPECT_NEAR(g2.z, -4.0f, kEps);
    EXPECT_NEAR(g2.w, 8.0f, kEps);
}

TEST(Vector4Test, Length)
{
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    // sqrt(1+4+4+16) = sqrt(25) = 5
    EXPECT_NEAR(v.Length(), 5.0f, kEps);

    // sqrt(2^2 + 3^2 + 6^2 + 0^2) = sqrt(49) = 7
    Vector4 v2(2.0f, 3.0f, 6.0f, 0.0f);
    EXPECT_NEAR(v2.Length(), 7.0f, kEps);

    // sqrt(1+1+1+1) = 2
    Vector4 v3(1.0f, 1.0f, 1.0f, 1.0f);
    EXPECT_NEAR(v3.Length(), 2.0f, kEps);
}

TEST(Vector4Test, SquareLength)
{
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    EXPECT_NEAR(v.SquareLength(), 25.0f, kEps);

    Vector4 v2(2.0f, 3.0f, 6.0f, 0.0f);
    EXPECT_NEAR(v2.SquareLength(), 49.0f, kEps);

    Vector4 v3(-1.0f, -2.0f, -3.0f, -4.0f);
    EXPECT_NEAR(v3.SquareLength(), 30.0f, kEps);
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

    Vector4 v2(2.0f, 3.0f, 6.0f, 0.0f);
    Float4 f2 = v2.LengthV().GetFloat4();
    EXPECT_NEAR(f2.x, 7.0f, kEps);
    EXPECT_NEAR(f2.y, 7.0f, kEps);
    EXPECT_NEAR(f2.z, 7.0f, kEps);
    EXPECT_NEAR(f2.w, 7.0f, kEps);
}

TEST(Vector4Test, Normalize)
{
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    v.Normalize();
    EXPECT_NEAR(v.Length(), 1.0f, kEps);

    // (2, 3, 6, 0) -> len=7
    Vector4 v2(2.0f, 3.0f, 6.0f, 0.0f);
    v2.Normalize();
    EXPECT_NEAR(v2.Length(), 1.0f, kEps);
    Float4 f2 = v2.GetFloat4();
    EXPECT_NEAR(f2.x, 2.0f / 7.0f, kEps);
    EXPECT_NEAR(f2.y, 3.0f / 7.0f, kEps);
    EXPECT_NEAR(f2.z, 6.0f / 7.0f, kEps);
    EXPECT_NEAR(f2.w, 0.0f, kEps);
}

TEST(Vector4Test, Normalized)
{
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    Vector4 n = v.Normalized();
    EXPECT_NEAR(n.Length(), 1.0f, kEps);
    // Original unchanged
    EXPECT_NEAR(v.Length(), 5.0f, kEps);

    Vector4 v2(2.0f, 3.0f, 6.0f, 0.0f);
    Vector4 n2 = v2.Normalized();
    EXPECT_NEAR(n2.Length(), 1.0f, kEps);
    EXPECT_NEAR(v2.Length(), 7.0f, kEps);
}

TEST(Vector4Test, DotProduct)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    // 1*5 + 2*6 + 3*7 + 4*8 = 5+12+21+32 = 70
    EXPECT_NEAR(a.Dot(b), 70.0f, kEps);
    EXPECT_NEAR(Vector4::Dot(a, b), 70.0f, kEps);

    // (-2)*3 + 5*(-1) + 4*2 + (-3)*7 = -6-5+8-21 = -24
    Vector4 c(-2.0f, 5.0f, 4.0f, -3.0f);
    Vector4 d(3.0f, -1.0f, 2.0f, 7.0f);
    EXPECT_NEAR(c.Dot(d), -24.0f, kEps);
    EXPECT_NEAR(Vector4::Dot(c, d), -24.0f, kEps);

    // Perpendicular
    Vector4 e(1.0f, 0.0f, 0.0f, 0.0f);
    Vector4 g(0.0f, 1.0f, 0.0f, 0.0f);
    EXPECT_NEAR(e.Dot(g), 0.0f, kEps);
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

    Vector4 c(-2.0f, 5.0f, 4.0f, -3.0f);
    Vector4 d(3.0f, -1.0f, 2.0f, 7.0f);
    Float4 f2 = c.DotV(d).GetFloat4();
    EXPECT_NEAR(f2.x, -24.0f, kEps);
    EXPECT_NEAR(f2.y, -24.0f, kEps);
    EXPECT_NEAR(f2.z, -24.0f, kEps);
    EXPECT_NEAR(f2.w, -24.0f, kEps);
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

    Vector4 c(-10.0f, 20.0f, -30.0f, 40.0f);
    Vector4 d(0.5f, -0.5f, 0.25f, -0.25f);
    Vector4::Swap(c, d);
    Float4 fc = c.GetFloat4();
    Float4 fd = d.GetFloat4();
    EXPECT_NEAR(fc.x, 0.5f, kEps);
    EXPECT_NEAR(fc.y, -0.5f, kEps);
    EXPECT_NEAR(fc.z, 0.25f, kEps);
    EXPECT_NEAR(fc.w, -0.25f, kEps);
    EXPECT_NEAR(fd.x, -10.0f, kEps);
    EXPECT_NEAR(fd.y, 20.0f, kEps);
    EXPECT_NEAR(fd.z, -30.0f, kEps);
    EXPECT_NEAR(fd.w, 40.0f, kEps);
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

    Vector4 b;
    b = -42.0f;
    Float4 f2 = b.GetFloat4();
    EXPECT_NEAR(f2.x, -42.0f, kEps);
    EXPECT_NEAR(f2.y, -42.0f, kEps);
    EXPECT_NEAR(f2.z, -42.0f, kEps);
    EXPECT_NEAR(f2.w, -42.0f, kEps);

    Vector4 c;
    c = 0.0f;
    Float4 f3 = c.GetFloat4();
    EXPECT_NEAR(f3.x, 0.0f, kEps);
    EXPECT_NEAR(f3.y, 0.0f, kEps);
    EXPECT_NEAR(f3.z, 0.0f, kEps);
    EXPECT_NEAR(f3.w, 0.0f, kEps);
}

TEST(Vector4Test, GetFloat2FromVector4)
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Float2 f = v.GetFloat2();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);

    Vector4 v2(-50.0f, 0.25f, 100.0f, -7.0f);
    Float2 f2 = v2.GetFloat2();
    EXPECT_NEAR(f2.x, -50.0f, kEps);
    EXPECT_NEAR(f2.y, 0.25f, kEps);
}

TEST(Vector4Test, GetFloat3FromVector4)
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Float3 f = v.GetFloat3();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);

    Vector4 v2(-50.0f, 0.25f, 100.0f, -7.0f);
    Float3 f2 = v2.GetFloat3();
    EXPECT_NEAR(f2.x, -50.0f, kEps);
    EXPECT_NEAR(f2.y, 0.25f, kEps);
    EXPECT_NEAR(f2.z, 100.0f, kEps);
}

// ===== Complex Vector3 Tests =====

TEST(Vector3Test, GramSchmidtOrthogonalization)
{
    // Given two non-parallel vectors, produce an orthonormal pair using
    // cross products and normalization (Gram-Schmidt-like procedure).
    Vector3 u(1.0f, 2.0f, 3.0f);
    Vector3 v(0.0f, 1.0f, 0.0f);

    // e1 = normalize(u)
    Vector3 e1 = u.Normalized();

    // e2 = normalize(v - (v . e1) * e1)   [subtract projection of v onto e1]
    float projScalar = v.Dot(e1);
    Vector3 vParallel = e1 * projScalar;
    Vector3 vPerp = v - vParallel;
    Vector3 e2 = vPerp.Normalized();

    // e3 = e1 x e2
    Vector3 e3 = Vector3::Cross(e1, e2);

    // Verify all three are unit length
    EXPECT_NEAR(e1.Length(), 1.0f, kEps);
    EXPECT_NEAR(e2.Length(), 1.0f, kEps);
    EXPECT_NEAR(e3.Length(), 1.0f, kEps);

    // Verify mutual orthogonality
    EXPECT_NEAR(e1.Dot(e2), 0.0f, kEps);
    EXPECT_NEAR(e1.Dot(e3), 0.0f, kEps);
    EXPECT_NEAR(e2.Dot(e3), 0.0f, kEps);
}

TEST(Vector3Test, CrossProductAnticommutativity)
{
    // a x b = -(b x a)
    Vector3 a(3.0f, -1.0f, 4.0f);
    Vector3 b(-2.0f, 5.0f, 7.0f);

    Vector3 axb = Vector3::Cross(a, b);
    Vector3 bxa = Vector3::Cross(b, a);
    Vector3 neg_bxa = -bxa;

    Float3 f1 = axb.GetFloat3();
    Float3 f2 = neg_bxa.GetFloat3();
    EXPECT_NEAR(f1.x, f2.x, kEps);
    EXPECT_NEAR(f1.y, f2.y, kEps);
    EXPECT_NEAR(f1.z, f2.z, kEps);
}

TEST(Vector3Test, ScalarTripleProduct)
{
    // a . (b x c) = b . (c x a) = c . (a x b)  (cyclic property)
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, -1.0f, 2.0f);
    Vector3 c(-3.0f, 0.0f, 5.0f);

    float stp1 = a.Dot(Vector3::Cross(b, c));
    float stp2 = b.Dot(Vector3::Cross(c, a));
    float stp3 = c.Dot(Vector3::Cross(a, b));

    EXPECT_NEAR(stp1, stp2, kEps);
    EXPECT_NEAR(stp2, stp3, kEps);

    // Verify the actual value: a . (b x c)
    // b x c = (-1*5 - 2*0, 2*(-3) - 4*5, 4*0 - (-1)*(-3)) = (-5, -26, -3)
    // a . (-5, -26, -3) = 1*(-5) + 2*(-26) + 3*(-3) = -5 - 52 - 9 = -66
    EXPECT_NEAR(stp1, -66.0f, kEps);
}

TEST(Vector3Test, VectorProjectionAndRejection)
{
    // proj_b(a) + rej_b(a) = a
    Vector3 a(3.0f, 4.0f, 0.0f);
    Vector3 b(1.0f, 0.0f, 0.0f);

    // proj_b(a) = (a . b / b . b) * b
    float dotAB = a.Dot(b);
    float dotBB = b.Dot(b);
    Vector3 proj = b * (dotAB / dotBB);

    // rej_b(a) = a - proj_b(a)
    Vector3 rej = a - proj;

    // Projection should be parallel to b: (3, 0, 0)
    Float3 fp = proj.GetFloat3();
    EXPECT_NEAR(fp.x, 3.0f, kEps);
    EXPECT_NEAR(fp.y, 0.0f, kEps);
    EXPECT_NEAR(fp.z, 0.0f, kEps);

    // Rejection should be perpendicular to b: (0, 4, 0)
    Float3 fr = rej.GetFloat3();
    EXPECT_NEAR(fr.x, 0.0f, kEps);
    EXPECT_NEAR(fr.y, 4.0f, kEps);
    EXPECT_NEAR(fr.z, 0.0f, kEps);

    // proj + rej = a
    Vector3 sum = proj + rej;
    Float3 fs = sum.GetFloat3();
    Float3 fa = a.GetFloat3();
    EXPECT_NEAR(fs.x, fa.x, kEps);
    EXPECT_NEAR(fs.y, fa.y, kEps);
    EXPECT_NEAR(fs.z, fa.z, kEps);

    // proj . rej = 0 (orthogonal)
    EXPECT_NEAR(proj.Dot(rej), 0.0f, kEps);
}

TEST(Vector3Test, LagrangeIdentity)
{
    // |a x b|^2 = |a|^2 * |b|^2 - (a . b)^2
    Vector3 a(2.0f, -3.0f, 1.0f);
    Vector3 b(1.0f, 4.0f, -2.0f);

    Vector3 cross = Vector3::Cross(a, b);
    float crossSqLen = cross.SquareLength();
    float aSqLen = a.SquareLength();
    float bSqLen = b.SquareLength();
    float dotAB = a.Dot(b);

    EXPECT_NEAR(crossSqLen, aSqLen * bSqLen - dotAB * dotAB, kEps);
}
