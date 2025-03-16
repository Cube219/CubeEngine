#define CUBE_VECTOR_IMPLEMENTATION

#include "../Vector.h"

namespace cube
{
    inline VectorBase VectorBase::Zero()
    {
        VectorBase res;
        res.mData[0] = 0.0f;
        res.mData[1] = 0.0f;
        res.mData[2] = 0.0f;
        res.mData[3] = 0.0f;

        return res;
    }

    inline VectorBase::VectorBase()
    {}

    inline VectorBase::VectorBase(float x, float y, float z, float w)
    {
        mData[0] = x;
        mData[1] = y;
        mData[2] = z;
        mData[3] = w;
    }

    inline VectorBase::~VectorBase()
    {}

    inline VectorBase& VectorBase::operator=(const VectorBase& rhs)
    {
        mData[0] = rhs.mData[0];
        mData[1] = rhs.mData[1];
        mData[2] = rhs.mData[2];
        mData[3] = rhs.mData[3];

        return *this;
    }

    inline VectorBase& VectorBase::operator=(float rhs)
    {
        mData[0] = mData[1] = mData[2] = mData[3] = rhs;

        return *this;
    }

    inline VectorBase VectorBase::operator+(const VectorBase& rhs) const
    {
        VectorBase res;
        res.mData[0] = mData[0] + rhs.mData[0];
        res.mData[1] = mData[1] + rhs.mData[1];
        res.mData[2] = mData[2] + rhs.mData[2];
        res.mData[3] = mData[3] + rhs.mData[3];

        return res;
    }

    inline VectorBase VectorBase::operator-(const VectorBase& rhs) const
    {
        VectorBase res;
        res.mData[0] = mData[0] - rhs.mData[0];
        res.mData[1] = mData[1] - rhs.mData[1];
        res.mData[2] = mData[2] - rhs.mData[2];
        res.mData[3] = mData[3] - rhs.mData[3];

        return res;
    }

    inline VectorBase VectorBase::operator*(float rhs) const
    {
        VectorBase res;
        res.mData[0] = mData[0] * rhs;
        res.mData[1] = mData[1] * rhs;
        res.mData[2] = mData[2] * rhs;
        res.mData[3] = mData[3] * rhs;

        return res;
    }

    inline VectorBase VectorBase::operator*(const VectorBase& rhs) const
    {
        VectorBase res;
        res.mData[0] = mData[0] * rhs.mData[0];
        res.mData[1] = mData[1] * rhs.mData[1];
        res.mData[2] = mData[2] * rhs.mData[2];
        res.mData[3] = mData[3] * rhs.mData[3];

        return res;
    }

    inline VectorBase VectorBase::operator/(float rhs) const
    {
        VectorBase res;
        res.mData[0] = mData[0] / rhs;
        res.mData[1] = mData[1] / rhs;
        res.mData[2] = mData[2] / rhs;
        res.mData[3] = mData[3] / rhs;

        return res;
    }

    inline VectorBase VectorBase::operator/(const VectorBase& rhs) const
    {
        VectorBase res;
        res.mData[0] = mData[0] / rhs.mData[0];
        res.mData[1] = mData[1] / rhs.mData[1];
        res.mData[2] = mData[2] / rhs.mData[2];
        res.mData[3] = mData[3] / rhs.mData[3];

        return res;
    }

    inline const VectorBase& VectorBase::operator+() const
    {
        return *this;
    }

    inline VectorBase VectorBase::operator-() const
    {
        VectorBase res;
        res = *this * -1.0f;

        return res;
    }

    inline VectorBase& VectorBase::operator+=(const VectorBase& rhs)
    {
        mData[0] = mData[0] + rhs.mData[0];
        mData[1] = mData[1] + rhs.mData[1];
        mData[2] = mData[2] + rhs.mData[2];
        mData[3] = mData[3] + rhs.mData[3];

        return *this;
    }

    inline VectorBase& VectorBase::operator-=(const VectorBase& rhs)
    {
        mData[0] = mData[0] - rhs.mData[0];
        mData[1] = mData[1] - rhs.mData[1];
        mData[2] = mData[2] - rhs.mData[2];
        mData[3] = mData[3] - rhs.mData[3];

        return *this;
    }

    inline VectorBase& VectorBase::operator*=(float rhs)
    {
        mData[0] = mData[0] * rhs;
        mData[1] = mData[1] * rhs;
        mData[2] = mData[2] * rhs;
        mData[3] = mData[3] * rhs;

        return *this;
    }

    inline VectorBase& VectorBase::operator*=(const VectorBase& rhs)
    {
        mData[0] = mData[0] * rhs.mData[0];
        mData[1] = mData[1] * rhs.mData[1];
        mData[2] = mData[2] * rhs.mData[2];
        mData[3] = mData[3] * rhs.mData[3];

        return *this;
    }

