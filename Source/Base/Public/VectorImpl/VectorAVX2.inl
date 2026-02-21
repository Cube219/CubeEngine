#define CUBE_VECTOR_IMPLEMENTATION

#include "../Vector.h"

namespace cube
{
    // Helper functions
    namespace AVX2
    {
        namespace internal
        {
            template <int N>
            inline __m128 GetSum(__m128 data)
            {
                if constexpr (N == 2)
                {
                    // data = x / y / ? / ?

                    // hadd: (x+y, ?+?, x+y, ?+?) -- element 0 = x+y
                    return _mm_hadd_ps(data, data);
                }
                if constexpr (N == 3)
                {
                    // data = x / y / z / ?

                    // tmp = y / z / x / ?
                    __m128 tmp = _mm_shuffle_ps(data, data, _MM_SHUFFLE(0, 0, 2, 1));
                    // tmp2 = z / x / y / ?
                    __m128 tmp2 = _mm_shuffle_ps(data, data, _MM_SHUFFLE(0, 1, 0, 2));
                    // x+y+z / x+y+z / x+y+z / ?
                    return _mm_add_ps(_mm_add_ps(data, tmp), tmp2);
                }
                if constexpr (N == 4)
                {
                    // data = x / y / z / w

                    // hadd: (x+y, z+w, x+y, z+w)
                    __m128 tmp = _mm_hadd_ps(data, data);
                    // hadd: (x+y+z+w, x+y+z+w, x+y+z+w, x+y+z+w)
                    return _mm_hadd_ps(tmp, tmp);
                }
                return {};
            }
        } // namespace internal
    } // namespace AVX2

