#define CUBE_MATRIX_IMPLEMENTATION

#include "../Matrix.h"

namespace cube
{
    inline Matrix Matrix::Zero()
    {
        return Matrix(
            Vector4::Zero(),
            Vector4::Zero(),
            Vector4::Zero(),
            Vector4::Zero());
    }

    inline Matrix Matrix::Identity()
    {
        return Matrix(
            Vector4(1.0f, 0.0f, 0.0f, 0.0f),
            Vector4(0.0f, 1.0f, 0.0f, 0.0f),
            Vector4(0.0f, 0.0f, 1.0f, 0.0f),
            Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    }

    inline Matrix::Matrix()
    {
    }

    inline Matrix::Matrix(const Vector4& row1, const Vector4& row2, const Vector4& row3, const Vector4& row4) :
        mRows{ row1, row2, row3, row4 }
    {
    }

    inline Matrix::Matrix(float v[16]) :
        mRows{
            Vector4(v[0], v[1], v[2], v[3]),
            Vector4(v[4], v[5], v[6], v[7]),
            Vector4(v[8], v[9], v[10], v[11]),
            Vector4(v[12], v[13], v[14], v[15])
        }
    {
    }

    inline Matrix::Matrix(float v11, float v12, float v13, float v14,
        float v21, float v22, float v23, float v24,
        float v31, float v32, float v33, float v34,
        float v41, float v42, float v43, float v44) :
        mRows{
            Vector4(v11, v12, v13, v14),
            Vector4(v21, v22, v23, v24),
            Vector4(v31, v32, v33, v34),
            Vector4(v41, v42, v43, v44)
        }
    {
    }

    inline Matrix::~Matrix()
    {
    }

    inline Matrix::Matrix(const Matrix& other) :
        mRows{
            other.mRows[0],
            other.mRows[1],
            other.mRows[2],
            other.mRows[3]
        }
    {
    }

    inline Matrix& Matrix::operator=(const Matrix& rhs)
    {
        mRows[0] = rhs.mRows[0];
        mRows[1] = rhs.mRows[1];
        mRows[2] = rhs.mRows[2];
        mRows[3] = rhs.mRows[3];

        return *this;
    }

    inline Vector4& Matrix::operator[](int i)
    {
        return mRows[i];
    }

    inline Matrix Matrix::operator+(const Matrix& rhs) const
    {
        Matrix res(*this);
        res += rhs;

        return res;
    }

    inline Matrix Matrix::operator-(const Matrix& rhs) const
    {
        Matrix res(*this);
        res -= rhs;

        return res;
    }

    inline Matrix Matrix::operator*(float rhs) const
    {
        Matrix res(*this);
        res *= rhs;

        return res;
    }

    inline Matrix Matrix::operator*(const Matrix& rhs) const
    {
        Matrix res(*this);
        res *= rhs;

        return res;
    }

    inline Matrix Matrix::operator/(float rhs) const
    {
        Matrix res(*this);
        res /= rhs;

        return res;
    }

    inline Matrix& Matrix::operator+= (const Matrix& rhs)
    {
        mRows[0] += rhs.mRows[0];
        mRows[1] += rhs.mRows[1];
        mRows[2] += rhs.mRows[2];
        mRows[3] += rhs.mRows[3];

        return *this;
    }

    inline Matrix& Matrix::operator-= (const Matrix& rhs)
    {
        mRows[0] -= rhs.mRows[0];
        mRows[1] -= rhs.mRows[1];
        mRows[2] -= rhs.mRows[2];
        mRows[3] -= rhs.mRows[3];

        return *this;
    }

    inline Matrix& Matrix::operator*= (float rhs)
    {
        mRows[0] *= rhs;
        mRows[1] *= rhs;
        mRows[2] *= rhs;
        mRows[3] *= rhs;

        return *this;
    }

    inline Matrix& Matrix::operator*= (const Matrix& rhs)
    {
        // Broadcast each rhs row to 256-bit (same row in both lanes)
        __m256 rhs0 = _mm256_set_m128(rhs.mRows[0].mData, rhs.mRows[0].mData);
        __m256 rhs1 = _mm256_set_m128(rhs.mRows[1].mData, rhs.mRows[1].mData);
        __m256 rhs2 = _mm256_set_m128(rhs.mRows[2].mData, rhs.mRows[2].mData);
        __m256 rhs3 = _mm256_set_m128(rhs.mRows[3].mData, rhs.mRows[3].mData);

        // Process rows 0 and 1 simultaneously
        {
            __m256 src = _mm256_set_m128(mRows[1].mData, mRows[0].mData);

            // Broadcast each element within its 128-bit lane
            __m256 e0 = _mm256_shuffle_ps(src, src, _MM_SHUFFLE(0, 0, 0, 0));
            __m256 e1 = _mm256_shuffle_ps(src, src, _MM_SHUFFLE(1, 1, 1, 1));
            __m256 e2 = _mm256_shuffle_ps(src, src, _MM_SHUFFLE(2, 2, 2, 2));
            __m256 e3 = _mm256_shuffle_ps(src, src, _MM_SHUFFLE(3, 3, 3, 3));

            // FMA accumulation: e0*rhsRow0 + e1*rhsRow1 + e2*rhsRow2 + e3*rhsRow3
            __m256 result = _mm256_mul_ps(e0, rhs0);
            result = _mm256_fmadd_ps(e1, rhs1, result);
            result = _mm256_fmadd_ps(e2, rhs2, result);
            result = _mm256_fmadd_ps(e3, rhs3, result);

            mRows[0].mData = _mm256_castps256_ps128(result);
            mRows[1].mData = _mm256_extractf128_ps(result, 1);
        }

        // Process rows 2 and 3 simultaneously
        {
            __m256 src = _mm256_set_m128(mRows[3].mData, mRows[2].mData);

            __m256 e0 = _mm256_shuffle_ps(src, src, _MM_SHUFFLE(0, 0, 0, 0));
            __m256 e1 = _mm256_shuffle_ps(src, src, _MM_SHUFFLE(1, 1, 1, 1));
            __m256 e2 = _mm256_shuffle_ps(src, src, _MM_SHUFFLE(2, 2, 2, 2));
            __m256 e3 = _mm256_shuffle_ps(src, src, _MM_SHUFFLE(3, 3, 3, 3));

            __m256 result = _mm256_mul_ps(e0, rhs0);
            result = _mm256_fmadd_ps(e1, rhs1, result);
            result = _mm256_fmadd_ps(e2, rhs2, result);
            result = _mm256_fmadd_ps(e3, rhs3, result);

            mRows[2].mData = _mm256_castps256_ps128(result);
            mRows[3].mData = _mm256_extractf128_ps(result, 1);
        }

        return *this;
    }

    inline Matrix& Matrix::operator/= (float rhs)
    {
        mRows[0] /= rhs;
        mRows[1] /= rhs;
        mRows[2] /= rhs;
        mRows[3] /= rhs;

        return *this;
    }

    inline Vector4 Matrix::GetCol(int index) const
    {
        Vector4 col;

        __m128 t0 = mRows[0].mData;
        __m128 t1 = mRows[1].mData;
        __m128 t2 = mRows[2].mData;
        __m128 t3 = mRows[3].mData;

        _MM_TRANSPOSE4_PS(t0, t1, t2, t3);

        switch (index)
        {
        case 0: col.mData = t0; break;
        case 1: col.mData = t1; break;
        case 2: col.mData = t2; break;
        case 3: col.mData = t3; break;
        }

        return col;
    }

    inline Vector4 Matrix::GetRow(int index) const
    {
        return mRows[index];
    }

    inline void Matrix::SetCol(int index, const Vector4& col)
    {
        __m128 t;

        if (index == 0)
        {
            mRows[0].mData = _mm_move_ss(mRows[0].mData, col.mData);
            mRows[1].mData = _mm_move_ss(mRows[1].mData, col.mData);
            mRows[2].mData = _mm_move_ss(mRows[2].mData, col.mData);
            mRows[3].mData = _mm_move_ss(mRows[3].mData, col.mData);

            return;
        }

        // row 0
        t = _mm_shuffle_ps(col.mData, col.mData, _MM_SHUFFLE(0, 0, 0, 0));
        switch (index)
        {
        case 1:
            t = _mm_shuffle_ps(mRows[0].mData, t, _MM_SHUFFLE(0, 0, 0, 0));
            mRows[0].mData = _mm_shuffle_ps(t, mRows[0].mData, _MM_SHUFFLE(3, 2, 2, 1));
            break;
        case 2:
            t = _mm_shuffle_ps(t, mRows[0].mData, _MM_SHUFFLE(3, 3, 0, 0));
            mRows[0].mData = _mm_shuffle_ps(mRows[0].mData, t, _MM_SHUFFLE(2, 1, 1, 0));
            break;
        case 3:
            t = _mm_shuffle_ps(mRows[0].mData, t, _MM_SHUFFLE(0, 0, 2, 2));
            mRows[0].mData = _mm_shuffle_ps(mRows[0].mData, t, _MM_SHUFFLE(2, 1, 1, 0));
            break;
        }

        // row 1
        t = _mm_shuffle_ps(col.mData, col.mData, _MM_SHUFFLE(1, 1, 1, 1));
        switch (index)
        {
        case 1:
            t = _mm_shuffle_ps(mRows[1].mData, t, _MM_SHUFFLE(0, 0, 0, 0));
            mRows[1].mData = _mm_shuffle_ps(t, mRows[1].mData, _MM_SHUFFLE(3, 2, 2, 1));
            break;
        case 2:
            t = _mm_shuffle_ps(t, mRows[1].mData, _MM_SHUFFLE(3, 3, 0, 0));
            mRows[1].mData = _mm_shuffle_ps(mRows[1].mData, t, _MM_SHUFFLE(2, 1, 1, 0));
            break;
        case 3:
            t = _mm_shuffle_ps(mRows[1].mData, t, _MM_SHUFFLE(0, 0, 2, 2));
            mRows[1].mData = _mm_shuffle_ps(mRows[1].mData, t, _MM_SHUFFLE(2, 1, 1, 0));
            break;
        }

        // row 2
        t = _mm_shuffle_ps(col.mData, col.mData, _MM_SHUFFLE(2, 2, 2, 2));
        switch (index)
        {
        case 1:
            t = _mm_shuffle_ps(mRows[2].mData, t, _MM_SHUFFLE(0, 0, 0, 0));
            mRows[2].mData = _mm_shuffle_ps(t, mRows[2].mData, _MM_SHUFFLE(3, 2, 2, 1));
            break;
        case 2:
            t = _mm_shuffle_ps(t, mRows[2].mData, _MM_SHUFFLE(3, 3, 0, 0));
            mRows[2].mData = _mm_shuffle_ps(mRows[2].mData, t, _MM_SHUFFLE(2, 1, 1, 0));
            break;
        case 3:
            t = _mm_shuffle_ps(mRows[2].mData, t, _MM_SHUFFLE(0, 0, 2, 2));
            mRows[2].mData = _mm_shuffle_ps(mRows[2].mData, t, _MM_SHUFFLE(2, 1, 1, 0));
            break;
        }

        // row 3
        t = _mm_shuffle_ps(col.mData, col.mData, _MM_SHUFFLE(3, 3, 3, 3));
        switch (index)
        {
        case 1:
            t = _mm_shuffle_ps(mRows[3].mData, t, _MM_SHUFFLE(0, 0, 0, 0));
            mRows[3].mData = _mm_shuffle_ps(t, mRows[3].mData, _MM_SHUFFLE(3, 2, 2, 1));
            break;
        case 2:
            t = _mm_shuffle_ps(t, mRows[3].mData, _MM_SHUFFLE(3, 3, 0, 0));
            mRows[3].mData = _mm_shuffle_ps(mRows[3].mData, t, _MM_SHUFFLE(2, 1, 1, 0));
            break;
        case 3:
            t = _mm_shuffle_ps(mRows[3].mData, t, _MM_SHUFFLE(0, 0, 2, 2));
            mRows[3].mData = _mm_shuffle_ps(mRows[3].mData, t, _MM_SHUFFLE(2, 1, 1, 0));
            break;
        }
    }

    inline void Matrix::SetRow(int index, const Vector4& row)
    {
        mRows[index] = row;
    }

    inline void Matrix::Transpose()
    {
        _MM_TRANSPOSE4_PS(mRows[0].mData, mRows[1].mData, mRows[2].mData, mRows[3].mData);
    }

    inline Matrix Matrix::Transposed() const
    {
        Matrix res(*this);
        res.Transpose();

        return res;
    }

    inline void Matrix::Inverse()
    {
        Transpose();

        auto CalculateColCofactor = [](__m128 c1, __m128 c2, __m128 c3)
        {
            __m128 c30Coef;
            // (0, c12*c23-c22*c13, c11*c23-c21*c13, c11*c22-c21*c12)
            {
                __m128 l1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(1, 1, 2, 0));
                __m128 l2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(2, 3, 3, 0));
                __m128 r1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(2, 3, 3, 0));
                __m128 r2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(1, 1, 2, 0));
                c30Coef = _mm_fnmadd_ps(r1, r2, _mm_mul_ps(l1, l2));
            }

            // (c12*c23-c22*c13, 0, c20*c13-c10*c23, c20*c12-c10*c22)
            __m128 c31Coef;
            {
                __m128 l1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(2, 3, 0, 2));
                __m128 l2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(0, 0, 0, 3));
                __m128 r1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(0, 0, 0, 3));
                __m128 r2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(2, 3, 0, 2));
                c31Coef = _mm_fnmadd_ps(r1, r2, _mm_mul_ps(l1, l2));
            }

            // (c21*c13-c11*c23, c20*c13-c10*c23, 0, c10*c21-c20*c11)
            __m128 c32Coef;
            {
                __m128 l1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(0, 0, 3, 3));
                __m128 l2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(1, 0, 0, 1));
                __m128 r1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(1, 0, 0, 1));
                __m128 r2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(0, 0, 3, 3));
                c32Coef = _mm_fnmadd_ps(r1, r2, _mm_mul_ps(l1, l2));
            }

            // (c11*c22-c21*c12, c10*c22-c20*c12, c10*c21-c20*c11, 0)
            __m128 c33Coef;
            {
                __m128 l1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(0, 0, 0, 1));
                __m128 l2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(0, 1, 2, 2));
                __m128 r1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(0, 1, 2, 2));
                __m128 r2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(0, 0, 0, 1));
                c33Coef = _mm_fnmadd_ps(r1, r2, _mm_mul_ps(l1, l2));
            }

            __m128 c30 = _mm_shuffle_ps(c3, c3, _MM_SHUFFLE(0, 0, 0, 0));
            __m128 c31 = _mm_shuffle_ps(c3, c3, _MM_SHUFFLE(1, 1, 1, 1));
            __m128 c32 = _mm_shuffle_ps(c3, c3, _MM_SHUFFLE(2, 2, 2, 2));
            __m128 c33 = _mm_shuffle_ps(c3, c3, _MM_SHUFFLE(3, 3, 3, 3));

            // FMA accumulation: c30Coef*c30 + c31Coef*c31 + c32Coef*c32 + c33Coef*c33
            __m128 cof = _mm_mul_ps(c30Coef, c30);
            cof = _mm_fmadd_ps(c31Coef, c31, cof);
            cof = _mm_fmadd_ps(c32Coef, c32, cof);
            cof = _mm_fmadd_ps(c33Coef, c33, cof);

            return cof;
        };

        __m128 c0 = mRows[0].mData;
        __m128 c1 = mRows[1].mData;
        __m128 c2 = mRows[2].mData;
        __m128 c3 = mRows[3].mData;

        __m128 cof0 = CalculateColCofactor(c1, c2, c3);
        __m128 cof1 = CalculateColCofactor(c0, c2, c3);
        __m128 cof2 = CalculateColCofactor(c0, c1, c3);
        __m128 cof3 = CalculateColCofactor(c0, c1, c2);

        __m128 pmpm = _mm_set_ps(1.0f, -1.0f, 1.0f, -1.0f);
        __m128 mpmp = _mm_set_ps(-1.0f, 1.0f, -1.0f, 1.0f);

        __m128 det = _mm_mul_ps(_mm_mul_ps(c0, pmpm), cof0);
        det = AVX2::internal::GetSum<4>(det);
        __m128 invDet = _mm_div_ps(_mm_set1_ps(1.0f), det);

        __m128 v0 = _mm_mul_ps(_mm_mul_ps(cof0, pmpm), invDet);
        __m128 v1 = _mm_mul_ps(_mm_mul_ps(cof1, mpmp), invDet);
        __m128 v2 = _mm_mul_ps(_mm_mul_ps(cof2, pmpm), invDet);
        __m128 v3 = _mm_mul_ps(_mm_mul_ps(cof3, mpmp), invDet);

        mRows[0].mData = v0;
        mRows[1].mData = v1;
        mRows[2].mData = v2;
        mRows[3].mData = v3;
    }

    inline Matrix Matrix::Inversed() const
    {
        Matrix res(*this);
        res.Inverse();

        return res;
    }

    inline bool Matrix::IsAffine() const
    {
        __m128 t0 = mRows[0].mData;
        __m128 t1 = mRows[1].mData;
        __m128 t2 = mRows[2].mData;
        __m128 t3 = mRows[3].mData;
        _MM_TRANSPOSE4_PS(t0, t1, t2, t3);

        __m128 expected = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
        __m128 diff = _mm_sub_ps(t3, expected);
        __m128 dot = _mm_dp_ps(diff, diff, 0xFF);
        constexpr float kEps = 1e-6f;
        return _mm_comilt_ss(dot, _mm_set_ss(kEps * kEps));
    }

    inline void Matrix::AffineInverse()
    {
        __m128 row0 = mRows[0].mData;
        __m128 row1 = mRows[1].mData;
        __m128 row2 = mRows[2].mData;
        __m128 row3 = mRows[3].mData;

        // Compute cofactors of 3x3 block using cross-product pattern
        __m128 r1_yzx = _mm_shuffle_ps(row1, row1, _MM_SHUFFLE(3, 0, 2, 1));
        __m128 r2_zxy = _mm_shuffle_ps(row2, row2, _MM_SHUFFLE(3, 1, 0, 2));
        __m128 r1_zxy = _mm_shuffle_ps(row1, row1, _MM_SHUFFLE(3, 1, 0, 2));
        __m128 r2_yzx = _mm_shuffle_ps(row2, row2, _MM_SHUFFLE(3, 0, 2, 1));
        __m128 cofRow0 = _mm_fnmadd_ps(r1_zxy, r2_yzx, _mm_mul_ps(r1_yzx, r2_zxy));

        // Determinant
        __m128 det = _mm_dp_ps(row0, cofRow0, 0x7F);
        __m128 invDet = _mm_div_ps(_mm_set1_ps(1.0f), det);

        __m128 r0_zxy = _mm_shuffle_ps(row0, row0, _MM_SHUFFLE(3, 1, 0, 2));
        __m128 r0_yzx = _mm_shuffle_ps(row0, row0, _MM_SHUFFLE(3, 0, 2, 1));
        __m128 cofRow1 = _mm_fnmadd_ps(r0_yzx, r2_zxy, _mm_mul_ps(r0_zxy, r2_yzx));
        __m128 cofRow2 = _mm_fnmadd_ps(r0_zxy, r1_yzx, _mm_mul_ps(r0_yzx, r1_zxy));

        __m128 invRow0 = _mm_mul_ps(cofRow0, invDet);
        __m128 invRow1 = _mm_mul_ps(cofRow1, invDet);
        __m128 invRow2 = _mm_mul_ps(cofRow2, invDet);

        // Transpose 3x3
        __m128 tmp0 = _mm_unpacklo_ps(invRow0, invRow1);
        __m128 tmp1 = _mm_unpackhi_ps(invRow0, invRow1);
        __m128 tmp2 = _mm_unpacklo_ps(invRow2, _mm_setzero_ps());
        __m128 tmp3 = _mm_unpackhi_ps(invRow2, _mm_setzero_ps());

        __m128 iRow0 = _mm_movelh_ps(tmp0, tmp2);
        __m128 iRow1 = _mm_movehl_ps(tmp2, tmp0);
        __m128 iRow2 = _mm_movelh_ps(tmp1, tmp3);

        // New translation using FMA: t' = -(tx*iRow0 + ty*iRow1 + tz*iRow2)
        __m128 tx = _mm_shuffle_ps(row3, row3, _MM_SHUFFLE(0, 0, 0, 0));
        __m128 ty = _mm_shuffle_ps(row3, row3, _MM_SHUFFLE(1, 1, 1, 1));
        __m128 tz = _mm_shuffle_ps(row3, row3, _MM_SHUFFLE(2, 2, 2, 2));

        __m128 newTrans = _mm_mul_ps(tx, iRow0);
        newTrans = _mm_fmadd_ps(ty, iRow1, newTrans);
        newTrans = _mm_fmadd_ps(tz, iRow2, newTrans);
        newTrans = _mm_sub_ps(_mm_setzero_ps(), newTrans);
        newTrans = _mm_blend_ps(newTrans, _mm_set1_ps(1.0f), 0x8);

        mRows[0].mData = iRow0;
        mRows[1].mData = iRow1;
        mRows[2].mData = iRow2;
        mRows[3].mData = newTrans;
    }

    inline Matrix Matrix::AffineInversed() const
    {
        Matrix res(*this);
        res.AffineInverse();
        return res;
    }

    inline Matrix operator* (float lhs, const Matrix& rhs)
    {
        Matrix r(rhs);
        r *= lhs;

        return r;
    }

    inline Vector4 operator*(const Vector4& lhs, const Matrix& rhs)
    {
        Vector4 res;

        __m128 v0, v1, v2, v3;

        v3 = lhs.mData;
        v0 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(0, 0, 0, 0));
        v1 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 1, 1, 1));
        v2 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(2, 2, 2, 2));
        v3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(3, 3, 3, 3));

        // FMA accumulation: v0*row0 + v1*row1 + v2*row2 + v3*row3
        v0 = _mm_mul_ps(v0, rhs.mRows[0].mData);
        v0 = _mm_fmadd_ps(v1, rhs.mRows[1].mData, v0);
        v0 = _mm_fmadd_ps(v2, rhs.mRows[2].mData, v0);
        v0 = _mm_fmadd_ps(v3, rhs.mRows[3].mData, v0);
        res.mData = v0;

        return res;
    }
} // namespace cube