    inline VectorBase& VectorBase::operator/=(float rhs)
    {
        mData[0] = mData[0] / rhs;
        mData[1] = mData[1] / rhs;
        mData[2] = mData[2] / rhs;
        mData[3] = mData[3] / rhs;

        return *this;
    }

    inline VectorBase& VectorBase::operator/=(const VectorBase& rhs)
    {
        mData[0] = mData[0] / rhs.mData[0];
        mData[1] = mData[1] / rhs.mData[1];
        mData[2] = mData[2] / rhs.mData[2];
        mData[3] = mData[3] / rhs.mData[3];

        return *this;
    }

    inline VectorBase operator*(float lhs, const VectorBase& rhs)
    {
        return rhs * lhs;
    }

    inline VectorBase operator/(float lhs, const VectorBase& rhs)
    {
        VectorBase res(lhs, lhs, lhs, lhs);
        res /= rhs;

        return res;
    }

    // -------------------------------------------
    //                  Vector2
    // -------------------------------------------

    inline Vector2::Vector2() :
        VectorBase()
    {}

    inline Vector2::Vector2(float x, float y) :
        VectorBase(x, y, 0.0f, 0.0f)
    {}

    inline Vector2::Vector2(const VectorBase& vec)
    {
        mData[0] = vec.mData[0];
        mData[1] = vec.mData[1];
        mData[2] = vec.mData[2];
        mData[3] = vec.mData[3];
    }

    inline Vector2& Vector2::operator=(const VectorBase& vec)
    {
        mData[0] = vec.mData[0];
        mData[1] = vec.mData[1];
        mData[2] = vec.mData[2];
        mData[3] = vec.mData[3];

        return *this;
    }

    inline Vector2::operator Vector3() const
    {
        Vector3 v3(mData);
        v3.mData[2] = 0.0f;
        v3.mData[3] = 0.0f;

        return v3;
    }

    inline Vector2::operator Vector4() const
    {
        Vector4 v4(mData);
        v4.mData[2] = 0.0f;
        v4.mData[3] = 0.0f;

        return v4;
    }

    inline Float2 Vector2::GetFloat2() const
    {
        Float2 f2;

        f2.x = mData[0];
        f2.y = mData[1];

        return f2;
    }

    inline VectorBase Vector2::Length() const
    {
        float l = mData[0] * mData[0];
        l += mData[1] * mData[1];

        l = (float)sqrt(l);

        return VectorBase(l, l, l, l);
    }

    inline VectorBase Vector2::SquareLength() const
    {
        float l = mData[0] * mData[0];
        l += mData[1] * mData[1];

        return VectorBase(l, l, l, l);
    }

    inline VectorBase Vector2::Dot(const Vector2& rhs) const
    {
        float res = mData[0] * rhs.mData[0];
        res += mData[1] * rhs.mData[1];

        return VectorBase(res, res, res, res);
    }

    inline VectorBase Vector2::Dot(const Vector2& lhs, const Vector2& rhs)
    {
        float res = lhs.mData[0] * rhs.mData[0];
        res += lhs.mData[1] * rhs.mData[1];

        return VectorBase(res, res, res, res);
    }

    inline void Vector2::Normalize()
    {
        *this /= Length();
    }

    inline VectorBase Vector2::Normalized() const
    {
        return *this / Length();
    }

    inline Vector2::Vector2(const VectorData vData)
    {
        mData[0] = vData[0];
        mData[1] = vData[1];
        mData[2] = vData[2];
        mData[3] = vData[3];
    }

    // -------------------------------------------
    //                  Vector3
    // -------------------------------------------

    inline Vector3::Vector3()
    {}

    inline Vector3::Vector3(float x, float y, float z) :
        VectorBase(x, y, z, 0.0f)
    {}

    inline Vector3::Vector3(const VectorBase& vec)
    {
        mData[0] = vec.mData[0];
        mData[1] = vec.mData[1];
        mData[2] = vec.mData[2];
        mData[3] = vec.mData[3];
    }

    inline Vector3& Vector3::operator=(const VectorBase& vec)
    {
        mData[0] = vec.mData[0];
        mData[1] = vec.mData[1];
        mData[2] = vec.mData[2];
        mData[3] = vec.mData[3];

        return *this;
    }

    inline Vector3::operator Vector2() const
    {
        Vector2 v2(mData);

        return v2;
    }

    inline Vector3::operator Vector4() const
    {
        Vector4 v4(mData);
        v4.mData[3] = 0.0f;

        return v4;
    }

    inline Float3 Vector3::GetFloat3() const
    {
        Float3 f3;

        f3.x = mData[0];
        f3.y = mData[1];
        f3.z = mData[2];

        return f3;
    }

    inline VectorBase Vector3::Length() const
    {
        float l = mData[0] * mData[0];
        l += mData[1] * mData[1];
        l += mData[2] * mData[2];

        l = (float)sqrt(l);

        return VectorBase(l, l, l, l);
    }

