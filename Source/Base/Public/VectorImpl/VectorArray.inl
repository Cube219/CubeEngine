#define CUBE_VECTOR_IMPLEMENTATION

#include "../Vector.h"

namespace cube
{
    template <int N>
    inline VectorBase<N> VectorBase<N>::Zero()
    {
        return VectorBase(0.0f);
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float v)
    {
        mData[0] = v;
        mData[1] = v;
        if constexpr (N >= 3)
        {
            mData[2] = v;
        }
        if constexpr (N >= 4)
        {
            mData[3] = v;
        }
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float x, float y)
    {
        mData[0] = x;
        mData[1] = y;
        if constexpr (N >= 3)
        {
            mData[2] = 0.0f;
        }
        if constexpr (N >= 4)
        {
            mData[3] = 0.0f;
        }
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float x, float y, float z)
    {
        mData[0] = x;
        mData[1] = y;
        if constexpr (N >= 3)
        {
            mData[2] = z;
        }
        if constexpr (N >= 4)
        {
            mData[3] = 0.0f;
        }
    }

    template <int N>
    inline VectorBase<N>::VectorBase(float x, float y, float z, float w)
    {
        mData[0] = x;
        mData[1] = y;
        if constexpr (N >= 3)
        {
            mData[2] = z;
        }
        if constexpr (N >= 4)
        {
            mData[3] = w;
        }
    }

    template <int N>
    inline VectorBase<N>::VectorBase(const VectorBase& other)
    {
        mData[0] = other.mData[0];
        mData[1] = other.mData[1];
        if constexpr (N >= 3)
        {
            mData[2] = other.mData[2];
        }
        if constexpr (N >= 4)
        {
            mData[3] = other.mData[3];
        }
    }

