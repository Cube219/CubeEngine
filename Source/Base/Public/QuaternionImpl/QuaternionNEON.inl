#define CUBE_QUATERNION_IMPLEMENTATION

#include "../Quaternion.h"

#include "../CubeMath.h"

namespace cube
{
    inline Quaternion Quaternion::Zero()
    {
        Quaternion res;
        res.mData = vdupq_n_f32(0.0f);

        return res;
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

        // Matches MatrixUtility::GetRotationXYZ, which equals Mx*My*Mz in the
        // row-vector convention (GetRotationX() * GetRotationY() * GetRotationZ()).
        return qz * qy * qx;
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
        alignas(16) float tmp[4] = {x, y, z, w};
        mData = vld1q_f32(tmp);
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
        uint32x4_t cmp = vceqq_f32(mData, rhs.mData);
        return vminvq_u32(cmp) == 0xFFFFFFFF;
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
        // Hamilton product. result = w1*q2 + x1*(w2,-z2,y2,-x2) + y1*(z2,w2,-x2,-y2) + z1*(-y2,x2,w2,-z2)
        alignas(16) float a[4];
        alignas(16) float b[4];
        vst1q_f32(a, mData);     // (x1, y1, z1, w1)
        vst1q_f32(b, rhs.mData); // (x2, y2, z2, w2)

        float32x4_t x1 = vdupq_n_f32(a[0]);
        float32x4_t y1 = vdupq_n_f32(a[1]);
        float32x4_t z1 = vdupq_n_f32(a[2]);
        float32x4_t w1 = vdupq_n_f32(a[3]);

        alignas(16) float t1[4] = {b[3], b[2], b[1], b[0]}; // (w2, z2, y2, x2)
        alignas(16) float t2[4] = {b[2], b[3], b[0], b[1]}; // (z2, w2, x2, y2)
        alignas(16) float t3[4] = {b[1], b[0], b[3], b[2]}; // (y2, x2, w2, z2)
        float32x4_t q2_1 = vld1q_f32(t1);
        float32x4_t q2_2 = vld1q_f32(t2);
        float32x4_t q2_3 = vld1q_f32(t3);

        alignas(16) float s1[4] = {1.0f, -1.0f, 1.0f, -1.0f};  // (x,y,z,w) = (+,-,+,-)
        alignas(16) float s2[4] = {1.0f, 1.0f, -1.0f, -1.0f};  // (+,+,-,-)
        alignas(16) float s3[4] = {-1.0f, 1.0f, 1.0f, -1.0f};  // (-,+,+,-)
        float32x4_t sign1 = vld1q_f32(s1);
        float32x4_t sign2 = vld1q_f32(s2);
        float32x4_t sign3 = vld1q_f32(s3);

        float32x4_t res = vmulq_f32(w1, rhs.mData);
        res = vfmaq_f32(res, x1, vmulq_f32(q2_1, sign1));
        res = vfmaq_f32(res, y1, vmulq_f32(q2_2, sign2));
        res = vfmaq_f32(res, z1, vmulq_f32(q2_3, sign3));

        Quaternion q;
        q.mData = res;

        return q;
    }

    inline const Quaternion& Quaternion::operator+() const
    {
        return *this;
    }

    inline Quaternion Quaternion::operator-() const
    {
        Quaternion res;
        res.mData = vnegq_f32(mData);

        return res;
    }

    inline Quaternion& Quaternion::operator+=(const Quaternion& rhs)
    {
        mData = vaddq_f32(mData, rhs.mData);

        return *this;
    }

    inline Quaternion& Quaternion::operator-=(const Quaternion& rhs)
    {
        mData = vsubq_f32(mData, rhs.mData);

        return *this;
    }

    inline Quaternion& Quaternion::operator*=(float rhs)
    {
        mData = vmulq_n_f32(mData, rhs);

        return *this;
    }

    inline Quaternion& Quaternion::operator*=(const Quaternion& rhs)
    {
        *this = *this * rhs;

        return *this;
    }

    inline Float4 Quaternion::GetFloat4() const
    {
        float f[4];
        vst1q_f32(f, mData);

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
        return vaddvq_f32(vmulq_f32(mData, mData));
    }

    inline void Quaternion::Normalize()
    {
        float invLen = 1.0f / Length();
        mData = vmulq_n_f32(mData, invLen);
    }

    inline Quaternion Quaternion::Normalized() const
    {
        Quaternion res(*this);
        res.Normalize();

        return res;
    }

    inline float Quaternion::Dot(const Quaternion& rhs) const
    {
        return vaddvq_f32(vmulq_f32(mData, rhs.mData));
    }

    inline float Quaternion::Dot(const Quaternion& lhs, const Quaternion& rhs)
    {
        return lhs.Dot(rhs);
    }

    inline void Quaternion::Conjugate()
    {
        alignas(16) float s[4] = {-1.0f, -1.0f, -1.0f, 1.0f};

        // Negate the vector part (x, y, z), keep the scalar part (w).
        mData = vmulq_f32(mData, vld1q_f32(s));
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
        Float4 f = GetFloat4();
        float x = f.x, y = f.y, z = f.z, w = f.w;

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