    inline VectorBase Vector3::SquareLength() const
    {
        float l = mData[0] * mData[0];
        l += mData[1] * mData[1];
        l += mData[2] * mData[2];

        return VectorBase(l, l, l, l);
    }

    inline VectorBase Vector3::Dot(const Vector3& rhs) const
    {
        float res = mData[0] * rhs.mData[0];
        res += mData[1] * rhs.mData[1];
        res += mData[2] * rhs.mData[2];

        return VectorBase(res, res, res, res);
    }

    inline VectorBase Vector3::Dot(const Vector3& lhs, const Vector3& rhs)
    {
        float res = lhs.mData[0] * rhs.mData[0];
        res += lhs.mData[1] * rhs.mData[1];
        res += lhs.mData[2] * rhs.mData[2];

        return VectorBase(res, res, res, res);
    }

    inline void Vector3::Normalize()
    {
        *this /= Length();
    }

    inline VectorBase Vector3::Normalized() const
    {
        return *this / Length();
    }

    inline VectorBase Vector3::Cross(const Vector3& rhs) const
    {
        VectorBase v;
        v.mData[0] = mData[1] * rhs.mData[2] - mData[2] * rhs.mData[1];
        v.mData[1] = mData[2] * rhs.mData[0] - mData[0] * rhs.mData[2];
        v.mData[2] = mData[0] * rhs.mData[1] - mData[1] * rhs.mData[0];
        v.mData[3] = 0.0f;

        return v;
    }

    inline VectorBase Vector3::Cross(const Vector3& lhs, const Vector3& rhs)
    {
        VectorBase v;
        v.mData[0] = lhs.mData[1] * rhs.mData[2] - lhs.mData[2] * rhs.mData[1];
        v.mData[1] = lhs.mData[2] * rhs.mData[0] - lhs.mData[0] * rhs.mData[2];
        v.mData[2] = lhs.mData[0] * rhs.mData[1] - lhs.mData[1] * rhs.mData[0];
        v.mData[3] = 0.0f;

        return v;
    }

    inline Vector3::Vector3(const VectorData vData)
    {
        mData[0] = vData[0];
        mData[1] = vData[1];
        mData[2] = vData[2];
        mData[3] = vData[3];
    }

    // -------------------------------------------
    //                  Vector4
    // -------------------------------------------

    inline Vector4::Vector4()
    {}

    inline Vector4::Vector4(float x, float y, float z, float w) :
        VectorBase(x, y, z, w)
    {}

    inline Vector4::Vector4(const VectorBase& vec)
    {
        mData[0] = vec.mData[0];
        mData[1] = vec.mData[1];
        mData[2] = vec.mData[2];
        mData[3] = vec.mData[3];
    }

    inline Vector4& Vector4::operator=(const VectorBase& vec)
    {
        mData[0] = vec.mData[0];
        mData[1] = vec.mData[1];
        mData[2] = vec.mData[2];
        mData[3] = vec.mData[3];

        return *this;
    }

    inline Vector4::operator Vector2() const
    {
        Vector2 v2(mData);

        return v2;
    }

    inline Vector4::operator Vector3() const
    {
        Vector3 v3(mData);

        return v3;
    }

    inline Float4 Vector4::GetFloat4() const
    {
        Float4 f4;

        f4.x = mData[0];
        f4.y = mData[1];
        f4.z = mData[2];
        f4.w = mData[3];

        return f4;
    }

    inline VectorBase Vector4::Length() const
    {
        float l = mData[0] * mData[0];
        l += mData[1] * mData[1];
        l += mData[2] * mData[2];
        l += mData[3] * mData[3];

        l = (float)sqrt(l);

        return VectorBase(l, l, l, l);
    }

    inline VectorBase Vector4::SquareLength() const
    {
        float l = mData[0] * mData[0];
        l += mData[1] * mData[1];
        l += mData[2] * mData[2];
        l += mData[3] * mData[3];

        return VectorBase(l, l, l, l);
    }

    inline VectorBase Vector4::Dot(const Vector4& rhs) const
    {
        float res = mData[0] * rhs.mData[0];
        res += mData[1] * rhs.mData[1];
        res += mData[2] * rhs.mData[2];
        res += mData[3] * rhs.mData[3];

        return VectorBase(res, res, res, res);
    }

    inline VectorBase Vector4::Dot(const Vector4& lhs, const Vector4& rhs)
    {
        float res = lhs.mData[0] * rhs.mData[0];
        res += lhs.mData[1] * rhs.mData[1];
        res += lhs.mData[2] * rhs.mData[2];
        res += lhs.mData[3] * rhs.mData[3];

        return VectorBase(res, res, res, res);
    }

    inline void Vector4::Normalize()
    {
        *this /= Length();
    }

    inline VectorBase Vector4::Normalized() const
    {
        return *this / Length();
    }

    inline Vector4::Vector4(const VectorData vData)
    {
        mData[0] = vData[0];
        mData[1] = vData[1];
        mData[2] = vData[2];
        mData[3] = vData[3];
    }
} // namespace cube
