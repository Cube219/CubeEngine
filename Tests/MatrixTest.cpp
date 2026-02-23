#include <gtest/gtest.h>

#include "Matrix.h"

using namespace cube;

constexpr float kEps = 1e-5f;

// Helper to compare two matrices element-wise
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

// ===== Construction =====

TEST(MatrixTest, DefaultConstruction)
{
    Matrix m;
    // Default construction - just verify no crash
    (void)m;
}

TEST(MatrixTest, IdentityMatrix)
{
    Matrix m = Matrix::Identity();
    float expected[4][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };
    ExpectMatrixNear(m, expected);
}

TEST(MatrixTest, ZeroMatrix)
{
    Matrix m = Matrix::Zero();
    float expected[4][4] = {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    };
    ExpectMatrixNear(m, expected);
}

TEST(MatrixTest, ConstructFromRows)
{
    Vector4 r0(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 r1(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4 r2(9.0f, 10.0f, 11.0f, 12.0f);
    Vector4 r3(13.0f, 14.0f, 15.0f, 16.0f);
    Matrix m(r0, r1, r2, r3);

    float expected[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    ExpectMatrixNear(m, expected);

    Vector4 s0(-1.0f, 0.5f, 3.0f, -2.0f);
    Vector4 s1(0.0f, 7.0f, -4.0f, 1.0f);
    Vector4 s2(10.0f, -10.0f, 0.0f, 5.0f);
    Vector4 s3(-3.0f, 2.0f, 8.0f, -6.0f);
    Matrix m2(s0, s1, s2, s3);

    float expected2[4][4] = {
        {-1, 0.5f, 3, -2},
        {0, 7, -4, 1},
        {10, -10, 0, 5},
        {-3, 2, 8, -6}
    };
    ExpectMatrixNear(m2, expected2);
}

TEST(MatrixTest, ConstructFromArray)
{
    float arr[16] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    };
    Matrix m(arr);

    float expected[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    ExpectMatrixNear(m, expected);

    float arr2[16] = {
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    };
    Matrix m2(arr2);
    float expected2[4][4] = {
        {-1, 0.5f, 3, -2},
        {0, 7, -4, 1},
        {10, -10, 0, 5},
        {-3, 2, 8, -6}
    };
    ExpectMatrixNear(m2, expected2);
}

TEST(MatrixTest, ConstructFrom16Floats)
{
    Matrix m(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );

    float expected[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    ExpectMatrixNear(m, expected);

    Matrix m2(
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    );
    float expected2[4][4] = {
        {-1, 0.5f, 3, -2},
        {0, 7, -4, 1},
        {10, -10, 0, 5},
        {-3, 2, 8, -6}
    };
    ExpectMatrixNear(m2, expected2);
}

TEST(MatrixTest, CopyConstruction)
{
    Matrix a(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix b(a);
    ExpectMatrixNear(a, b);

    Matrix c(
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    );
    Matrix d(c);
    ExpectMatrixNear(c, d);
}

TEST(MatrixTest, CopyAssignment)
{
    Matrix a(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix b;
    b = a;
    ExpectMatrixNear(a, b);

    Matrix c(
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    );
    Matrix d;
    d = c;
    ExpectMatrixNear(c, d);
}

// ===== Element Access =====

TEST(MatrixTest, OperatorBracket)
{
    Matrix m(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );

    Float4 r0 = m[0].GetFloat4();
    EXPECT_NEAR(r0.x, 1.0f, kEps);
    EXPECT_NEAR(r0.y, 2.0f, kEps);

    Float4 r2 = m[2].GetFloat4();
    EXPECT_NEAR(r2.x, 9.0f, kEps);
    EXPECT_NEAR(r2.w, 12.0f, kEps);

    Float4 r3 = m[3].GetFloat4();
    EXPECT_NEAR(r3.x, 13.0f, kEps);
    EXPECT_NEAR(r3.y, 14.0f, kEps);
    EXPECT_NEAR(r3.z, 15.0f, kEps);
    EXPECT_NEAR(r3.w, 16.0f, kEps);

    Matrix m2(
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    );
    Float4 s1 = m2[1].GetFloat4();
    EXPECT_NEAR(s1.x, 0.0f, kEps);
    EXPECT_NEAR(s1.y, 7.0f, kEps);
    EXPECT_NEAR(s1.z, -4.0f, kEps);
    EXPECT_NEAR(s1.w, 1.0f, kEps);
}

TEST(MatrixTest, GetRow)
{
    Matrix m(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );

    Vector4 row1 = m.GetRow(1);
    Float4 f = row1.GetFloat4();
    EXPECT_NEAR(f.x, 5.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
    EXPECT_NEAR(f.z, 7.0f, kEps);
    EXPECT_NEAR(f.w, 8.0f, kEps);

    Vector4 row3 = m.GetRow(3);
    Float4 f2 = row3.GetFloat4();
    EXPECT_NEAR(f2.x, 13.0f, kEps);
    EXPECT_NEAR(f2.y, 14.0f, kEps);
    EXPECT_NEAR(f2.z, 15.0f, kEps);
    EXPECT_NEAR(f2.w, 16.0f, kEps);

    Vector4 row0 = m.GetRow(0);
    Float4 f3 = row0.GetFloat4();
    EXPECT_NEAR(f3.x, 1.0f, kEps);
    EXPECT_NEAR(f3.y, 2.0f, kEps);
    EXPECT_NEAR(f3.z, 3.0f, kEps);
    EXPECT_NEAR(f3.w, 4.0f, kEps);
}

TEST(MatrixTest, GetCol)
{
    Matrix m(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );

    Vector4 col1 = m.GetCol(1);
    Float4 f = col1.GetFloat4();
    EXPECT_NEAR(f.x, 2.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
    EXPECT_NEAR(f.z, 10.0f, kEps);
    EXPECT_NEAR(f.w, 14.0f, kEps);

    Vector4 col0 = m.GetCol(0);
    Float4 f2 = col0.GetFloat4();
    EXPECT_NEAR(f2.x, 1.0f, kEps);
    EXPECT_NEAR(f2.y, 5.0f, kEps);
    EXPECT_NEAR(f2.z, 9.0f, kEps);
    EXPECT_NEAR(f2.w, 13.0f, kEps);

    Vector4 col3 = m.GetCol(3);
    Float4 f3 = col3.GetFloat4();
    EXPECT_NEAR(f3.x, 4.0f, kEps);
    EXPECT_NEAR(f3.y, 8.0f, kEps);
    EXPECT_NEAR(f3.z, 12.0f, kEps);
    EXPECT_NEAR(f3.w, 16.0f, kEps);
}

TEST(MatrixTest, SetRow)
{
    Matrix m = Matrix::Zero();
    m.SetRow(2, Vector4(9.0f, 10.0f, 11.0f, 12.0f));
    Float4 f = m.GetRow(2).GetFloat4();
    EXPECT_NEAR(f.x, 9.0f, kEps);
    EXPECT_NEAR(f.y, 10.0f, kEps);
    EXPECT_NEAR(f.z, 11.0f, kEps);
    EXPECT_NEAR(f.w, 12.0f, kEps);

    m.SetRow(0, Vector4(-1.0f, 0.5f, 3.0f, -2.0f));
    Float4 f2 = m.GetRow(0).GetFloat4();
    EXPECT_NEAR(f2.x, -1.0f, kEps);
    EXPECT_NEAR(f2.y, 0.5f, kEps);
    EXPECT_NEAR(f2.z, 3.0f, kEps);
    EXPECT_NEAR(f2.w, -2.0f, kEps);

    m.SetRow(3, Vector4(100.0f, -200.0f, 300.0f, -400.0f));
    Float4 f3 = m.GetRow(3).GetFloat4();
    EXPECT_NEAR(f3.x, 100.0f, kEps);
    EXPECT_NEAR(f3.y, -200.0f, kEps);
    EXPECT_NEAR(f3.z, 300.0f, kEps);
    EXPECT_NEAR(f3.w, -400.0f, kEps);
}

TEST(MatrixTest, SetCol)
{
    Matrix m = Matrix::Zero();
    m.SetCol(1, Vector4(2.0f, 6.0f, 10.0f, 14.0f));
    Vector4 col = m.GetCol(1);
    Float4 f = col.GetFloat4();
    EXPECT_NEAR(f.x, 2.0f, kEps);
    EXPECT_NEAR(f.y, 6.0f, kEps);
    EXPECT_NEAR(f.z, 10.0f, kEps);
    EXPECT_NEAR(f.w, 14.0f, kEps);

    m.SetCol(3, Vector4(-1.0f, -2.0f, -3.0f, -4.0f));
    Float4 f2 = m.GetCol(3).GetFloat4();
    EXPECT_NEAR(f2.x, -1.0f, kEps);
    EXPECT_NEAR(f2.y, -2.0f, kEps);
    EXPECT_NEAR(f2.z, -3.0f, kEps);
    EXPECT_NEAR(f2.w, -4.0f, kEps);

    m.SetCol(0, Vector4(100.0f, 200.0f, 300.0f, 400.0f));
    Float4 f3 = m.GetCol(0).GetFloat4();
    EXPECT_NEAR(f3.x, 100.0f, kEps);
    EXPECT_NEAR(f3.y, 200.0f, kEps);
    EXPECT_NEAR(f3.z, 300.0f, kEps);
    EXPECT_NEAR(f3.w, 400.0f, kEps);
}

// ===== Arithmetic =====

TEST(MatrixTest, Addition)
{
    Matrix a(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix b(
        16, 15, 14, 13,
        12, 11, 10, 9,
        8, 7, 6, 5,
        4, 3, 2, 1
    );
    Matrix c = a + b;
    float expected[4][4] = {
        {17, 17, 17, 17},
        {17, 17, 17, 17},
        {17, 17, 17, 17},
        {17, 17, 17, 17}
    };
    ExpectMatrixNear(c, expected);

    Matrix d(
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    );
    Matrix e(
        1, -0.5f, -3, 2,
        0, -7, 4, -1,
        -10, 10, 0, -5,
        3, -2, -8, 6
    );
    Matrix f = d + e;
    ExpectMatrixNear(f, Matrix::Zero());
}

TEST(MatrixTest, Subtraction)
{
    Matrix a(
        10, 20, 30, 40,
        50, 60, 70, 80,
        90, 100, 110, 120,
        130, 140, 150, 160
    );
    Matrix b(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix c = a - b;
    float expected[4][4] = {
        {9, 18, 27, 36},
        {45, 54, 63, 72},
        {81, 90, 99, 108},
        {117, 126, 135, 144}
    };
    ExpectMatrixNear(c, expected);

    Matrix d(
        5, -3, 7, 2,
        -1, 4, 0, 8,
        6, -2, 9, -5,
        3, 1, -4, 7
    );
    Matrix e = d - d;
    ExpectMatrixNear(e, Matrix::Zero());

    Matrix f(
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    );
    Matrix g(
        1, -0.5f, -3, 2,
        0, -7, 4, -1,
        -10, 10, 0, -5,
        3, -2, -8, 6
    );
    Matrix h = f - g;
    float expected2[4][4] = {
        {-2, 1, 6, -4},
        {0, 14, -8, 2},
        {20, -20, 0, 10},
        {-6, 4, 16, -12}
    };
    ExpectMatrixNear(h, expected2);
}

TEST(MatrixTest, ScalarMultiply)
{
    Matrix a(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix b = a * 2.0f;
    float expected[4][4] = {
        {2, 4, 6, 8},
        {10, 12, 14, 16},
        {18, 20, 22, 24},
        {26, 28, 30, 32}
    };
    ExpectMatrixNear(b, expected);

    Matrix c = a * -1.0f;
    float expected2[4][4] = {
        {-1, -2, -3, -4},
        {-5, -6, -7, -8},
        {-9, -10, -11, -12},
        {-13, -14, -15, -16}
    };
    ExpectMatrixNear(c, expected2);

    Matrix d = a * 0.5f;
    float expected3[4][4] = {
        {0.5f, 1, 1.5f, 2},
        {2.5f, 3, 3.5f, 4},
        {4.5f, 5, 5.5f, 6},
        {6.5f, 7, 7.5f, 8}
    };
    ExpectMatrixNear(d, expected3);
}

TEST(MatrixTest, ScalarMultiplyLhs)
{
    Matrix a(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix b = a * 2.0f;
    float expected[4][4] = {
        {2, 4, 6, 8},
        {10, 12, 14, 16},
        {18, 20, 22, 24},
        {26, 28, 30, 32}
    };
    ExpectMatrixNear(b, expected);

    Matrix c = -0.5f * a;
    float expected2[4][4] = {
        {-0.5f, -1, -1.5f, -2},
        {-2.5f, -3, -3.5f, -4},
        {-4.5f, -5, -5.5f, -6},
        {-6.5f, -7, -7.5f, -8}
    };
    ExpectMatrixNear(c, expected2);
}

TEST(MatrixTest, ScalarDivide)
{
    Matrix a(
        2, 4, 6, 8,
        10, 12, 14, 16,
        18, 20, 22, 24,
        26, 28, 30, 32
    );
    Matrix b = a / 2.0f;
    float expected[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    ExpectMatrixNear(b, expected);

    Matrix c(
        -9, 15, -21, 3,
        6, -12, 18, -24,
        30, 0, -27, 33,
        -6, 9, -3, 12
    );
    Matrix d = c / 3.0f;
    float expected2[4][4] = {
        {-3, 5, -7, 1},
        {2, -4, 6, -8},
        {10, 0, -9, 11},
        {-2, 3, -1, 4}
    };
    ExpectMatrixNear(d, expected2);
}

TEST(MatrixTest, MatrixMultiply)
{
    Matrix a = Matrix::Identity();
    Matrix b(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    // Identity * B = B
    Matrix c = a * b;
    ExpectMatrixNear(c, b);

    // B * Identity = B
    Matrix d = b * a;
    ExpectMatrixNear(d, b);

    // Zero * B = Zero
    Matrix z = Matrix::Zero() * b;
    ExpectMatrixNear(z, Matrix::Zero());
}

TEST(MatrixTest, MatrixMultiplyNonTrivial)
{
    Matrix a(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix b(
        17, 18, 19, 20,
        21, 22, 23, 24,
        25, 26, 27, 28,
        29, 30, 31, 32
    );
    Matrix c = a * b;
    float expected[4][4] = {
        {250, 260, 270, 280},
        {618, 644, 670, 696},
        {986, 1028, 1070, 1112},
        {1354, 1412, 1470, 1528}
    };
    ExpectMatrixNear(c, expected);

    // Another non-trivial product with different values
    Matrix d(
        2, 0, 1, 0,
        0, 3, 0, 1,
        1, 0, 2, 0,
        0, 1, 0, 3
    );
    Matrix e(
        1, 2, 0, 0,
        0, 1, 2, 0,
        0, 0, 1, 2,
        2, 0, 0, 1
    );
    Matrix f = d * e;
    // Row0: 2*1+0+1*0+0, 2*2+0+1*0+0, 0+0+1*1+0, 0+0+1*2+0 = 2,4,1,2
    // Row1: 0+0+0+2, 0+3+0+0, 0+6+0+0, 0+0+0+1 = 2,3,6,1
    // Row2: 1+0+0+0, 2+0+0+0, 0+0+2+0, 0+0+4+0 = 1,2,2,4
    // Row3: 0+0+0+6, 0+1+0+0, 0+2+0+0, 0+0+0+3 = 6,1,2,3
    float expected2[4][4] = {
        {2, 4, 1, 2},
        {2, 3, 6, 1},
        {1, 2, 2, 4},
        {6, 1, 2, 3}
    };
    ExpectMatrixNear(f, expected2);
}

// ===== Compound Assignment =====

TEST(MatrixTest, CompoundAddition)
{
    Matrix a = Matrix::Identity();
    Matrix b = Matrix::Identity();
    a += b;
    float expected[4][4] = {
        {2, 0, 0, 0},
        {0, 2, 0, 0},
        {0, 0, 2, 0},
        {0, 0, 0, 2}
    };
    ExpectMatrixNear(a, expected);

    Matrix c(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix d(
        -1, -2, -3, -4,
        -5, -6, -7, -8,
        -9, -10, -11, -12,
        -13, -14, -15, -16
    );
    c += d;
    ExpectMatrixNear(c, Matrix::Zero());
}

TEST(MatrixTest, CompoundSubtraction)
{
    Matrix a = Matrix::Identity();
    Matrix b = Matrix::Identity();
    a -= b;
    ExpectMatrixNear(a, Matrix::Zero());

    Matrix c(
        10, 20, 30, 40,
        50, 60, 70, 80,
        90, 100, 110, 120,
        130, 140, 150, 160
    );
    Matrix d(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    c -= d;
    float expected[4][4] = {
        {9, 18, 27, 36},
        {45, 54, 63, 72},
        {81, 90, 99, 108},
        {117, 126, 135, 144}
    };
    ExpectMatrixNear(c, expected);
}

TEST(MatrixTest, CompoundScalarMultiply)
{
    Matrix a = Matrix::Identity();
    a *= 3.0f;
    float expected[4][4] = {
        {3, 0, 0, 0},
        {0, 3, 0, 0},
        {0, 0, 3, 0},
        {0, 0, 0, 3}
    };
    ExpectMatrixNear(a, expected);

    Matrix b(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    b *= -1.0f;
    float expected2[4][4] = {
        {-1, -2, -3, -4},
        {-5, -6, -7, -8},
        {-9, -10, -11, -12},
        {-13, -14, -15, -16}
    };
    ExpectMatrixNear(b, expected2);
}

TEST(MatrixTest, CompoundMatrixMultiply)
{
    Matrix a(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix b = Matrix::Identity();
    a *= b;
    float expected[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    ExpectMatrixNear(a, expected);

    Matrix c(
        2, 0, 1, 0,
        0, 3, 0, 1,
        1, 0, 2, 0,
        0, 1, 0, 3
    );
    Matrix d(
        1, 0, 0, 2,
        0, 1, 2, 0,
        0, 2, 1, 0,
        2, 0, 0, 1
    );
    Matrix cd = c * d;
    c *= d;
    ExpectMatrixNear(c, cd);
}

TEST(MatrixTest, CompoundScalarDivide)
{
    Matrix a(
        2, 4, 6, 8,
        10, 12, 14, 16,
        18, 20, 22, 24,
        26, 28, 30, 32
    );
    a /= 2.0f;
    float expected[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    ExpectMatrixNear(a, expected);

    Matrix b(
        -9, 15, -21, 3,
        6, -12, 18, -24,
        30, 0, -27, 33,
        -6, 9, -3, 12
    );
    b /= -3.0f;
    float expected2[4][4] = {
        {3, -5, 7, -1},
        {-2, 4, -6, 8},
        {-10, 0, 9, -11},
        {2, -3, 1, -4}
    };
    ExpectMatrixNear(b, expected2);
}

// ===== Transpose =====

TEST(MatrixTest, Transpose)
{
    Matrix m(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    m.Transpose();
    float expected[4][4] = {
        {1, 5, 9, 13},
        {2, 6, 10, 14},
        {3, 7, 11, 15},
        {4, 8, 12, 16}
    };
    ExpectMatrixNear(m, expected);

    Matrix m2(
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    );
    m2.Transpose();
    float expected2[4][4] = {
        {-1, 0, 10, -3},
        {0.5f, 7, -10, 2},
        {3, -4, 0, 8},
        {-2, 1, 5, -6}
    };
    ExpectMatrixNear(m2, expected2);
}

TEST(MatrixTest, Transposed)
{
    Matrix m(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix t = m.Transposed();
    float expected[4][4] = {
        {1, 5, 9, 13},
        {2, 6, 10, 14},
        {3, 7, 11, 15},
        {4, 8, 12, 16}
    };
    ExpectMatrixNear(t, expected);

    // Original unchanged
    float origExpected[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    ExpectMatrixNear(m, origExpected);

    Matrix m2(
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    );
    Matrix t2 = m2.Transposed();
    float expected2[4][4] = {
        {-1, 0, 10, -3},
        {0.5f, 7, -10, 2},
        {3, -4, 0, 8},
        {-2, 1, 5, -6}
    };
    ExpectMatrixNear(t2, expected2);
}

TEST(MatrixTest, TransposeIdentity)
{
    Matrix m = Matrix::Identity();
    Matrix t = m.Transposed();
    ExpectMatrixNear(t, Matrix::Identity());
}

TEST(MatrixTest, DoubleTranspose)
{
    Matrix m(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Matrix t = m.Transposed().Transposed();
    ExpectMatrixNear(t, m);

    Matrix m2(
        -1, 0.5f, 3, -2,
        0, 7, -4, 1,
        10, -10, 0, 5,
        -3, 2, 8, -6
    );
    Matrix t2 = m2.Transposed().Transposed();
    ExpectMatrixNear(t2, m2);
}

// ===== Inverse =====

TEST(MatrixTest, InverseIdentity)
{
    Matrix m = Matrix::Identity();
    Matrix inv = m.Inversed();
    ExpectMatrixNear(inv, Matrix::Identity());
}

TEST(MatrixTest, InverseTimesOriginal)
{
    // Use a known invertible matrix
    Matrix m(
        1, 0, 0, 0,
        0, 2, 0, 0,
        0, 0, 3, 0,
        0, 0, 0, 4
    );
    Matrix inv = m.Inversed();
    Matrix product = m * inv;
    ExpectMatrixNear(product, Matrix::Identity(), 1e-4f);

    // Another diagonal matrix
    Matrix m2(
        5, 0, 0, 0,
        0, 0.5f, 0, 0,
        0, 0, -2, 0,
        0, 0, 0, 10
    );
    Matrix inv2 = m2.Inversed();
    Matrix product2 = m2 * inv2;
    ExpectMatrixNear(product2, Matrix::Identity(), 1e-4f);

    // Verify actual inverse values for the diagonal
    float expected2[4][4] = {
        {0.2f, 0, 0, 0},
        {0, 2.0f, 0, 0},
        {0, 0, -0.5f, 0},
        {0, 0, 0, 0.1f}
    };
    ExpectMatrixNear(inv2, expected2, 1e-4f);
}

TEST(MatrixTest, InverseNonTrivial)
{
    Matrix m(
        2, 1, 1, 0,
        4, 3, 3, 1,
        8, 7, 9, 5,
        6, 7, 9, 8
    );
    Matrix inv = m.Inversed();
    Matrix product = m * inv;
    ExpectMatrixNear(product, Matrix::Identity(), 1e-4f);

    Matrix m2(
        1, 2, 3, 0,
        0, 1, 4, 0,
        5, 6, 0, 0,
        0, 0, 0, 1
    );
    Matrix inv2 = m2.Inversed();
    Matrix product2 = m2 * inv2;
    ExpectMatrixNear(product2, Matrix::Identity(), 1e-4f);

    Matrix m3(
        3, 0, 2, -1,
        1, 2, 0, -2,
        4, 0, 6, -3,
        5, 0, 2, 0
    );
    Matrix inv3 = m3.Inversed();
    Matrix product3 = m3 * inv3;
    ExpectMatrixNear(product3, Matrix::Identity(), 1e-4f);
}

TEST(MatrixTest, InverseMutating)
{
    Matrix m(
        2, 1, 1, 0,
        4, 3, 3, 1,
        8, 7, 9, 5,
        6, 7, 9, 8
    );
    Matrix original(m);
    m.Inverse();
    Matrix product = original * m;
    ExpectMatrixNear(product, Matrix::Identity(), 1e-4f);

    Matrix m2(
        3, 0, 2, -1,
        1, 2, 0, -2,
        4, 0, 6, -3,
        5, 0, 2, 0
    );
    Matrix original2(m2);
    m2.Inverse();
    Matrix product2 = original2 * m2;
    ExpectMatrixNear(product2, Matrix::Identity(), 1e-4f);
}

// ===== Vector * Matrix =====

TEST(MatrixTest, VectorTimesMatrix)
{
    Matrix m = Matrix::Identity();
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 result = v * m;
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 1.0f, kEps);
    EXPECT_NEAR(f.y, 2.0f, kEps);
    EXPECT_NEAR(f.z, 3.0f, kEps);
    EXPECT_NEAR(f.w, 4.0f, kEps);

    Vector4 v2(-5.0f, 10.0f, -15.0f, 20.0f);
    Vector4 result2 = v2 * m;
    Float4 f2 = result2.GetFloat4();
    EXPECT_NEAR(f2.x, -5.0f, kEps);
    EXPECT_NEAR(f2.y, 10.0f, kEps);
    EXPECT_NEAR(f2.z, -15.0f, kEps);
    EXPECT_NEAR(f2.w, 20.0f, kEps);
}

TEST(MatrixTest, VectorTimesMatrixNonTrivial)
{
    Matrix m(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 result = v * m;
    // result.x = 1*1 + 2*5 + 3*9 + 4*13 = 1+10+27+52 = 90
    // result.y = 1*2 + 2*6 + 3*10 + 4*14 = 2+12+30+56 = 100
    // result.z = 1*3 + 2*7 + 3*11 + 4*15 = 3+14+33+60 = 110
    // result.w = 1*4 + 2*8 + 3*12 + 4*16 = 4+16+36+64 = 120
    Float4 f = result.GetFloat4();
    EXPECT_NEAR(f.x, 90.0f, kEps);
    EXPECT_NEAR(f.y, 100.0f, kEps);
    EXPECT_NEAR(f.z, 110.0f, kEps);
    EXPECT_NEAR(f.w, 120.0f, kEps);

    // v2 = (-1, 0, 2, -3)
    // result.x = -1*1 + 0*5 + 2*9 + -3*13 = -1+0+18-39 = -22
    // result.y = -1*2 + 0*6 + 2*10 + -3*14 = -2+0+20-42 = -24
    // result.z = -1*3 + 0*7 + 2*11 + -3*15 = -3+0+22-45 = -26
    // result.w = -1*4 + 0*8 + 2*12 + -3*16 = -4+0+24-48 = -28
    Vector4 v2(-1.0f, 0.0f, 2.0f, -3.0f);
    Float4 f2 = (v2 * m).GetFloat4();
    EXPECT_NEAR(f2.x, -22.0f, kEps);
    EXPECT_NEAR(f2.y, -24.0f, kEps);
    EXPECT_NEAR(f2.z, -26.0f, kEps);
    EXPECT_NEAR(f2.w, -28.0f, kEps);

    // Diagonal matrix: v * diag = element-wise multiply
    Matrix diag(
        2, 0, 0, 0,
        0, 3, 0, 0,
        0, 0, 4, 0,
        0, 0, 0, 5
    );
    Vector4 v3(10.0f, 20.0f, 30.0f, 40.0f);
    Float4 f3 = (v3 * diag).GetFloat4();
    EXPECT_NEAR(f3.x, 20.0f, kEps);
    EXPECT_NEAR(f3.y, 60.0f, kEps);
    EXPECT_NEAR(f3.z, 120.0f, kEps);
    EXPECT_NEAR(f3.w, 200.0f, kEps);
}

// ===== Complex Algebraic Properties =====

TEST(MatrixTest, TransposeOfProduct)
{
    // (A * B)^T = B^T * A^T
    Matrix a(
        2, 1, 0, 3,
        1, 0, 2, 1,
        0, 3, 1, 2,
        1, 2, 3, 0
    );
    Matrix b(
        1, 0, 3, 2,
        0, 2, 1, 3,
        3, 1, 0, 2,
        2, 3, 2, 1
    );

    Matrix abT = (a * b).Transposed();
    Matrix bTaT = b.Transposed() * a.Transposed();
    ExpectMatrixNear(abT, bTaT);
}

TEST(MatrixTest, InverseOfProduct)
{
    // (A * B)^-1 = B^-1 * A^-1
    Matrix a(
        2, 1, 0, 3,
        1, 0, 2, 1,
        0, 3, 1, 2,
        1, 2, 3, 0
    );
    Matrix b(
        1, 0, 3, 2,
        0, 2, 1, 3,
        3, 1, 0, 2,
        2, 3, 2, 1
    );

    Matrix abInv = (a * b).Inversed();
    Matrix bInvAInv = b.Inversed() * a.Inversed();
    ExpectMatrixNear(abInv, bInvAInv, 1e-3f);
}

TEST(MatrixTest, MultiplyAssociativity)
{
    // (A * B) * C = A * (B * C)
    Matrix a(
        2, 1, 0, 3,
        1, 0, 2, 1,
        0, 3, 1, 2,
        1, 2, 3, 0
    );
    Matrix b(
        1, 0, 3, 2,
        0, 2, 1, 3,
        3, 1, 0, 2,
        2, 3, 2, 1
    );
    Matrix c(
        0, 1, 2, 3,
        3, 2, 1, 0,
        1, 3, 0, 2,
        2, 0, 3, 1
    );

    Matrix lhs = (a * b) * c;
    Matrix rhs = a * (b * c);
    ExpectMatrixNear(lhs, rhs);
}

TEST(MatrixTest, InverseOfTransposeEqualsTransposeOfInverse)
{
    // (A^T)^-1 = (A^-1)^T
    Matrix a(
        2, 1, 0, 3,
        1, 0, 2, 1,
        0, 3, 1, 2,
        1, 2, 3, 0
    );

    Matrix atInv = a.Transposed().Inversed();
    Matrix aInvT = a.Inversed().Transposed();
    ExpectMatrixNear(atInv, aInvT, 1e-4f);
}

TEST(MatrixTest, InverseNonTrivialWithKnownResult)
{
    Matrix a(
        2, 1, 0, 0,
        1, 1, 0, 0,
        0, 0, 3, 1,
        0, 0, 1, 1
    );
    Matrix inv = a.Inversed();

    float expected[4][4] = {
        { 1.0f, -1.0f,  0.0f,  0.0f},
        {-1.0f,  2.0f,  0.0f,  0.0f},
        { 0.0f,  0.0f,  0.5f, -0.5f},
        { 0.0f,  0.0f, -0.5f,  1.5f}
    };
    ExpectMatrixNear(inv, expected);
}