    template <int N>
    inline VectorBase<N> VectorBase<N>::Zero()
    {
        VectorBase res;
        res.mData = _mm_setzero_ps();

        return res;
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float v)
    {
        mData = _mm_set1_ps(v);
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float x, float y)
    {
        mData = _mm_set_ps(0, 0, y, x);
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float x, float y, float z)
    {
        mData = _mm_set_ps(0, z, y, x);
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float x, float y, float z, float w)
    {
        mData = _mm_set_ps(w, z, y, x);
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
            __m128 zero = _mm_setzero_ps();

            if constexpr (M == 2)
            {
                // 2->3, 2->4
                mData = _mm_shuffle_ps(other.mData, zero, _MM_SHUFFLE(0, 0, 1, 0));
            }
            if constexpr (M == 3)
            {
                // 3->4
                __m128 tmp;
                // tmp = z / z / 0 / 0
                tmp = _mm_shuffle_ps(other.mData, zero, _MM_SHUFFLE(0, 0, 2, 2));
                // mData = x / y / z / 0
                mData = _mm_shuffle_ps(other.mData, tmp, _MM_SHUFFLE(2, 0, 1, 0));
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
        mData = _mm_set1_ps(rhs);

        return *this;
    }

    template <int N>
    inline bool VectorBase<N>::operator==(const VectorBase& rhs) const
    {
        __m128 res = _mm_cmpeq_ps(mData, rhs.mData);
        if constexpr (N == 2)
        {
            return (_mm_movemask_ps(res) & 0b0011) == 0b0011;
        }
        if constexpr (N == 3)
        {
            return (_mm_movemask_ps(res) & 0b0111) == 0b0111;
        }
        if constexpr (N == 4)
        {
            return (_mm_movemask_ps(res) & 0b1111) == 0b1111;
        }

        return false;
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
        res.mData = _mm_xor_ps(mData, _mm_set1_ps(-0.0f));

        return res;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator+=(const VectorBase& rhs)
    {
        mData = _mm_add_ps(mData, rhs.mData);

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator-=(const VectorBase& rhs)
    {
        mData = _mm_sub_ps(mData, rhs.mData);

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator*=(float rhs)
    {
        mData = _mm_mul_ps(mData, _mm_set1_ps(rhs));

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator*=(const VectorBase& rhs)
    {
        mData = _mm_mul_ps(mData, rhs.mData);

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator/=(float rhs)
    {
        mData = _mm_div_ps(mData, _mm_set1_ps(rhs));

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator/=(const VectorBase& rhs)
    {
        mData = _mm_div_ps(mData, rhs.mData);

        return *this;
    }

    template <int N>
    inline void VectorBase<N>::Swap(VectorBase& other)
    {
        std::swap(mData, other.mData);
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
        _mm_store_ps(f, mData);

        Float2 res;
        res.x = f[0];
        res.y = f[1];

        return res;
    }

    template <int N>
    inline Float3 VectorBase<N>::GetFloat3() const
    {
        float f[4];
        _mm_store_ps(f, mData);

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
        _mm_store_ps(f, mData);

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
        __m128 res;
        if constexpr (N == 2) { res = _mm_dp_ps(mData, mData, 0x31); }
        if constexpr (N == 3) { res = _mm_dp_ps(mData, mData, 0x71); }
        if constexpr (N == 4) { res = _mm_dp_ps(mData, mData, 0xF1); }
        float f;
        _mm_store_ss(&f, res);

        return f;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::LengthV() const
    {
        VectorBase r;
        if constexpr (N == 2) { r.mData = _mm_sqrt_ps(_mm_dp_ps(mData, mData, 0x3F)); }
        if constexpr (N == 3) { r.mData = _mm_sqrt_ps(_mm_dp_ps(mData, mData, 0x7F)); }
        if constexpr (N == 4) { r.mData = _mm_sqrt_ps(_mm_dp_ps(mData, mData, 0xFF)); }

        return r;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::SquareLengthV() const
    {
        VectorBase r;
        if constexpr (N == 2) { r.mData = _mm_dp_ps(mData, mData, 0x3F); }
        if constexpr (N == 3) { r.mData = _mm_dp_ps(mData, mData, 0x7F); }
        if constexpr (N == 4) { r.mData = _mm_dp_ps(mData, mData, 0xFF); }

        return r;
    }

    template <int N>
    inline void VectorBase<N>::Normalize()
    {
        __m128 squareLen;
        if constexpr (N == 2) { squareLen = _mm_dp_ps(mData, mData, 0x3F); }
        if constexpr (N == 3) { squareLen = _mm_dp_ps(mData, mData, 0x7F); }
        if constexpr (N == 4) { squareLen = _mm_dp_ps(mData, mData, 0xFF); }

        // Reciprocal sqrt estimate + two Newton-Raphson refinement steps using FMA
        __m128 rsqrt = _mm_rsqrt_ps(squareLen);
        // Newton-Raphson: rsqrt = rsqrt * (1.5 - 0.5 * squareLen * rsqrt * rsqrt)
        __m128 half = _mm_set1_ps(0.5f);
        __m128 threeHalf = _mm_set1_ps(1.5f);
        __m128 halfSqLen = _mm_mul_ps(half, squareLen);
        // Refinement step 1
        // threeHalf - halfSqLen * rsqrt * rsqrt = fnmadd(halfSqLen, rsqrt*rsqrt, threeHalf)
        __m128 rsqrtSq = _mm_mul_ps(rsqrt, rsqrt);
        rsqrt = _mm_mul_ps(rsqrt, _mm_fnmadd_ps(halfSqLen, rsqrtSq, threeHalf));
        // Refinement step 2
        rsqrtSq = _mm_mul_ps(rsqrt, rsqrt);
        rsqrt = _mm_mul_ps(rsqrt, _mm_fnmadd_ps(halfSqLen, rsqrtSq, threeHalf));

        mData = _mm_mul_ps(mData, rsqrt);
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
        __m128 res;
        if constexpr (N == 2) { res = _mm_dp_ps(mData, rhs.mData, 0x31); }
        if constexpr (N == 3) { res = _mm_dp_ps(mData, rhs.mData, 0x71); }
        if constexpr (N == 4) { res = _mm_dp_ps(mData, rhs.mData, 0xF1); }
        float f;
        _mm_store_ss(&f, res);

        return f;
    }

    template <int N>
    inline float VectorBase<N>::Dot(const VectorBase& lhs, const VectorBase& rhs)
    {
        return lhs.Dot(rhs);
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::DotV(const VectorBase& rhs) const
    {
        VectorBase r;
        if constexpr (N == 2) { r.mData = _mm_dp_ps(mData, rhs.mData, 0x3F); }
        if constexpr (N == 3) { r.mData = _mm_dp_ps(mData, rhs.mData, 0x7F); }
        if constexpr (N == 4) { r.mData = _mm_dp_ps(mData, rhs.mData, 0xFF); }

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

        // y1 / z1 / x1 / ??
        __m128 l1 = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(0, 0, 2, 1));
        // z2 / x2 / y2 / ??
        __m128 r1 = _mm_shuffle_ps(rhs.mData, rhs.mData, _MM_SHUFFLE(0, 1, 0, 2));

        // z1 / x1 / y1 / ??
        __m128 l2 = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(0, 1, 0, 2));
        // y2 / z2 / x2 / ??
        __m128 r2 = _mm_shuffle_ps(rhs.mData, rhs.mData, _MM_SHUFFLE(0, 0, 2, 1));

        // (l1*r1) - (l2*r2) = fnmadd(l2, r2, l1*r1)
        res.mData = _mm_fnmadd_ps(l2, r2, _mm_mul_ps(l1, r1));

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
