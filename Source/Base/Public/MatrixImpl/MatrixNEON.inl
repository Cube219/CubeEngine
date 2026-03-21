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
        float32x4_t src, acc;

        // Row 0
        src = mRows[0].mData;
        acc = vmulq_laneq_f32(rhs.mRows[0].mData, src, 0);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[1].mData, src, 1);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[2].mData, src, 2);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[3].mData, src, 3);
        mRows[0].mData = acc;

        // Row 1
        src = mRows[1].mData;
        acc = vmulq_laneq_f32(rhs.mRows[0].mData, src, 0);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[1].mData, src, 1);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[2].mData, src, 2);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[3].mData, src, 3);
        mRows[1].mData = acc;

        // Row 2
        src = mRows[2].mData;
        acc = vmulq_laneq_f32(rhs.mRows[0].mData, src, 0);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[1].mData, src, 1);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[2].mData, src, 2);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[3].mData, src, 3);
        mRows[2].mData = acc;

        // Row 3
        src = mRows[3].mData;
        acc = vmulq_laneq_f32(rhs.mRows[0].mData, src, 0);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[1].mData, src, 1);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[2].mData, src, 2);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[3].mData, src, 3);
        mRows[3].mData = acc;

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
        // Transpose to turn columns into rows, then return the desired row
        float32x4x2_t t01 = vtrnq_f32(mRows[0].mData, mRows[1].mData);
        float32x4x2_t t23 = vtrnq_f32(mRows[2].mData, mRows[3].mData);

        Vector4 col;
        switch (index)
        {
        case 0:
            col.mData = vcombine_f32(vget_low_f32(t01.val[0]), vget_low_f32(t23.val[0]));
            break;
        case 1:
            col.mData = vcombine_f32(vget_low_f32(t01.val[1]), vget_low_f32(t23.val[1]));
            break;
        case 2:
            col.mData = vcombine_f32(vget_high_f32(t01.val[0]), vget_high_f32(t23.val[0]));
            break;
        case 3:
            col.mData = vcombine_f32(vget_high_f32(t01.val[1]), vget_high_f32(t23.val[1]));
            break;
        }

        return col;
    }

    inline Vector4 Matrix::GetRow(int index) const
    {
        return mRows[index];
    }

    inline void Matrix::SetCol(int index, const Vector4& col)
    {
        float f[4];
        vst1q_f32(f, col.mData);

        switch (index)
        {
        case 0:
            mRows[0].mData = vsetq_lane_f32(f[0], mRows[0].mData, 0);
            mRows[1].mData = vsetq_lane_f32(f[1], mRows[1].mData, 0);
            mRows[2].mData = vsetq_lane_f32(f[2], mRows[2].mData, 0);
            mRows[3].mData = vsetq_lane_f32(f[3], mRows[3].mData, 0);
            break;
        case 1:
            mRows[0].mData = vsetq_lane_f32(f[0], mRows[0].mData, 1);
            mRows[1].mData = vsetq_lane_f32(f[1], mRows[1].mData, 1);
            mRows[2].mData = vsetq_lane_f32(f[2], mRows[2].mData, 1);
            mRows[3].mData = vsetq_lane_f32(f[3], mRows[3].mData, 1);
            break;
        case 2:
            mRows[0].mData = vsetq_lane_f32(f[0], mRows[0].mData, 2);
            mRows[1].mData = vsetq_lane_f32(f[1], mRows[1].mData, 2);
            mRows[2].mData = vsetq_lane_f32(f[2], mRows[2].mData, 2);
            mRows[3].mData = vsetq_lane_f32(f[3], mRows[3].mData, 2);
            break;
        case 3:
            mRows[0].mData = vsetq_lane_f32(f[0], mRows[0].mData, 3);
            mRows[1].mData = vsetq_lane_f32(f[1], mRows[1].mData, 3);
            mRows[2].mData = vsetq_lane_f32(f[2], mRows[2].mData, 3);
            mRows[3].mData = vsetq_lane_f32(f[3], mRows[3].mData, 3);
            break;
        }
    }

    inline void Matrix::SetRow(int index, const Vector4& row)
    {
        mRows[index] = row;
    }

    inline void Matrix::Transpose()
    {
        // NEON 4x4 transpose using trn and zip operations
        // r0 = a0 a1 a2 a3
        // r1 = b0 b1 b2 b3
        // r2 = c0 c1 c2 c3
        // r3 = d0 d1 d2 d3

        float32x4x2_t t01 = vtrnq_f32(mRows[0].mData, mRows[1].mData);
        float32x4x2_t t23 = vtrnq_f32(mRows[2].mData, mRows[3].mData);

        // t01.val[0] = a0 b0 a2 b2
        // t01.val[1] = a1 b1 a3 b3
        // t23.val[0] = c0 d0 c2 d2
        // t23.val[1] = c1 d1 c3 d3

        mRows[0].mData = vcombine_f32(vget_low_f32(t01.val[0]), vget_low_f32(t23.val[0]));
        mRows[1].mData = vcombine_f32(vget_low_f32(t01.val[1]), vget_low_f32(t23.val[1]));
        mRows[2].mData = vcombine_f32(vget_high_f32(t01.val[0]), vget_high_f32(t23.val[0]));
        mRows[3].mData = vcombine_f32(vget_high_f32(t01.val[1]), vget_high_f32(t23.val[1]));
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

        // Computes signed cofactors with (+,-,+,-) row signs baked in via
        // swapped shuffle indices, eliminating the need for pmpm/mpmp sign
        // multiplication. For odd columns, swap c1/c2 args to negate all results.
        auto CalculateSignedColCofactor = [](float32x4_t c1, float32x4_t c2, float32x4_t c3)
        {
            // (0, c13*c22-c12*c23, c11*c23-c13*c21, c12*c21-c11*c22)
            float32x4_t c30Coef;
            {
                float32x4_t l1 = __builtin_shufflevector(c1, c1, 0, 3, 1, 2);
                float32x4_t l2 = __builtin_shufflevector(c2, c2, 0, 2, 3, 1);
                float32x4_t r1 = __builtin_shufflevector(c1, c1, 0, 2, 3, 1);
                float32x4_t r2 = __builtin_shufflevector(c2, c2, 0, 3, 1, 2);
                c30Coef = vfmsq_f32(vmulq_f32(l1, l2), r1, r2);
            }

            // (c12*c23-c13*c22, 0, c20*c13-c10*c23, c10*c22-c20*c12)
            float32x4_t c31Coef;
            {
                float32x4_t l1 = __builtin_shufflevector(c1, c1, 2, 0, 3, 0);
                float32x4_t l2 = __builtin_shufflevector(c2, c2, 3, 0, 0, 2);
                float32x4_t r1 = __builtin_shufflevector(c1, c1, 3, 0, 0, 2);
                float32x4_t r2 = __builtin_shufflevector(c2, c2, 2, 0, 3, 0);
                c31Coef = vfmsq_f32(vmulq_f32(l1, l2), r1, r2);
            }

            // (c13*c21-c11*c23, c10*c23-c13*c20, 0, c11*c20-c10*c21)
            float32x4_t c32Coef;
            {
                float32x4_t l1 = __builtin_shufflevector(c1, c1, 3, 0, 0, 1);
                float32x4_t l2 = __builtin_shufflevector(c2, c2, 1, 3, 0, 0);
                float32x4_t r1 = __builtin_shufflevector(c1, c1, 1, 3, 0, 0);
                float32x4_t r2 = __builtin_shufflevector(c2, c2, 3, 0, 0, 1);
                c32Coef = vfmsq_f32(vmulq_f32(l1, l2), r1, r2);
            }

            // (c11*c22-c12*c21, c12*c20-c10*c22, c10*c21-c11*c20, 0)
            float32x4_t c33Coef;
            {
                float32x4_t l1 = __builtin_shufflevector(c1, c1, 1, 2, 0, 0);
                float32x4_t l2 = __builtin_shufflevector(c2, c2, 2, 0, 1, 0);
                float32x4_t r1 = __builtin_shufflevector(c1, c1, 2, 0, 1, 0);
                float32x4_t r2 = __builtin_shufflevector(c2, c2, 1, 2, 0, 0);
                c33Coef = vfmsq_f32(vmulq_f32(l1, l2), r1, r2);
            }

            float32x4_t cof = vmulq_laneq_f32(c30Coef, c3, 0);
            cof = vfmaq_laneq_f32(cof, c31Coef, c3, 1);
            cof = vfmaq_laneq_f32(cof, c32Coef, c3, 2);
            cof = vfmaq_laneq_f32(cof, c33Coef, c3, 3);

            return cof;
        };

        float32x4_t c0 = mRows[0].mData;
        float32x4_t c1 = mRows[1].mData;
        float32x4_t c2 = mRows[2].mData;
        float32x4_t c3 = mRows[3].mData;

        // Lambda bakes in (+,-,+,-) sign pattern.
        // Swapping first two args negates the result, giving (-,+,-,+) for cof1/cof3.
        float32x4_t cof0 = CalculateSignedColCofactor(c1, c2, c3);
        float32x4_t cof1 = CalculateSignedColCofactor(c2, c0, c3);
        float32x4_t cof2 = CalculateSignedColCofactor(c0, c1, c3);
        float32x4_t cof3 = CalculateSignedColCofactor(c1, c0, c2);

        float32x4_t det = vmulq_f32(c0, cof0);
        det = NEON::internal::GetSum<4>(det);
        float32x4_t invDet = vdivq_f32(vdupq_n_f32(1.0f), det);

        mRows[0].mData = vmulq_f32(cof0, invDet);
        mRows[1].mData = vmulq_f32(cof1, invDet);
        mRows[2].mData = vmulq_f32(cof2, invDet);
        mRows[3].mData = vmulq_f32(cof3, invDet);
    }

    inline Matrix Matrix::Inversed() const
    {
        Matrix res(*this);
        res.Inverse();

        return res;
    }

    inline bool Matrix::IsAffine() const
    {
        float32x4x2_t t01 = vtrnq_f32(mRows[0].mData, mRows[1].mData);
        float32x4x2_t t23 = vtrnq_f32(mRows[2].mData, mRows[3].mData);
        float32x4_t col3 = vcombine_f32(vget_high_f32(t01.val[1]), vget_high_f32(t23.val[1]));

        float32x4_t expected = vdupq_n_f32(0.0f);
        expected = vsetq_lane_f32(1.0f, expected, 3); // (0.0f, 0.0f, 0.0f, 1.0f)
        float32x4_t diff = vsubq_f32(col3, expected);
        float32x4_t sq = vmulq_f32(diff, diff);
        float dot = vaddvq_f32(sq);
        return dot < FLOAT_EPS * FLOAT_EPS;
    }

    inline void Matrix::AffineInverse()
    {
        float32x4_t r0 = mRows[0].mData;
        float32x4_t r1 = mRows[1].mData;
        float32x4_t r2 = mRows[2].mData;
        float32x4_t r3 = mRows[3].mData;

        // Compute cofactors using cross-product pattern
        float32x4_t r1_yzx = vextq_f32(r1, r1, 3); // (?, x, y, z)
        r1_yzx = vextq_f32(r1_yzx, r1, 2); // (y, z, x, y)
        float32x4_t r1_zxy = vextq_f32(r1_yzx, r1_yzx, 1); // (z, x, y, y)

        float32x4_t r2_yzx = vextq_f32(r2, r2, 3);
        r2_yzx = vextq_f32(r2_yzx, r2, 2);
        float32x4_t r2_zxy = vextq_f32(r2_yzx, r2_yzx, 1);
        float32x4_t c0 = vfmsq_f32(vmulq_f32(r1_yzx, r2_zxy), r1_zxy, r2_yzx);

        float32x4_t detV = vmulq_f32(r0, c0);
        float det = vaddvq_f32(vsetq_lane_f32(0.0f, detV, 3));
        float32x4_t invDet = vdupq_n_f32(1.0f / det);

        float32x4_t r0_yzx = vextq_f32(r0, r0, 3);
        r0_yzx = vextq_f32(r0_yzx, r0, 2);
        float32x4_t r0_zxy = vextq_f32(r0_yzx, r0_yzx, 1);
        float32x4_t c1 = vfmsq_f32(vmulq_f32(r0_zxy, r2_yzx), r0_yzx, r2_zxy);
        float32x4_t c2 = vfmsq_f32(vmulq_f32(r0_yzx, r1_zxy), r0_zxy, r1_yzx);

        float32x4_t invR0 = vmulq_f32(c0, invDet);
        float32x4_t invR1 = vmulq_f32(c1, invDet);
        float32x4_t invR2 = vmulq_f32(c2, invDet);

        // Transpose 3x3 (invRow0/1/2 are columns of result, need rows)
        float32x4x2_t t01 = vtrnq_f32(invR0, invR1);
        float32x4_t zero = vdupq_n_f32(0.0f);
        float32x4x2_t t2z = vtrnq_f32(invR2, zero);

        float32x4_t iRow0 = vcombine_f32(vget_low_f32(t01.val[0]), vget_low_f32(t2z.val[0]));
        float32x4_t iRow1 = vcombine_f32(vget_low_f32(t01.val[1]), vget_low_f32(t2z.val[1]));
        float32x4_t iRow2 = vcombine_f32(vget_high_f32(t01.val[0]), vget_high_f32(t2z.val[0]));

        // New translation: t' = -(tx*iRow0 + ty*iRow1 + tz*iRow2)
        float32x4_t tr = vmulq_laneq_f32(iRow0, r3, 0);
        tr = vfmaq_laneq_f32(tr, iRow1, r3, 1);
        tr = vfmaq_laneq_f32(tr, iRow2, r3, 2);
        tr = vnegq_f32(tr);
        tr = vsetq_lane_f32(1.0f, tr, 3);

        mRows[0].mData = iRow0;
        mRows[1].mData = iRow1;
        mRows[2].mData = iRow2;
        mRows[3].mData = tr;
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

        float32x4_t src = lhs.mData;
        float32x4_t acc = vmulq_laneq_f32(rhs.mRows[0].mData, src, 0);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[1].mData, src, 1);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[2].mData, src, 2);
        acc = vfmaq_laneq_f32(acc, rhs.mRows[3].mData, src, 3);
        res.mData = acc;

        return res;
    }
} // namespace cube
