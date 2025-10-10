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
        __m128 v0, v1, v2, v3;

        // Row 0
        v3 = mRows[0].mData;
        v0 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(0, 0, 0, 0));
        v1 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 1, 1, 1));
        v2 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(2, 2, 2, 2));
        v3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(3, 3, 3, 3));

        v0 = _mm_mul_ps(v0, rhs.mRows[0].mData);
        v1 = _mm_mul_ps(v1, rhs.mRows[1].mData);
        v2 = _mm_mul_ps(v2, rhs.mRows[2].mData);
        v3 = _mm_mul_ps(v3, rhs.mRows[3].mData);

        v2 = _mm_add_ps(v2, v3);
        v1 = _mm_add_ps(v1, v2);
        v0 = _mm_add_ps(v0, v1);
        mRows[0].mData = v0;

        // Row 1
        v3 = mRows[1].mData;
        v0 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(0, 0, 0, 0));
        v1 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 1, 1, 1));
        v2 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(2, 2, 2, 2));
        v3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(3, 3, 3, 3));

        v0 = _mm_mul_ps(v0, rhs.mRows[0].mData);
        v1 = _mm_mul_ps(v1, rhs.mRows[1].mData);
        v2 = _mm_mul_ps(v2, rhs.mRows[2].mData);
        v3 = _mm_mul_ps(v3, rhs.mRows[3].mData);

        v2 = _mm_add_ps(v2, v3);
        v1 = _mm_add_ps(v1, v2);
        v0 = _mm_add_ps(v0, v1);
        mRows[1].mData = v0;

        // Row 2
        v3 = mRows[2].mData;
        v0 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(0, 0, 0, 0));
        v1 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 1, 1, 1));
        v2 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(2, 2, 2, 2));
        v3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(3, 3, 3, 3));

        v0 = _mm_mul_ps(v0, rhs.mRows[0].mData);
        v1 = _mm_mul_ps(v1, rhs.mRows[1].mData);
        v2 = _mm_mul_ps(v2, rhs.mRows[2].mData);
        v3 = _mm_mul_ps(v3, rhs.mRows[3].mData);

        v2 = _mm_add_ps(v2, v3);
        v1 = _mm_add_ps(v1, v2);
        v0 = _mm_add_ps(v0, v1);
        mRows[2].mData = v0;

        // Row 3
        v3 = mRows[3].mData;
        v0 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(0, 0, 0, 0));
        v1 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 1, 1, 1));
        v2 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(2, 2, 2, 2));
        v3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(3, 3, 3, 3));

        v0 = _mm_mul_ps(v0, rhs.mRows[0].mData);
        v1 = _mm_mul_ps(v1, rhs.mRows[1].mData);
        v2 = _mm_mul_ps(v2, rhs.mRows[2].mData);
        v3 = _mm_mul_ps(v3, rhs.mRows[3].mData);

        v2 = _mm_add_ps(v2, v3);
        v1 = _mm_add_ps(v1, v2);
        v0 = _mm_add_ps(v0, v1);
        mRows[3].mData = v0;

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

        __m128 t0, t1, t2, t3;

        switch (index)
        {
        case 0:
            t0 = _mm_shuffle_ps(mRows[0].mData, mRows[0].mData, _MM_SHUFFLE(0, 0, 0, 0));
            t1 = _mm_shuffle_ps(mRows[1].mData, mRows[1].mData, _MM_SHUFFLE(0, 0, 0, 0));
            t2 = _mm_shuffle_ps(mRows[2].mData, mRows[2].mData, _MM_SHUFFLE(0, 0, 0, 0));
            t3 = _mm_shuffle_ps(mRows[3].mData, mRows[3].mData, _MM_SHUFFLE(0, 0, 0, 0));
            break;
        case 1:
            t0 = _mm_shuffle_ps(mRows[0].mData, mRows[0].mData, _MM_SHUFFLE(1, 1, 1, 1));
            t1 = _mm_shuffle_ps(mRows[1].mData, mRows[1].mData, _MM_SHUFFLE(1, 1, 1, 1));
            t2 = _mm_shuffle_ps(mRows[2].mData, mRows[2].mData, _MM_SHUFFLE(1, 1, 1, 1));
            t3 = _mm_shuffle_ps(mRows[3].mData, mRows[3].mData, _MM_SHUFFLE(1, 1, 1, 1));
            break;
        case 2:
            t0 = _mm_shuffle_ps(mRows[0].mData, mRows[0].mData, _MM_SHUFFLE(2, 2, 2, 2));
            t1 = _mm_shuffle_ps(mRows[1].mData, mRows[1].mData, _MM_SHUFFLE(2, 2, 2, 2));
            t2 = _mm_shuffle_ps(mRows[2].mData, mRows[2].mData, _MM_SHUFFLE(2, 2, 2, 2));
            t3 = _mm_shuffle_ps(mRows[3].mData, mRows[3].mData, _MM_SHUFFLE(2, 2, 2, 2));
            break;
        case 3:
            t0 = _mm_shuffle_ps(mRows[0].mData, mRows[0].mData, _MM_SHUFFLE(3, 3, 3, 3));
            t1 = _mm_shuffle_ps(mRows[1].mData, mRows[1].mData, _MM_SHUFFLE(3, 3, 3, 3));
            t2 = _mm_shuffle_ps(mRows[2].mData, mRows[2].mData, _MM_SHUFFLE(3, 3, 3, 3));
            t3 = _mm_shuffle_ps(mRows[3].mData, mRows[3].mData, _MM_SHUFFLE(3, 3, 3, 3));
            break;
        }

        t3 = _mm_move_ss(t3, t2);
        t1 = _mm_move_ss(t1, t0);

        col.mData = _mm_movelh_ps(t1, t3);

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
                __m128 ll = _mm_mul_ps(l1, l2);
                __m128 rr = _mm_mul_ps(r1, r2);
                c30Coef = _mm_sub_ps(ll, rr);
            }

            // (c12*c23-c22*c13, 0, c20*c13-c10*c23, c20*c12-c10*c22)
            __m128 c31Coef;
            {
                __m128 l1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(2, 3, 0, 2));
                __m128 l2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(0, 0, 0, 3));
                __m128 r1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(0, 0, 0, 3));
                __m128 r2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(2, 3, 0, 2));
                __m128 ll = _mm_mul_ps(l1, l2);
                __m128 rr = _mm_mul_ps(r1, r2);
                c31Coef = _mm_sub_ps(ll, rr);
            }

            // (c21*c13-c11*c23, c20*c13-c10*c23, 0, c10*c21-c20*c11)
            __m128 c32Coef;
            {
                __m128 l1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(0, 0, 3, 3));
                __m128 l2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(1, 0, 0, 1));
                __m128 r1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(1, 0, 0, 1));
                __m128 r2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(0, 0, 3, 3));
                __m128 ll = _mm_mul_ps(l1, l2);
                __m128 rr = _mm_mul_ps(r1, r2);
                c32Coef = _mm_sub_ps(ll, rr);
            }

            // (c11*c22-c21*c12, c10*c22-c20*c12, c10*c21-c20*c11, 0)
            __m128 c33Coef;
            {
                __m128 l1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(0, 0, 0, 1));
                __m128 l2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(0, 1, 2, 2));
                __m128 r1 = _mm_shuffle_ps(c1, c1, _MM_SHUFFLE(0, 1, 2, 2));
                __m128 r2 = _mm_shuffle_ps(c2, c2, _MM_SHUFFLE(0, 0, 0, 1));
                __m128 ll = _mm_mul_ps(l1, l2);
                __m128 rr = _mm_mul_ps(r1, r2);
                c33Coef = _mm_sub_ps(ll, rr);
            }

            __m128 c30 = _mm_shuffle_ps(c3, c3, _MM_SHUFFLE(0, 0, 0, 0));
            __m128 c31 = _mm_shuffle_ps(c3, c3, _MM_SHUFFLE(1, 1, 1, 1));
            __m128 c32 = _mm_shuffle_ps(c3, c3, _MM_SHUFFLE(2, 2, 2, 2));
            __m128 c33 = _mm_shuffle_ps(c3, c3, _MM_SHUFFLE(3, 3, 3, 3));

            __m128 cof = _mm_mul_ps(c30Coef, c30);
            __m128 t = _mm_mul_ps(c31Coef, c31);
            cof = _mm_add_ps(cof, t);
            t = _mm_mul_ps(c32Coef, c32);
            cof = _mm_add_ps(cof, t);
            t = _mm_mul_ps(c33Coef, c33);
            cof = _mm_add_ps(cof, t);

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
        det = SSE::internal::GetSum<4>(det);
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

        v0 = _mm_mul_ps(v0, rhs.mRows[0].mData);
        v1 = _mm_mul_ps(v1, rhs.mRows[1].mData);
        v2 = _mm_mul_ps(v2, rhs.mRows[2].mData);
        v3 = _mm_mul_ps(v3, rhs.mRows[3].mData);

        v2 = _mm_add_ps(v2, v3);
        v1 = _mm_add_ps(v1, v2);
        v0 = _mm_add_ps(v0, v1);
        res.mData = v0;

        return res;
    }
} // namespace cube
