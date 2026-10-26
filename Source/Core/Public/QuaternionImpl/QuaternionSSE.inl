#define CUBE_QUATERNION_IMPLEMENTATION

#include "../Quaternion.h"

#include "../CubeMath.h"

namespace cube
{
    inline Quaternion Quaternion::Zero()
    {
        Quaternion res;
        res.mData = _mm_setzero_ps();

        return res;
    }

    inline Quaternion Quaternion::Identity()
    {
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }

    inline Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angle)
    {
        // Cast to Vector4 to set w element 0.
        Vector4 a = Vector4(axis.Normalized());

        float half = angle * 0.5f;
        float s = Math::Sin(half);
        float c = Math::Cos(half);

        __m128 scalar = _mm_set_ps(c, 0.0f, 0.0f, 0.0f);

        Quaternion res;
        res.mData = _mm_add_ps(_mm_mul_ps(a.mData, _mm_set1_ps(s)), scalar);

        return res;
    }

    inline Quaternion Quaternion::FromEulerXYZ(float xAngle, float yAngle, float zAngle)
    {
        Quaternion qx = FromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), xAngle);
        Quaternion qy = FromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), yAngle);
        Quaternion qz = FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), zAngle);

        // Same as MatrixUtility::GetRotationXYZ(). (X -> Y -> Z)
        return qz * qy * qx;
    }

    inline Quaternion Quaternion::FromEulerXYZ(const Vector3& angles)
    {
        Float3 f = angles.GetFloat3();
        return FromEulerXYZ(f.x, f.y, f.z);
    }

    inline Quaternion Quaternion::FromRotationMatrix(const Matrix& matrix)
    {
        Float4 r0 = matrix.GetRow(0).GetFloat4();
        Float4 r1 = matrix.GetRow(1).GetFloat4();
        Float4 r2 = matrix.GetRow(2).GetFloat4();

        float m00 = r0.x, m01 = r0.y, m02 = r0.z;
        float m10 = r1.x, m11 = r1.y, m12 = r1.z;
        float m20 = r2.x, m21 = r2.y, m22 = r2.z;

        // Derived quaternion components from ToRotationMatrix().
        float trace = m00 + m11 + m22;
        float x, y, z, w;

        if (trace > 0.0f)
        {
            float s = Math::Sqrt(trace + 1.0f) * 2.0f; // s = 4w
            w = 0.25f * s;
            x = (m12 - m21) / s;
            y = (m20 - m02) / s;
            z = (m01 - m10) / s;
        }
        else if (m00 > m11 && m00 > m22)
        {
            float s = Math::Sqrt(1.0f + m00 - m11 - m22) * 2.0f; // s = 4x
            w = (m12 - m21) / s;
            x = 0.25f * s;
            y = (m10 + m01) / s;
            z = (m20 + m02) / s;
        }
        else if (m11 > m22)
        {
            float s = Math::Sqrt(1.0f + m11 - m00 - m22) * 2.0f; // s = 4y
            w = (m20 - m02) / s;
            x = (m10 + m01) / s;
            y = 0.25f * s;
            z = (m21 + m12) / s;
        }
        else
        {
            float s = Math::Sqrt(1.0f + m22 - m00 - m11) * 2.0f; // s = 4z
            w = (m01 - m10) / s;
            x = (m20 + m02) / s;
            y = (m21 + m12) / s;
            z = 0.25f * s;
        }

        return Quaternion(x, y, z, w);
    }

    inline Quaternion::Quaternion(float x, float y, float z, float w)
    {
        mData = _mm_set_ps(w, z, y, x);
    }

    inline Quaternion::Quaternion(const Quaternion& other)
    {
        mData = other.mData;
    }

    inline Quaternion& Quaternion::operator=(const Quaternion& rhs)
    {
        mData = rhs.mData;

        return *this;
    }

    inline bool Quaternion::operator==(const Quaternion& rhs) const
    {
        __m128 res = _mm_cmpeq_ps(mData, rhs.mData);
        return (_mm_movemask_ps(res) & 0b1111) == 0b1111;
    }

    inline bool Quaternion::operator!=(const Quaternion& rhs) const
    {
        return !(*this == rhs);
    }

    inline Quaternion Quaternion::operator+(const Quaternion& rhs) const
    {
        Quaternion res(*this);
        res += rhs;

        return res;
    }

    inline Quaternion Quaternion::operator-(const Quaternion& rhs) const
    {
        Quaternion res(*this);
        res -= rhs;

        return res;
    }

    inline Quaternion Quaternion::operator*(float rhs) const
    {
        Quaternion res(*this);
        res *= rhs;

        return res;
    }

    inline Quaternion Quaternion::operator*(const Quaternion& rhs) const
    {
        Quaternion res(*this);
        res *= rhs;

        return res;
    }

    inline const Quaternion& Quaternion::operator+() const
    {
        return *this;
    }

    inline Quaternion Quaternion::operator-() const
    {
        Quaternion res;
        res.mData = _mm_xor_ps(mData, _mm_set1_ps(-0.0f));

        return res;
    }

    inline Quaternion& Quaternion::operator+=(const Quaternion& rhs)
    {
        mData = _mm_add_ps(mData, rhs.mData);

        return *this;
    }

    inline Quaternion& Quaternion::operator-=(const Quaternion& rhs)
    {
        mData = _mm_sub_ps(mData, rhs.mData);

        return *this;
    }

    inline Quaternion& Quaternion::operator*=(float rhs)
    {
        mData = _mm_mul_ps(mData, _mm_set1_ps(rhs));

        return *this;
    }

    inline Quaternion& Quaternion::operator*=(const Quaternion& rhs)
    {
        // Hamilton product. result = w1*q2 + x1*(w2,-z2,y2,-x2) + y1*(z2,w2,-x2,-y2) + z1*(-y2,x2,w2,-z2)
        __m128 q1 = mData;
        __m128 q2 = rhs.mData;

        __m128 x1 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(0, 0, 0, 0));
        __m128 y1 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(1, 1, 1, 1));
        __m128 z1 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(2, 2, 2, 2));
        __m128 w1 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(3, 3, 3, 3));

        __m128 q2_1 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(0, 1, 2, 3)); // (w2, z2, y2, x2)
        __m128 q2_2 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(1, 0, 3, 2)); // (z2, w2, x2, y2)
        __m128 q2_3 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(2, 3, 0, 1)); // (y2, x2, w2, z2)

        __m128 pnpn = _mm_set_ps(-1.0f, 1.0f, -1.0f, 1.0f);
        __m128 ppnn = _mm_shuffle_ps(pnpn, pnpn, _MM_SHUFFLE(1, 1, 0, 0));
        __m128 nppn = _mm_shuffle_ps(pnpn, pnpn, _MM_SHUFFLE(1, 0, 0, 1));

        __m128 res = _mm_mul_ps(w1, q2);
        res = _mm_add_ps(res, _mm_mul_ps(x1, _mm_mul_ps(q2_1, pnpn)));
        res = _mm_add_ps(res, _mm_mul_ps(y1, _mm_mul_ps(q2_2, ppnn)));
        res = _mm_add_ps(res, _mm_mul_ps(z1, _mm_mul_ps(q2_3, nppn)));

        mData = res;

        return *this;
    }

    inline Float4 Quaternion::GetFloat4() const
    {
        float f[4];
        _mm_store_ps(f, mData);

        Float4 res;
        res.x = f[0];
        res.y = f[1];
        res.z = f[2];
        res.w = f[3];

        return res;
    }

    inline float Quaternion::Length() const
    {
        return Math::Sqrt(SquareLength());
    }

    inline float Quaternion::SquareLength() const
    {
        __m128 res = _mm_dp_ps(mData, mData, 0xF1);
        float f;
        _mm_store_ss(&f, res);

        return f;
    }

    inline void Quaternion::Normalize()
    {
        __m128 len = _mm_sqrt_ps(_mm_dp_ps(mData, mData, 0xFF));
        mData = _mm_div_ps(mData, len);
    }

    inline Quaternion Quaternion::Normalized() const
    {
        Quaternion res(*this);
        res.Normalize();

        return res;
    }

    inline float Quaternion::Dot(const Quaternion& rhs) const
    {
        __m128 res = _mm_dp_ps(mData, rhs.mData, 0xF1);
        float f;
        _mm_store_ss(&f, res);

        return f;
    }

    inline float Quaternion::Dot(const Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs.Dot(rhs);
    }

    inline void Quaternion::Conjugate()
    {
        mData = _mm_mul_ps(mData, _mm_set_ps(1.0f, -1.0f, -1.0f, -1.0f));
    }

    inline Quaternion Quaternion::Conjugated() const
    {
        Quaternion res(*this);
        res.Conjugate();

        return res;
    }

    inline void Quaternion::Inverse()
    {
        float sqLen = SquareLength();
        Conjugate();
        *this *= (1.0f / sqLen);
    }

    inline Quaternion Quaternion::Inversed() const
    {
        Quaternion res(*this);
        res.Inverse();

        return res;
    }

    inline Vector3 Quaternion::RotateVector(const Vector3& vec) const
    {
        // v' = v + 2w(u x v) + 2(u x (u x v)), where u is the vector part.
        Float4 f = GetFloat4();
        Vector3 u(f.x, f.y, f.z);

        Vector3 t = Vector3::Cross(u, vec) * 2.0f;
        return vec + t * f.w + Vector3::Cross(u, t);
    }

    inline Matrix Quaternion::ToRotationMatrix() const
    {
        // Row-vector convention (v' = v * M), matching MatrixUtility::GetRotationAxis().

        /*
            1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
            2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx), 0.0f,
            2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        */

        __m128 xxxx = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(0, 0, 0, 0));
        __m128 yyyy = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(1, 1, 1, 1));
        __m128 zzzz = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(2, 2, 2, 2));

        __m128 yxwz = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(2, 3, 0, 1));
        __m128 wzyx = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(0, 1, 2, 3));
        __m128 zwxy = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(1, 0, 3, 2));

        // Sign + 2x multiplier.
        __m128 pnpn = _mm_set_ps(-2.0f, 2.0f, -2.0f, 2.0f);
        __m128 npnp = _mm_shuffle_ps(pnpn, pnpn, _MM_SHUFFLE(0, 1, 0, 1));
        __m128 pnnp = _mm_shuffle_ps(pnpn, pnpn, _MM_SHUFFLE(0, 1, 1, 0));
        __m128 nppn = _mm_shuffle_ps(pnpn, pnpn, _MM_SHUFFLE(1, 0, 0, 1));
        __m128 nnpp = _mm_shuffle_ps(pnpn, pnpn, _MM_SHUFFLE(0, 0, 1, 1));
        __m128 ppnn = _mm_shuffle_ps(pnpn, pnpn, _MM_SHUFFLE(1, 1, 0, 0));

        // (-yy-zz, yx+zw, -yw+zx, yz-zy(->0))
        Vector4 row0;
        row0.mData = _mm_add_ps(_mm_mul_ps(yyyy, _mm_mul_ps(npnp, yxwz)), _mm_mul_ps(zzzz, _mm_mul_ps(nppn, zwxy)));
        // (xy-zw, -xx-zz, xw+zy, -xz+zx(->0))
        Vector4 row1;
        row1.mData = _mm_add_ps(_mm_mul_ps(xxxx, _mm_mul_ps(pnpn, yxwz)), _mm_mul_ps(zzzz, _mm_mul_ps(nnpp, wzyx)));
        // (xz+yw, -xw+yz, -xx-yy, xy-yx(->0))
        Vector4 row2;
        row2.mData = _mm_add_ps(_mm_mul_ps(xxxx, _mm_mul_ps(pnnp, zwxy)), _mm_mul_ps(yyyy, _mm_mul_ps(ppnn, wzyx)));

        Matrix res = Matrix::Identity();
        res.mRows[0] += row0;
        res.mRows[1] += row1;
        res.mRows[2] += row2;

        return res;
    }

    inline Quaternion Quaternion::Lerp(const Quaternion& a, const Quaternion& b, float t)
    {
        // Take the shortest path.
        Quaternion bb = (a.Dot(b) < 0.0f) ? -b : b;

        Quaternion res = a * (1.0f - t) + bb * t;
        return res.Normalized();
    }

    inline Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t)
    {
        float cosTheta = a.Dot(b);

        // Take the shortest path.
        Quaternion bb = b;
        if (cosTheta < 0.0f)
        {
            bb = -b;
            cosTheta = -cosTheta;
        }

        // Fall back to normalized lerp when the quaternions are nearly identical.
        if (cosTheta > 0.9995f)
        {
            return Lerp(a, bb, t);
        }

        cosTheta = Math::Min(cosTheta, 1.0f);
        float sinTheta = Math::Sqrt(1.0f - cosTheta * cosTheta);
        float theta = Math::Atan2(sinTheta, cosTheta);

        float wa = Math::Sin((1.0f - t) * theta) / sinTheta;
        float wb = Math::Sin(t * theta) / sinTheta;

        return a * wa + bb * wb;
    }

    inline Quaternion operator*(float lhs, const Quaternion& rhs)
    {
        return rhs * lhs;
    }
} // namespace cube
