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
    // Row 0: (1*17+2*21+3*25+4*29, 1*18+2*22+3*26+4*30, ...)
    // = (17+42+75+116, 18+44+78+120, 19+46+81+124, 20+48+84+128)
    // = (250, 260, 270, 280)
    float expected[4][4] = {
        {250, 260, 270, 280},
        {618, 644, 670, 696},
        {986, 1028, 1070, 1112},
        {1354, 1412, 1470, 1528}
    };
    ExpectMatrixNear(c, expected);
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
}

TEST(MatrixTest, CompoundSubtraction)
{
    Matrix a = Matrix::Identity();
    Matrix b = Matrix::Identity();
    a -= b;
    ExpectMatrixNear(a, Matrix::Zero());
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
}