    template <int N>
    template <int M>
    inline VectorBase<N>::VectorBase(const VectorBase<M>& other)
    {
        mData[0] = other.mData[0];
        mData[1] = other.mData[1];
        if constexpr (N >= 3)
        {
            if constexpr (M >= 3)
            {
                mData[2] = other.mData[2];
            }
            else
            {
                mData[2] = 0.0f;
            }
        }
        if constexpr (N >= 4)
        {
            if constexpr (M >= 4)
            {
                mData[3] = other.mData[3];
            }
            else
            {
                mData[3] = 0.0f;
            }
        }
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator=(const VectorBase& rhs)
    {
        mData[0] = rhs.mData[0];
        mData[1] = rhs.mData[1];
        if constexpr (N >= 3)
        {
            mData[2] = rhs.mData[2];
        }
        if constexpr (N >= 4)
        {
            mData[3] = rhs.mData[3];
        }

        return *this;
    }

    template <int N>
    template <int M>
    inline VectorBase<N>& VectorBase<N>::operator=(const VectorBase<M>& rhs)
    {
        mData[0] = rhs.mData[0];
        mData[1] = rhs.mData[1];
        if constexpr (N >= 3)
        {
            if constexpr (M >= 3)
            {
                mData[2] = rhs.mData[2];
            }
            else
            {
                mData[2] = 0.0f;
            }
        }
        if constexpr (N >= 4)
        {
            if constexpr (M >= 4)
            {
                mData[3] = rhs.mData[3];
            }
            else
            {
                mData[3] = 0.0f;
            }
        }

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator=(float rhs)
    {
        mData[0] = rhs;
        mData[1] = rhs;
        if constexpr (N >= 3)
        {
            mData[2] = rhs;
        }
        if constexpr (N >= 4)
        {
            mData[3] = rhs;
        }

        return *this;
    }

    template <int N>
    inline bool VectorBase<N>::operator==(const VectorBase& rhs) const
    {
        bool res = true;
        res &= (abs(mData[0] - rhs.mData[0]) < FLOAT_EPS);
        res &= (abs(mData[1] - rhs.mData[1]) < FLOAT_EPS);
        if constexpr (N >= 3)
        {
            res &= (abs(mData[2] - rhs.mData[2]) < FLOAT_EPS);
        }
        if constexpr (N >= 4)
        {
            res &= (abs(mData[3] - rhs.mData[3]) < FLOAT_EPS);
        }

        return res;
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
        res /= rhs;

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
        mData[0] += rhs.mData[0];
        mData[1] += rhs.mData[1];
        if constexpr (N >= 3 && M >= 3)
        {
            mData[2] += rhs.mData[2];
        }
        if constexpr (N >= 4 && M >= 4)
        {
            mData[3] += rhs.mData[3];
        }

        return *this;
    }

    template <int N>
    template <int M>
    inline VectorBase<N>& VectorBase<N>::operator-=(const VectorBase<M>& rhs)
    {
        mData[0] -= rhs.mData[0];
        mData[1] -= rhs.mData[1];
        if constexpr (N >= 3 && M >= 3)
        {
            mData[2] -= rhs.mData[2];
        }
        if constexpr (N >= 4 && M >= 4)
        {
            mData[3] -= rhs.mData[3];
        }

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator*=(float rhs)
    {
        mData[0] *= rhs;
        mData[1] *= rhs;
        if constexpr (N >= 3)
        {
            mData[2] *= rhs;
        }
        if constexpr (N >= 4)
        {
            mData[3] *= rhs;
        }

        return *this;
    }

    template <int N>
    template <int M>
    inline VectorBase<N>& VectorBase<N>::operator*=(const VectorBase<M>& rhs)
    {
        mData[0] *= rhs.mData[0];
        mData[1] *= rhs.mData[1];
        if constexpr (N >= 3 && M >= 3)
        {
            mData[2] *= rhs.mData[2];
        }
        if constexpr (N >= 4 && M >= 4)
        {
            mData[3] *= rhs.mData[3];
        }

        return *this;
    }

    template <int N>
    inline VectorBase<N>& VectorBase<N>::operator/=(float rhs)
    {
        mData[0] /= rhs;
        mData[1] /= rhs;
        if constexpr (N >= 3)
        {
            mData[2] /= rhs;
        }
        if constexpr (N >= 4)
        {
            mData[3] /= rhs;
        }

        return *this;
    }

    template <int N>
    template <int M>
    inline VectorBase<N>& VectorBase<N>::operator/=(const VectorBase<M>& rhs)
    {
        mData[0] /= rhs.mData[0];
        mData[1] /= rhs.mData[1];
        if constexpr (N >= 3 && M >= 3)
        {
            mData[2] /= rhs.mData[2];
        }
        if constexpr (N >= 4 && M >= 4)
        {
            mData[3] /= rhs.mData[3];
        }

        return *this;
    }

    template <int N>
    inline void VectorBase<N>::Swap(VectorBase& other)
    {
        std::swap(mData[0], other.mData[0]);
        std::swap(mData[1], other.mData[1]);
        if constexpr (N >= 3)
        {
            std::swap(mData[2], other.mData[2]);
        }
        if constexpr (N >= 4)
        {
            std::swap(mData[3], other.mData[3]);
        }
    }

    template <int N>
    inline void VectorBase<N>::Swap(VectorBase& lhs, VectorBase& rhs)
    {
        lhs.Swap(rhs);
    }

    template <int N>
    inline Float2 VectorBase<N>::GetFloat2() const
    {
        Float2 res;
        res.x = mData[0];
        res.y = mData[1];

        return res;
    }

    template <int N>
    inline Float3 VectorBase<N>::GetFloat3() const
    {
        Float3 res;
        res.x = mData[0];
        res.y = mData[1];
        if constexpr (N >= 3)
        {
            res.z = mData[2];
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
        Float4 res;
        res.x = mData[0];
        res.y = mData[1];
        if constexpr (N >= 3)
        {
            res.z = mData[2];
        }
        else
        {
            res.z = 0.0f;
        }
        if constexpr (N >= 4)
        {
            res.w = mData[3];
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
        float res = 0.0f;
        res += mData[0] * mData[0];
        res += mData[1] * mData[1];
        if constexpr (N >= 3)
        {
            res += mData[2] * mData[2];
        }
        if constexpr (N >= 4)
        {
            res += mData[3] * mData[3];
        }

        return res;
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::LengthV() const
    {
        return VectorBase(Length());
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::SquareLengthV() const
    {
        return VectorBase(SquareLength());
    }

    template <int N>
    inline void VectorBase<N>::Normalize()
    {
        *this /= Length();
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::Normalized()
    {
        return *this / Length();
    }

    template <int N>
    inline float VectorBase<N>::Dot(const VectorBase& rhs) const
    {
        float res = 0.0f;
        res += mData[0] * rhs.mData[0];
        res += mData[1] * rhs.mData[1];
        if constexpr (N >= 3)
        {
            res += mData[2] * rhs.mData[2];
        }
        if constexpr (N >= 4)
        {
            res += mData[3] * rhs.mData[3];
        }

        return res;
    }

    template <int N>
    inline float VectorBase<N>::Dot(const VectorBase& lhs, const VectorBase& rhs)
    {
        return lhs.Dot(rhs);
    }

    template <int N>
    inline VectorBase<N> VectorBase<N>::DotV(const VectorBase& rhs) const
    {
        return VectorBase(Dot(rhs));
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

        VectorBase<3> res;
        res.mData[0] = mData[1] * rhs.mData[2] - mData[2] * rhs.mData[1];
        res.mData[1] = mData[2] * rhs.mData[0] - mData[0] * rhs.mData[2];
        res.mData[2] = mData[0] * rhs.mData[1] - mData[1] * rhs.mData[0];

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
