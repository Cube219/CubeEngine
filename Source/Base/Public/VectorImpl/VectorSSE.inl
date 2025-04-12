#define CUBE_VECTOR_IMPLEMENTATION

#include "../Vector.h"

namespace cube
{
    // Helper functions
    namespace SSE
    {
        namespace internal
        {
            template <int N>
            inline VectorData<N> GetSum(VectorData<N> data)
            {
                if constexpr (N == 2)
                {
                    // data = x / y / ? / ?

                    // tmp = y / y / y / y
                    VectorData<N> tmp = _mm_shuffle_ps(data, data, _MM_SHUFFLE(1, 1, 1, 1));
                    // x+y / ? / ? / ?
                    return _mm_add_ss(tmp, data);
                }
                if constexpr (N == 3)
                {
                    // data = x / y / z / ?

                    // tmp = y / y / y / y
                    VectorData<N> tmp = _mm_shuffle_ps(data, data, _MM_SHUFFLE(1, 1, 1, 1));
                    // tmp = x+y / ? / ? / ?
                    tmp = _mm_add_ss(tmp, data);
                    // tmp2 = z / z / z / z
                    VectorData<N> tmp2 = _mm_shuffle_ps(data, data, _MM_SHUFFLE(2, 2, 2, 2));
                    // x+y+z / ? / ? / ?
                    return _mm_add_ss(tmp, tmp2);
                }
                if constexpr (N == 4)
                {
                    // data = x / y / z / w

                    // tmp = y / y / w / w
                    VectorData<N> tmp = _mm_shuffle_ps(data, data, _MM_SHUFFLE(3, 3, 1, 1));
                    // tmp = x+y / ? / z+w / ?
                    tmp = _mm_add_ps(tmp, data);
                    // tmp2 = z+w / z+w / z+w / z+w
                    VectorData<N> tmp2 = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2, 2, 2, 2));
                    // x+y+z+w / ? / ? / ?
                    return _mm_add_ss(tmp, tmp2);
                }
                return {};
            }
        } // namespace internal
    } // namespace SSE

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
            VectorData<N> zero = _mm_setzero_ps();

            if constexpr (M == 2)
            {
                // 2->3, 2->4
                mData = _mm_shuffle_ps(other.mData, zero, _MM_SHUFFLE(0, 0, 1, 0));
            }
            if constexpr (M == 3)
            {
                // 3->4
                VectorData<N> tmp;
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
    template <int M>
    inline VectorBase<N>& VectorBase<N>::operator=(const VectorBase<M>& rhs)
    {
        if constexpr (N <= M)
        {
            mData = rhs.mData;
        }
        else
        {
            // N > M
            VectorData<N> zero = _mm_setzero_ps();

            if constexpr (M == 2)
            {
                // 2->3, 2->4
                mData = _mm_shuffle_ps(rhs.mData, zero, _MM_SHUFFLE(0, 0, 1, 0));
            }
            if constexpr (M == 3)
            {
                // 3->4
                VectorData<N> tmp;
                // tmp = z / z / 0 / 0
                tmp = _mm_shuffle_ps(rhs.mData, zero, _MM_SHUFFLE(0, 0, 2, 2));
                // mData = x / y / z / 0
                mData = _mm_shuffle_ps(rhs.mData, tmp, _MM_SHUFFLE(2, 0, 1, 0));
            }
        }

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
        VectorData<N> res = _mm_cmpeq_ps(mData, rhs.mData);
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
    template <int M>
    inline VectorBase<std::max(N, M)> VectorBase<N>::operator+(const VectorBase<M>& rhs) const
    {
        VectorBase<std::max(N, M)> res(*this);
        res += rhs;

        return res;
    }

    template <int N>
    template <int M>
    inline VectorBase<std::max(N, M)> VectorBase<N>::operator-(const VectorBase<M>& rhs) const
    {
        VectorBase<std::max(N, M)> res(*this);
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
    template <int M>
    inline VectorBase<std::max(N, M)> VectorBase<N>::operator*(const VectorBase<M>& rhs) const
    {
        VectorBase<std::max(N, M)> res(*this);
        res *= rhs;

        return res;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::operator/(float rhs) const
    {
        VectorBase res(*this);
        res *= rhs;

        return res;
    }

    template <int N>
    template <int M>
    inline VectorBase<std::max(N, M)> VectorBase<N>::operator/(const VectorBase<M>& rhs) const
    {
        VectorBase<std::max(N, M)> res(*this);
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
        VectorBase res(*this);
        res *= -1.0f;

        return res;
    }

    template <int N>
    template <int M>
    inline VectorBase<N>& VectorBase<N>::operator+=(const VectorBase<M>& rhs)
    {
        if constexpr (N <= M)
        {
            mData = _mm_add_ps(mData, rhs.mData);
        }
        else
        {
            *this += VectorBase<N>(rhs);
        }

        return *this;
    }

    template <int N>
    template <int M>
    inline VectorBase<N>& VectorBase<N>::operator-=(const VectorBase<M>& rhs)
    {
        if constexpr (N <= M)
        {
            mData = _mm_sub_ps(mData, rhs.mData);
        }
        else
        {
            *this -= VectorBase<N>(rhs);
        }

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator*=(float rhs)
    {
        mData = _mm_mul_ps(mData, _mm_set1_ps(rhs));

        return *this;
    }

    template <int N>
    template <int M>
    inline VectorBase<N>& VectorBase<N>::operator*=(const VectorBase<M>& rhs)
    {
        if constexpr (N <= M)
        {
            mData = _mm_mul_ps(mData, rhs.mData);
        }
        else
        {
            *this *= VectorBase<N>(rhs);
        }

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator/=(float rhs)
    {
        mData = _mm_div_ps(mData, _mm_set1_ps(rhs));

        return *this;
    }

    template <int N>
    template <int M>
    inline VectorBase<N>& VectorBase<N>::operator/=(const VectorBase<M>& rhs)
    {
        if constexpr (N <= M)
        {
            mData = _mm_div_ps(mData, rhs.mData);
        }
        else
        {
            *this /= VectorBase<N>(rhs);
        }

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
        VectorData<N> res = SSE::internal::GetSum<N>(_mm_mul_ps(mData, mData));
        float f[4];
        _mm_store_ps(f, res);

        return f[0];
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::LengthV() const
    {
        return _mm_sqrt_ps(SquareLengthV());
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::SquareLengthV() const
    {
        VectorData<N> res = SSE::internal::GetSum<N>(_mm_mul_ps(mData, mData));

        VectorBase r;
        r.mData = _mm_shuffle_ps(res, res, _MM_SHUFFLE(0, 0, 0, 0));
        return r;
    }

    template <int N>
    inline void VectorBase<N>::Normalize()
    {
        VectorData<N> squareLen = SSE::internal::GetSum<N>(_mm_mul_ps(mData, mData));
        // sqLen / ? / ? / ? -> len / len / len / len
        VectorData<N> len = _mm_sqrt_ps(_mm_shuffle_ps(squareLen, squareLen, _MM_SHUFFLE(0, 0, 0, 0)));

        mData = _mm_div_ps(mData, len);
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
        VectorData<N> res = SSE::internal::GetSum<N>(_mm_mul_ps(mData, rhs.mData));
        float f[4];
        _mm_store_ps(f, res);

        return f[0];
    }

    template <int N>
    inline float VectorBase<N>::Dot(const VectorBase& lhs, const VectorBase& rhs)
    {
        return lhs.Dot(rhs);
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::DotV(const VectorBase& rhs) const
    {
        VectorData<N> res = SSE::internal::GetSum<N>(_mm_mul_ps(mData, rhs.mData));

        VectorBase r;
        r.mData = _mm_shuffle_ps(res, res, _MM_SHUFFLE(0, 0, 0, 0));
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
        VectorData<N> leftMul = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(0, 0, 2, 1));
        // z2 / x2 / y2 / ??
        VectorData<N> rightMul = _mm_shuffle_ps(rhs.mData, rhs.mData, _MM_SHUFFLE(0, 1, 0, 2));

        res.mData = _mm_mul_ps(leftMul, rightMul);

        // z1 / x1 / y1 / ??
        leftMul = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(0, 1, 0, 2));
        // y2 / z2 / x2 / ??
        rightMul = _mm_shuffle_ps(rhs.mData, rhs.mData, _MM_SHUFFLE(0, 0, 2, 1));

        leftMul = _mm_mul_ps(leftMul, rightMul);
        res.mData = _mm_sub_ps(res.mData, leftMul);

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
