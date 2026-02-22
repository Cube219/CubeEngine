#define CUBE_VECTOR_IMPLEMENTATION

#include "../Vector.h"

namespace cube
{
    // Helper functions
    namespace NEON
    {
        namespace internal
        {
            template <int N>
            inline float32x4_t GetSum(float32x4_t data)
            {
                if constexpr (N == 2)
                {
                    // (x, y, ?, ?)
                    float32x4_t rdata = vrev64q_f32(data); // (y, x, ?, ?)
                    return vaddq_f32(data, rdata); // (x+y, x+y, ?, ?)
                }
                if constexpr (N == 3)
                {
                    // (x, y, z, ?)
                    data = vsetq_lane_f32(0.0f, data, 3); // (x, y, z, 0)

                    float32x4_t rdata = vrev64q_f32(data); // (y, x, 0, z)
                    float32x4_t v1 = vaddq_f32(data, rdata); // (x+y, x+y, z, z)
                    float32x4_t v2 = vextq_f32(v1, v1, 2); // (z, z, x+y, x+y)
                    return vaddq_f32(v1, v2); // (x+y+z, ...)
                }
                if constexpr (N == 4)
                {
                    // (x, y, z, w)
                    float32x4_t rdata = vrev64q_f32(data); // (y, x, w, z)
                    float32x4_t v1 = vaddq_f32(data, rdata); // (x+y, x+y, z+w, z+w)
                    float32x4_t v2 = vextq_f32(v1, v1, 2); // (z+w, z+w, x+y, x+y)
                    return vaddq_f32(v1, v2); // (x+y+z+w, ...)
                }
                return {};
            }
        } // namespace internal
    } // namespace NEON

    template <int N>
    inline VectorBase<N> VectorBase<N>::Zero()
    {
        VectorBase res;
        res.mData = vdupq_n_f32(0.0f);

        return res;
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float v)
    {
        mData = vdupq_n_f32(v);
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float x, float y)
    {
        alignas(16) float tmp[4] = {x, y, 0.0f, 0.0f};
        mData = vld1q_f32(tmp);
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float x, float y, float z)
    {
        alignas(16) float tmp[4] = {x, y, z, 0.0f};
        mData = vld1q_f32(tmp);
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float x, float y, float z, float w)
    {
        alignas(16) float tmp[4] = {x, y, z, w};
        mData = vld1q_f32(tmp);
    }

    template <int N>
    inline VectorBase<N>::VectorBase(const VectorBase& other)
    {
        mData = other.mData;
    }

