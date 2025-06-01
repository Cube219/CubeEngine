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
            Vector4::Zero()
        );
    }

    inline Matrix Matrix::Identity()
    {
        return Matrix(
            Vector4(1.0f, 0.0f, 0.0f, 0.0f),
            Vector4(0.0f, 1.0f, 0.0f, 0.0f),
            Vector4(0.0f, 0.0f, 1.0f, 0.0f),
            Vector4(0.0f, 0.0f, 0.0f, 1.0f)
        );
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

    inline Matrix Matrix::operator+ (const Matrix& rhs) const
    {
        Matrix res(*this);
        res += rhs;

        return res;
    }

    inline Matrix Matrix::operator- (const Matrix& rhs) const
    {
        Matrix res(*this);
        res -= rhs;

        return res;
    }

    inline Matrix Matrix::operator* (float rhs) const
    {
        Matrix res(*this);
        res *= rhs;

        return res;
    }

    inline Matrix Matrix::operator* (const Matrix& rhs) const
    {
        Matrix res(*this);
        res *= rhs;

        return res;
    }

    inline Matrix Matrix::operator/ (float rhs) const
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
        float v0, v1, v2, v3;

        // Row 0
        v0 = mRows[0].mData[0];
        v1 = mRows[0].mData[1];
        v2 = mRows[0].mData[2];
        v3 = mRows[0].mData[3];

        mRows[0].mData[0] = (v0 * rhs.mRows[0].mData[0]) + (v1 * rhs.mRows[1].mData[0]) + (v2 * rhs.mRows[2].mData[0]) + (v3 * rhs.mRows[3].mData[0]);
        mRows[0].mData[1] = (v0 * rhs.mRows[0].mData[1]) + (v1 * rhs.mRows[1].mData[1]) + (v2 * rhs.mRows[2].mData[1]) + (v3 * rhs.mRows[3].mData[1]);
        mRows[0].mData[2] = (v0 * rhs.mRows[0].mData[2]) + (v1 * rhs.mRows[1].mData[2]) + (v2 * rhs.mRows[2].mData[2]) + (v3 * rhs.mRows[3].mData[2]);
        mRows[0].mData[3] = (v0 * rhs.mRows[0].mData[3]) + (v1 * rhs.mRows[1].mData[3]) + (v2 * rhs.mRows[2].mData[3]) + (v3 * rhs.mRows[3].mData[3]);

        // Row 1
        v0 = mRows[1].mData[0];
        v1 = mRows[1].mData[1];
        v2 = mRows[1].mData[2];
        v3 = mRows[1].mData[3];

        mRows[1].mData[0] = (v0 * rhs.mRows[0].mData[0]) + (v1 * rhs.mRows[1].mData[0]) + (v2 * rhs.mRows[2].mData[0]) + (v3 * rhs.mRows[3].mData[0]);
        mRows[1].mData[1] = (v0 * rhs.mRows[0].mData[1]) + (v1 * rhs.mRows[1].mData[1]) + (v2 * rhs.mRows[2].mData[1]) + (v3 * rhs.mRows[3].mData[1]);
        mRows[1].mData[2] = (v0 * rhs.mRows[0].mData[2]) + (v1 * rhs.mRows[1].mData[2]) + (v2 * rhs.mRows[2].mData[2]) + (v3 * rhs.mRows[3].mData[2]);
        mRows[1].mData[3] = (v0 * rhs.mRows[0].mData[3]) + (v1 * rhs.mRows[1].mData[3]) + (v2 * rhs.mRows[2].mData[3]) + (v3 * rhs.mRows[3].mData[3]);

        // Row 2
        v0 = mRows[2].mData[0];
        v1 = mRows[2].mData[1];
        v2 = mRows[2].mData[2];
        v3 = mRows[2].mData[3];

        mRows[2].mData[0] = (v0 * rhs.mRows[0].mData[0]) + (v1 * rhs.mRows[1].mData[0]) + (v2 * rhs.mRows[2].mData[0]) + (v3 * rhs.mRows[3].mData[0]);
        mRows[2].mData[1] = (v0 * rhs.mRows[0].mData[1]) + (v1 * rhs.mRows[1].mData[1]) + (v2 * rhs.mRows[2].mData[1]) + (v3 * rhs.mRows[3].mData[1]);
        mRows[2].mData[2] = (v0 * rhs.mRows[0].mData[2]) + (v1 * rhs.mRows[1].mData[2]) + (v2 * rhs.mRows[2].mData[2]) + (v3 * rhs.mRows[3].mData[2]);
        mRows[2].mData[3] = (v0 * rhs.mRows[0].mData[3]) + (v1 * rhs.mRows[1].mData[3]) + (v2 * rhs.mRows[2].mData[3]) + (v3 * rhs.mRows[3].mData[3]);

        // Row 3
        v0 = mRows[3].mData[0];
        v1 = mRows[3].mData[1];
        v2 = mRows[3].mData[2];
        v3 = mRows[3].mData[3];

        mRows[3].mData[0] = (v0 * rhs.mRows[0].mData[0]) + (v1 * rhs.mRows[1].mData[0]) + (v2 * rhs.mRows[2].mData[0]) + (v3 * rhs.mRows[3].mData[0]);
        mRows[3].mData[1] = (v0 * rhs.mRows[0].mData[1]) + (v1 * rhs.mRows[1].mData[1]) + (v2 * rhs.mRows[2].mData[1]) + (v3 * rhs.mRows[3].mData[1]);
        mRows[3].mData[2] = (v0 * rhs.mRows[0].mData[2]) + (v1 * rhs.mRows[1].mData[2]) + (v2 * rhs.mRows[2].mData[2]) + (v3 * rhs.mRows[3].mData[2]);
        mRows[3].mData[3] = (v0 * rhs.mRows[0].mData[3]) + (v1 * rhs.mRows[1].mData[3]) + (v2 * rhs.mRows[2].mData[3]) + (v3 * rhs.mRows[3].mData[3]);

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

        col.mData[0] = mRows[0].mData[index];
        col.mData[1] = mRows[1].mData[index];
        col.mData[2] = mRows[2].mData[index];
        col.mData[3] = mRows[3].mData[index];

        return col;
    }

    inline Vector4 Matrix::GetRow(int index) const
    {
        return mRows[index];
    }

    inline void Matrix::SetCol(int index, const Vector4& col)
    {
        mRows[0].mData[index] = col.mData[0];
        mRows[1].mData[index] = col.mData[1];
        mRows[2].mData[index] = col.mData[2];
        mRows[3].mData[index] = col.mData[3];
    }

    inline void Matrix::SetRow(int index, const Vector4& row)
    {
        mRows[index] = row;
    }

    inline void Matrix::Transpose()
    {
        for(int i = 1; i < 4; i++) {
            for(int j = 0; j < i; j++) {
                float tmp = mRows[i].mData[j];
                mRows[i].mData[j] = mRows[j].mData[i];
                mRows[j].mData[i] = tmp;
            }
        }
    }

    inline Matrix Matrix::Transposed() const
    {
        Matrix res(*this);
        res.Transpose();

        return res;
    }

    inline void Matrix::Inverse()
    {
        *this = this->Inversed();
    }

    inline Matrix Matrix::Inversed() const
    {
        float subMatrixDet[4][4];
        subMatrixDet[0][0] = GetSubMatrixDet<0, 0>();
        subMatrixDet[0][1] = GetSubMatrixDet<0, 1>();
        subMatrixDet[0][2] = GetSubMatrixDet<0, 2>();
        subMatrixDet[0][3] = GetSubMatrixDet<0, 3>();
        subMatrixDet[1][0] = GetSubMatrixDet<1, 0>();
        subMatrixDet[1][1] = GetSubMatrixDet<1, 1>();
        subMatrixDet[1][2] = GetSubMatrixDet<1, 2>();
        subMatrixDet[1][3] = GetSubMatrixDet<1, 3>();
        subMatrixDet[2][0] = GetSubMatrixDet<2, 0>();
        subMatrixDet[2][1] = GetSubMatrixDet<2, 1>();
        subMatrixDet[2][2] = GetSubMatrixDet<2, 2>();
        subMatrixDet[2][3] = GetSubMatrixDet<2, 3>();
        subMatrixDet[3][0] = GetSubMatrixDet<3, 0>();
        subMatrixDet[3][1] = GetSubMatrixDet<3, 1>();
        subMatrixDet[3][2] = GetSubMatrixDet<3, 2>();
        subMatrixDet[3][3] = GetSubMatrixDet<3, 3>();

        const float det =
            mRows[0].mData[0] * subMatrixDet[0][0] +
            mRows[1].mData[0] * subMatrixDet[1][0] +
            mRows[2].mData[0] * subMatrixDet[2][0] +
            mRows[3].mData[0] * subMatrixDet[3][0];
        const float rDet = 1.0f / det;

        Matrix res;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                res.mRows[i].mData[j] = subMatrixDet[i][j] * rDet;
                if ((i + j) % 2 == 1)
                {
                    res.mRows[i].mData[j] *= -1;
                }
            }
        }
        res.Transpose();
        
        return res;
    }
    
    template <int removeRow, int removeCol>
    float Matrix::GetSubMatrixDet() const
    {
        const Vector4* row0;
        const Vector4* row1;
        const Vector4* row2;
        if constexpr (removeRow == 0)
        {
            row0 = &mRows[1];
            row1 = &mRows[2];
            row2 = &mRows[3];
        }
        else if constexpr (removeRow == 1)
        {
            row0 = &mRows[0];
            row1 = &mRows[2];
            row2 = &mRows[3];
        }
        else if constexpr (removeRow == 2)
        {
            row0 = &mRows[0];
            row1 = &mRows[1];
            row2 = &mRows[3];
        }
        else if constexpr (removeRow == 3)
        {
            row0 = &mRows[0];
            row1 = &mRows[1];
            row2 = &mRows[2];
        }
        else
        {
            static_assert(removeRow < 0 || removeRow > 3, "Invalid remove row");
            return 0.0f;
        }

        int col0, col1, col2;
        if constexpr (removeCol == 0)
        {
            col0 = 1;
            col1 = 2;
            col2 = 3;
        }
        else if constexpr (removeCol == 1)
        {
            col0 = 0;
            col1 = 2;
            col2 = 3;
        }
        else if constexpr (removeCol == 2)
        {
            col0 = 0;
            col1 = 1;
            col2 = 3;
        }
        else if constexpr (removeCol == 3)
        {
            col0 = 0;
            col1 = 1;
            col2 = 2;
        }
        else
        {
            static_assert(removeCol < 0 || removeCol > 3, "Invalid remove col");
            return 0.0f;
        }

        return
            (row0->mData[col0])*(row1->mData[col1])*(row2->mData[col2]) + (row0->mData[col1])*(row1->mData[col2])*(row2->mData[col0]) + (row0->mData[col2])*(row1->mData[col0])*(row2->mData[col1])
            - (row0->mData[col0])*(row1->mData[col2])*(row2->mData[col1]) - (row0->mData[col1])*(row1->mData[col0])*(row2->mData[col2]) - (row0->mData[col2])*(row1->mData[col1])*(row2->mData[col0]);
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

        float v0, v1, v2, v3;

        v0 = lhs.mData[0];
        v1 = lhs.mData[1];
        v2 = lhs.mData[2];
        v3 = lhs.mData[3];

        res.mData[0] = (v0 * rhs.mRows[0].mData[0]) + (v1 * rhs.mRows[1].mData[0]) + (v2 * rhs.mRows[2].mData[0]) + (v3 * rhs.mRows[3].mData[0]);
        res.mData[1] = (v0 * rhs.mRows[0].mData[1]) + (v1 * rhs.mRows[1].mData[1]) + (v2 * rhs.mRows[2].mData[1]) + (v3 * rhs.mRows[3].mData[1]);
        res.mData[2] = (v0 * rhs.mRows[0].mData[2]) + (v1 * rhs.mRows[1].mData[2]) + (v2 * rhs.mRows[2].mData[2]) + (v3 * rhs.mRows[3].mData[2]);
        res.mData[3] = (v0 * rhs.mRows[0].mData[3]) + (v1 * rhs.mRows[1].mData[3]) + (v2 * rhs.mRows[2].mData[3]) + (v3 * rhs.mRows[3].mData[3]);

        return res;
    }
} // namespace cube
