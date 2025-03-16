#define CUBE_MATRIX_IMPLEMENTATION

#include "../Matrix.h"

namespace cube
{
    inline Matrix Matrix::Zero()
    {
        Matrix r;
        r.mRows[0] = VectorBase::Zero();
        r.mRows[1] = VectorBase::Zero();
        r.mRows[2] = VectorBase::Zero();
        r.mRows[3] = VectorBase::Zero();

        return r;
    }

    inline Matrix Matrix::Identity()
    {
        Matrix r;
        r.mRows[0] = VectorBase(1.0f, 0.0f, 0.0f, 0.0f);
        r.mRows[1] = VectorBase(0.0f, 1.0f, 0.0f, 0.0f);
        r.mRows[2] = VectorBase(0.0f, 0.0f, 1.0f, 0.0f);
        r.mRows[3] = VectorBase(0.0f, 0.0f, 0.0f, 1.0f);

        return r;
    }

    inline Matrix::Matrix() {}

    inline Matrix::Matrix(const VectorBase& row1, const VectorBase& row2, const VectorBase& row3, const VectorBase& row4) :
        mRows{ row1, row2, row3, row4 }
    {}

    inline Matrix::Matrix(float v[16])
    {
        mRows[0] = VectorBase(v[0], v[1], v[2], v[3]);
        mRows[1] = VectorBase(v[4], v[5], v[6], v[7]);
        mRows[2] = VectorBase(v[8], v[9], v[10], v[11]);
        mRows[3] = VectorBase(v[12], v[13], v[14], v[15]);
    }

    inline Matrix::Matrix(float v11, float v12, float v13, float v14,
        float v21, float v22, float v23, float v24,
        float v31, float v32, float v33, float v34,
        float v41, float v42, float v43, float v44)
    {
        mRows[0] = VectorBase(v11, v12, v13, v14);
        mRows[1] = VectorBase(v21, v22, v23, v24);
        mRows[2] = VectorBase(v31, v32, v33, v34);
        mRows[3] = VectorBase(v41, v42, v43, v44);
    }

    inline Matrix::~Matrix() {}

    inline Matrix::Matrix(const Matrix& other)
    {
        mRows[0] = other.mRows[0];
        mRows[1] = other.mRows[1];
        mRows[2] = other.mRows[2];
        mRows[3] = other.mRows[3];
    }
    inline Matrix& Matrix::operator=(const Matrix& rhs)
    {
        mRows[0] = rhs.mRows[0];
        mRows[1] = rhs.mRows[1];
        mRows[2] = rhs.mRows[2];
        mRows[3] = rhs.mRows[3];

        return *this;
    }
    inline VectorBase& Matrix::operator[](int i)
    {
        return mRows[i];
    }

    inline Matrix Matrix::operator+ (const Matrix& rhs) const
    {
        Matrix r;
        r.mRows[0] = mRows[0] + rhs.mRows[0];
        r.mRows[1] = mRows[1] + rhs.mRows[1];
        r.mRows[2] = mRows[2] + rhs.mRows[2];
        r.mRows[3] = mRows[3] + rhs.mRows[3];

        return r;
    }
    inline Matrix Matrix::operator- (const Matrix& rhs) const
    {
        Matrix r;
        r.mRows[0] = mRows[0] - rhs.mRows[0];
        r.mRows[1] = mRows[1] - rhs.mRows[1];
        r.mRows[2] = mRows[2] - rhs.mRows[2];
        r.mRows[3] = mRows[3] - rhs.mRows[3];

        return r;
    }
    inline Matrix Matrix::operator* (float rhs) const
    {
        Matrix r;
        r.mRows[0] = mRows[0] * rhs;
        r.mRows[1] = mRows[1] * rhs;
        r.mRows[2] = mRows[2] * rhs;
        r.mRows[3] = mRows[3] * rhs;

        return r;
    }
    inline Matrix Matrix::operator* (const Matrix& rhs) const
    {
        Matrix res = *this;
        res *= rhs;

        return res;
    }
    inline Matrix Matrix::operator/ (float rhs) const
    {
        Matrix r;
        r.mRows[0] = mRows[0] / rhs;
        r.mRows[1] = mRows[1] / rhs;
        r.mRows[2] = mRows[2] / rhs;
        r.mRows[3] = mRows[3] / rhs;

        return r;
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
        VectorData v0, v1, v2, v3;

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

    inline VectorBase Matrix::GetCol(int index) const
    {
        VectorBase col;

        VectorData t0, t1, t2, t3;

        switch(index) {
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
    inline VectorBase Matrix::GetRow(int index) const
    {
        return mRows[index];
    }

    inline void Matrix::SetCol(int index, const VectorBase& col)
    {
        VectorData t;

        if(index == 0) {
            mRows[0].mData = _mm_move_ss(mRows[0].mData, col.mData);
            mRows[1].mData = _mm_move_ss(mRows[1].mData, col.mData);
            mRows[2].mData = _mm_move_ss(mRows[2].mData, col.mData);
            mRows[3].mData = _mm_move_ss(mRows[3].mData, col.mData);

            return;
        }

        // row 0
        t = _mm_shuffle_ps(col.mData, col.mData, _MM_SHUFFLE(0, 0, 0, 0));
        switch(index) {
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
        switch(index) {
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
        switch(index) {
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
        switch(index) {
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
    inline void Matrix::SetRow(int index, const VectorBase& row)
    {
        mRows[index] = row;
    }

    inline void Matrix::Transpose()
    {
        _MM_TRANSPOSE4_PS(mRows[0].mData, mRows[1].mData, mRows[2].mData, mRows[3].mData);
    }
    inline Matrix Matrix::Transposed() const
    {
        Matrix res = *this;
        res.Transpose();

        return res;
    }

    inline Matrix operator* (float lhs, const Matrix& rhs)
    {
        Matrix r;
        r.mRows[0] = rhs.mRows[0] * lhs;
        r.mRows[1] = rhs.mRows[1] * lhs;
        r.mRows[2] = rhs.mRows[2] * lhs;
        r.mRows[3] = rhs.mRows[3] * lhs;

        return r;
    }

    inline VectorBase operator*(const Vector4& lhs, const Matrix& rhs)
    {
        VectorBase res;

        VectorData v0, v1, v2, v3;

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