    template <int N>
    template <int M>
    inline VectorBase<N>::VectorBase(const VectorBase<M>& other)
    {
        if constexpr (N <= M)
        {
            mData = other.mData;
        }
        else
        {
            // N > M
            if constexpr (M == 2)
            {
                // 2->3, 2->4
                float32x4_t zero = vdupq_n_f32(0.0f);
                mData = vcombine_f32(vget_low_f32(other.mData), vget_low_f32(zero));
            }
            if constexpr (M == 3)
            {
                // 3->4
                mData = vsetq_lane_f32(0.0f, other.mData, 3);
            }
        }
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator=(const VectorBase& rhs)
    {
        mData = rhs.mData;

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator=(float rhs)
    {
        mData = vdupq_n_f32(rhs);

        return *this;
    }

    template <int N>
    inline bool VectorBase<N>::operator==(const VectorBase& rhs) const
    {
        uint32x4_t cmp = vceqq_f32(mData, rhs.mData);
        if constexpr (N == 2)
        {
            return vminv_u32(vget_low_u32(cmp)) == 0xFFFFFFFF;
        }
        if constexpr (N == 3)
        {
            uint32x4_t masked = vsetq_lane_u32(0xFFFFFFFF, cmp, 3);
            return vminvq_u32(masked) == 0xFFFFFFFF;
        }
        if constexpr (N == 4)
        {
            return vminvq_u32(cmp) == 0xFFFFFFFF;
        }
    }

    template <int N>
    inline bool VectorBase<N>::operator!=(const VectorBase& rhs) const
    {
        return !(*this == rhs);
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::operator+(const VectorBase& rhs) const
    {
        VectorBase res(*this);
        res += rhs;

        return res;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::operator-(const VectorBase& rhs) const
    {
        VectorBase<N> res(*this);
        res -= rhs;

        return res;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::operator*(float rhs) const
    {
        VectorBase res(*this);
        res *= rhs;

        return res;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::operator*(const VectorBase& rhs) const
    {
        VectorBase<N> res(*this);
        res *= rhs;

        return res;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::operator/(float rhs) const
    {
        VectorBase res(*this);
        res /= rhs;

        return res;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::operator/(const VectorBase& rhs) const
    {
        VectorBase<N> res(*this);
        res /= rhs;

        return res;
    }

    template <int N>
    inline const VectorBase<N>& VectorBase<N>::operator+() const
    {
        return *this;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::operator-() const
    {
        VectorBase res;
        res.mData = vnegq_f32(mData);

        return res;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator+=(const VectorBase& rhs)
    {
        mData = vaddq_f32(mData, rhs.mData);

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator-=(const VectorBase& rhs)
    {
        mData = vsubq_f32(mData, rhs.mData);

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator*=(float rhs)
    {
        mData = vmulq_n_f32(mData, rhs);

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator*=(const VectorBase& rhs)
    {
        mData = vmulq_f32(mData, rhs.mData);

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator/=(float rhs)
    {
        mData = vdivq_f32(mData, vdupq_n_f32(rhs));

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator/=(const VectorBase& rhs)
    {
        mData = vdivq_f32(mData, rhs.mData);

        return *this;
    }

    template <int N>
    inline void VectorBase<N>::Swap(VectorBase& other)
    {
        float32x4_t tmp = mData;
        mData = other.mData;
        other.mData = tmp;
    }

    template <int N>
    inline void VectorBase<N>::Swap(VectorBase& lhs, VectorBase& rhs)
    {
        lhs.Swap(rhs);
    }

    template <int N>
    inline Float2 VectorBase<N>::GetFloat2() const
    {
        float f[4];
        vst1q_f32(f, mData);

        Float2 res;
        res.x = f[0];
        res.y = f[1];

        return res;
    }

    template <int N>
    inline Float3 VectorBase<N>::GetFloat3() const
    {
        float f[4];
        vst1q_f32(f, mData);

        Float3 res;
        res.x = f[0];
        res.y = f[1];
        if constexpr (N >= 3)
        {
            res.z = f[2];
        }
        else
        {
            res.z = 0.0f;
        }

        return res;
    }

    template <int N>
    inline Float4 VectorBase<N>::GetFloat4() const
    {
        float f[4];
        vst1q_f32(f, mData);

        Float4 res;
        res.x = f[0];
        res.y = f[1];
        if constexpr (N >= 3)
        {
            res.z = f[2];
        }
        else
        {
            res.z = 0.0f;
        }
        if constexpr (N >= 4)
        {
            res.w = f[3];
        }
        else
        {
            res.w = 0.0f;
        }

        return res;
    }

    template <int N>
    inline float VectorBase<N>::Length() const
    {
        return sqrt(SquareLength());
    }

    template <int N>
    inline float VectorBase<N>::SquareLength() const
    {
        float32x4_t sum = NEON::internal::GetSum<N>(vmulq_f32(mData, mData));
        return vgetq_lane_f32(sum, 0);
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::LengthV() const
    {
        VectorBase res;
        res.mData = vsqrtq_f32(SquareLengthV().mData);
        return res;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::SquareLengthV() const
    {
        float32x4_t sum = NEON::internal::GetSum<N>(vmulq_f32(mData, mData));

        VectorBase r;
        r.mData = vdupq_laneq_f32(sum, 0);
        return r;
    }

    template <int N>
    inline void VectorBase<N>::Normalize()
    {
        float32x4_t squareLen = NEON::internal::GetSum<N>(vmulq_f32(mData, mData));

        // Reciprocal sqrt estimate + two Newton-Raphson refinement steps
        float32x4_t rsqrt = vrsqrteq_f32(squareLen);
        rsqrt = vmulq_f32(rsqrt, vrsqrtsq_f32(vmulq_f32(squareLen, rsqrt), rsqrt));
        rsqrt = vmulq_f32(rsqrt, vrsqrtsq_f32(vmulq_f32(squareLen, rsqrt), rsqrt));

        mData = vmulq_f32(mData, rsqrt);
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::Normalized()
    {
        VectorBase res(*this);
        res.Normalize();

        return res;
    }

    template <int N>
    inline float VectorBase<N>::Dot(const VectorBase& rhs) const
    {
        float32x4_t res = NEON::internal::GetSum<N>(vmulq_f32(mData, rhs.mData));
        return vgetq_lane_f32(res, 0);
    }

    template <int N>
    inline float VectorBase<N>::Dot(const VectorBase& lhs, const VectorBase& rhs)
    {
        return lhs.Dot(rhs);
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::DotV(const VectorBase& rhs) const
    {
        float32x4_t res = NEON::internal::GetSum<N>(vmulq_f32(mData, rhs.mData));

        VectorBase r;
        r.mData = vdupq_laneq_f32(res, 0);
        return r;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::DotV(const VectorBase& lhs, const VectorBase& rhs)
    {
        return lhs.DotV(rhs);
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::Cross(const VectorBase& rhs) const
    {
        static_assert(N == 3, "Cross product can be used in Vector3 only.");

        VectorBase res;

        float32x4_t tmp = vextq_f32(mData, mData, 3); // (?, x1, y1, z1)
        float32x4_t l1 = vextq_f32(tmp, mData, 2); // (y1, z1, x1, y1)
        float32x4_t l2 = vextq_f32(l1, l1, 1); // (z1, x1, y1, y1);

        tmp = vextq_f32(rhs.mData, rhs.mData, 3); // (?, x2, y2, z2)
        float32x4_t r1 = vextq_f32(tmp, rhs.mData, 2); // (y2, z2, x2, y2)
        float32x4_t r2 = vextq_f32(r1, r1, 1); // (z2, x2, y2, y2);

        // (l1*r2) - (l2*r1)
        res.mData = vfmsq_f32(vmulq_f32(l1, r2), l2, r1);

        return res;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::Cross(const VectorBase& lhs, const VectorBase& rhs)
    {
        return lhs.Cross(rhs);
    }

    template <int N>
    inline VectorBase<N> operator*(float lhs, const VectorBase<N>& rhs)
    {
        VectorBase res(rhs);
        res *= lhs;

        return res;
    }

    template <int N>
    inline VectorBase<N> operator/(float lhs, const VectorBase<N>& rhs)
    {
        VectorBase<N> res(lhs);
        res /= rhs;

        return res;
    }
} // namespace cube
