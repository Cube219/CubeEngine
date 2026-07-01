#define CUBE_QUATERNION_IMPLEMENTATION

#include "../Quaternion.h"

#include "../CubeMath.h"

namespace cube
{
    inline Quaternion Quaternion::Zero()
    {
        return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
    }

    inline Quaternion Quaternion::Identity()
    {
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }

    inline Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angle)
    {
        Vector3 a = axis;
        a.Normalize();
        Float3 af = a.GetFloat3();

        float half = angle * 0.5f;
        float s = Math::Sin(half);
        float c = Math::Cos(half);

        return Quaternion(af.x * s, af.y * s, af.z * s, c);
    }

    inline Quaternion Quaternion::FromEulerXYZ(float xAngle, float yAngle, float zAngle)
    {
        Quaternion qx = FromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), xAngle);
        Quaternion qy = FromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), yAngle);
        Quaternion qz = FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), zAngle);

        // Matches MatrixUtility::GetRotationXYZ, which equals (Mz*My*Mx)^T in the
        // row-vector convention, i.e. the conjugate of (qx * qy * qz).
        return (qx * qy * qz).Conjugated();
    }

    inline Quaternion Quaternion::FromEulerXYZ(const Vector3& angles)
    {
        Float3 f = angles.GetFloat3();
        return FromEulerXYZ(f.x, f.y, f.z);
    }

    inline Quaternion Quaternion::FromRotationMatrix(const Matrix& matrix)
    {
        // matrix uses the row-vector convention (M = R^T of the textbook column-vector
        // rotation matrix), so R[i][j] = m[j][i]. (See ToRotationMatrix)
        Float4 r0 = matrix.GetRow(0).GetFloat4();
        Float4 r1 = matrix.GetRow(1).GetFloat4();
        Float4 r2 = matrix.GetRow(2).GetFloat4();

        float m00 = r0.x, m01 = r0.y, m02 = r0.z;
        float m10 = r1.x, m11 = r1.y, m12 = r1.z;
        float m20 = r2.x, m21 = r2.y, m22 = r2.z;

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
        mData[0] = x;
        mData[1] = y;
        mData[2] = z;
        mData[3] = w;
    }

    inline Quaternion::Quaternion(const Quaternion& other)
    {
        mData[0] = other.mData[0];
        mData[1] = other.mData[1];
        mData[2] = other.mData[2];
        mData[3] = other.mData[3];
    }

    inline Quaternion& Quaternion::operator=(const Quaternion& rhs)
    {
        mData[0] = rhs.mData[0];
        mData[1] = rhs.mData[1];
        mData[2] = rhs.mData[2];
        mData[3] = rhs.mData[3];

        return *this;
    }

    inline bool Quaternion::operator==(const Quaternion& rhs) const
    {
        bool res = true;
        res &= (abs(mData[0] - rhs.mData[0]) < FLOAT_EPS);
        res &= (abs(mData[1] - rhs.mData[1]) < FLOAT_EPS);
        res &= (abs(mData[2] - rhs.mData[2]) < FLOAT_EPS);
        res &= (abs(mData[3] - rhs.mData[3]) < FLOAT_EPS);

        return res;
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
        float x1 = mData[0], y1 = mData[1], z1 = mData[2], w1 = mData[3];
        float x2 = rhs.mData[0], y2 = rhs.mData[1], z2 = rhs.mData[2], w2 = rhs.mData[3];

        // Hamilton product.
        return Quaternion(
            w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2,
            w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2,
            w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2,
            w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2);
    }

    inline const Quaternion& Quaternion::operator+() const
    {
        return *this;
    }

    inline Quaternion Quaternion::operator-() const
    {
        return Quaternion(-mData[0], -mData[1], -mData[2], -mData[3]);
    }

    inline Quaternion& Quaternion::operator+=(const Quaternion& rhs)
    {
        mData[0] += rhs.mData[0];
        mData[1] += rhs.mData[1];
        mData[2] += rhs.mData[2];
        mData[3] += rhs.mData[3];

        return *this;
    }

    inline Quaternion& Quaternion::operator-=(const Quaternion& rhs)
    {
        mData[0] -= rhs.mData[0];
        mData[1] -= rhs.mData[1];
        mData[2] -= rhs.mData[2];
        mData[3] -= rhs.mData[3];

        return *this;
    }

    inline Quaternion& Quaternion::operator*=(float rhs)
    {
        mData[0] *= rhs;
        mData[1] *= rhs;
        mData[2] *= rhs;
        mData[3] *= rhs;

        return *this;
    }

    inline Quaternion& Quaternion::operator*=(const Quaternion& rhs)
    {
        *this = *this * rhs;

        return *this;
    }

    inline Float4 Quaternion::GetFloat4() const
    {
        Float4 res;
        res.x = mData[0];
        res.y = mData[1];
        res.z = mData[2];
        res.w = mData[3];

        return res;
    }

    inline float Quaternion::Length() const
    {
        return Math::Sqrt(SquareLength());
    }

    inline float Quaternion::SquareLength() const
    {
        return mData[0] * mData[0] + mData[1] * mData[1] + mData[2] * mData[2] + mData[3] * mData[3];
    }

    inline void Quaternion::Normalize()
    {
        *this *= (1.0f / Length());
    }

    inline Quaternion Quaternion::Normalized() const
    {
        Quaternion res(*this);
        res.Normalize();

        return res;
    }

    inline float Quaternion::Dot(const Quaternion& rhs) const
    {
        return mData[0] * rhs.mData[0] + mData[1] * rhs.mData[1] + mData[2] * rhs.mData[2] + mData[3] * rhs.mData[3];
    }

    inline float Quaternion::Dot(const Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs.Dot(rhs);
    }

    inline void Quaternion::Conjugate()
    {
        // Negate the vector part (x, y, z), keep the scalar part (w).
        mData[0] = -mData[0];
        mData[1] = -mData[1];
        mData[2] = -mData[2];
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
        Vector3 u(mData[0], mData[1], mData[2]);

        Vector3 t = Vector3::Cross(u, vec) * 2.0f;
        return vec + t * mData[3] + Vector3::Cross(u, t);
    }

    inline Matrix Quaternion::ToRotationMatrix() const
    {
        // Row-vector convention (v' = v * M), matching MatrixUtility::GetRotationAxis().
        float x = mData[0], y = mData[1], z = mData[2], w = mData[3];

        float xx = x * x, yy = y * y, zz = z * z;
        float xy = x * y, xz = x * z, yz = y * z;
        float wx = w * x, wy = w * y, wz = w * z;

        return Matrix{
            1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
            2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx), 0.0f,
            2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    inline Quaternion Quaternion::Lerp(const Quaternion& a, const Quaternion& b, float t)
    {
        return a + (b - a) * t;
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
