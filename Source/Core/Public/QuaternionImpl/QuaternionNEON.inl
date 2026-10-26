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
        Vector3 a = axis.Normalized();

        float half = angle * 0.5f;
        float s = Math::Sin(half);
        float c = Math::Cos(half);

        Quaternion res;
        res.mData = vmulq_n_f32(a.mData, s);
        res.mData = vsetq_lane_f32(c, res.mData, 3);

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

        // Derive quaternion components from ToRotationMatrix().
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
        // Hamilton product. result = w1*q2 + x1*(w2,-z2,y2,-x2) + y1*(z2,w2,-x2,-y2) + z1*(-y2,x2,w2,-z2)

        float32x4_t x1 = vdupq_laneq_f32(mData, 0);
        float32x4_t y1 = vdupq_laneq_f32(mData, 1);
        float32x4_t z1 = vdupq_laneq_f32(mData, 2);
        float32x4_t w1 = vdupq_laneq_f32(mData, 3);

        float32x4_t q2_2 = vextq_f32(rhs.mData, rhs.mData, 2); // (z2, w2, x2, y2)
        float32x4_t q2_1 = vrev64q_f32(q2_2);                  // (w2, z2, y2, x2)
        float32x4_t q2_3 = vrev64q_f32(rhs.mData);             // (y2, x2, w2, z2)

        const float32x2_t pp = vdup_n_f32(1.0f);
        const float32x2_t nn = vneg_f32(pp);
        const float32x4_t ppnn = vcombine_f32(pp, nn);
        const float32x4_t nppn = vextq_f32(ppnn, ppnn, 3);
        const float32x2_t pn = vget_high_f32(nppn);
        const float32x4_t pnpn = vcombine_f32(pn, pn);

        float32x4_t res = vmulq_f32(w1, rhs.mData);
        res = vfmaq_f32(res, x1, vmulq_f32(q2_1, pnpn));
        res = vfmaq_f32(res, y1, vmulq_f32(q2_2, ppnn));
        res = vfmaq_f32(res, z1, vmulq_f32(q2_3, nppn));

        mData = res;

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
        float32x4_t nnnp = vsetq_lane_f32(1.0f, vdupq_n_f32(-1.0f), 3);
        mData = vmulq_f32(mData, nnnp);
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

        float32x4_t xxxx = vdupq_laneq_f32(mData, 0);
        float32x4_t yyyy = vdupq_laneq_f32(mData, 1);
        float32x4_t zzzz = vdupq_laneq_f32(mData, 2);

        float32x4_t yxwz = vrev64q_f32(mData);
        float32x4_t wzyx = vextq_f32(yxwz, yxwz, 2);
        float32x4_t zwxy = vrev64q_f32(wzyx);

        // Sign + 2x multiplier.
        const float32x2_t pp = vdup_n_f32(2.0f);
        const float32x2_t nn = vneg_f32(pp);
        const float32x4_t ppnn = vcombine_f32(pp, nn);
        const float32x4_t nnpp = vnegq_f32(ppnn);
        const float32x4_t nppn = vextq_f32(ppnn, ppnn, 3);
        const float32x4_t pnnp = vrev64q_f32(nppn);
        const float32x4_t pnpn = vextq_f32(nppn, pnnp, 2);
        const float32x4_t npnp = vrev64q_f32(pnpn);

        // (-yy-zz, yx+zw, -yw+zx, yz-zy(->0))
        Vector4 row0;
        row0.mData = vfmaq_f32(vmulq_f32(yyyy, vmulq_f32(npnp, yxwz)), zzzz, vmulq_f32(nppn, zwxy));
        // (xy-zw, -xx-zz, xw+zy, -xz+zx(->0))
        Vector4 row1;
        row1.mData = vfmaq_f32(vmulq_f32(xxxx, vmulq_f32(pnpn, yxwz)), zzzz, vmulq_f32(nnpp, wzyx));
        // (xz+yw, -xw+yz, -xx-yy, xy-yx(->0))
        Vector4 row2;
        row2.mData = vfmaq_f32(vmulq_f32(xxxx, vmulq_f32(pnnp, zwxy)), yyyy, vmulq_f32(ppnn, wzyx));

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
