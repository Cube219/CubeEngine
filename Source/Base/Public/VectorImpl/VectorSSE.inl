#define CUBE_VECTOR_IMPLEMENTATION

#include "../Vector.h"

namespace cube
{
    inline VectorBase VectorBase::Zero()
    {
        VectorBase res;
        res.mData = _mm_setzero_ps();

        return res;
    }

    inline VectorBase::VectorBase()
    {}

    inline VectorBase::VectorBase(float x, float y, float z, float w)
    {
        mData = _mm_set_ps(w, z, y, x);
    }

    inline VectorBase::~VectorBase()
    {}

    inline VectorBase& VectorBase::operator=(const VectorBase& rhs)
    {
        mData = rhs.mData;

        return *this;
    }

    inline VectorBase& VectorBase::operator=(float rhs)
    {
        mData = _mm_set1_ps(rhs);

        return *this;
    }

    inline bool VectorBase::operator==(const VectorBase& rhs) const
    {
        VectorData temp = _mm_cmpeq_ps(mData, rhs.mData);
        return (_mm_movemask_ps(temp) & 0b1111) == 0b1111;
    }

    inline bool VectorBase::operator!=(const VectorBase& rhs) const
    {
        VectorData temp = _mm_cmpneq_ps(mData, rhs.mData);
        return (_mm_movemask_ps(temp) & 0b1111) == 0b1111;
    }

    inline VectorBase VectorBase::operator+(const VectorBase& rhs) const
    {
        VectorBase res;
        res.mData = _mm_add_ps(mData, rhs.mData);

        return res;
    }

    inline VectorBase VectorBase::operator-(const VectorBase& rhs) const
    {
        VectorBase res;
        res.mData = _mm_sub_ps(mData, rhs.mData);

        return res;
    }

    inline VectorBase VectorBase::operator*(float rhs) const
    {
        VectorBase res;
        res.mData = _mm_mul_ps(_mm_set1_ps(rhs), mData);

        return res;
    }

    inline VectorBase VectorBase::operator*(const VectorBase& rhs) const
    {
        VectorBase res;
        res.mData = _mm_mul_ps(rhs.mData, mData);

        return res;
    }

    inline VectorBase VectorBase::operator/(float rhs) const
    {
        VectorBase res;
        res.mData = _mm_div_ps(mData, _mm_set1_ps(rhs));

        return res;
    }

    inline VectorBase VectorBase::operator/(const VectorBase& rhs) const
    {
        VectorBase res;
        res.mData = _mm_div_ps(mData, rhs.mData);

        return res;
    }

    inline const VectorBase& VectorBase::operator+() const
    {
        return *this;
    }

    inline VectorBase VectorBase::operator-() const
    {
        VectorBase res = VectorBase::Zero();
        res.mData = _mm_sub_ps(res.mData, mData);

        return res;
    }

    inline VectorBase& VectorBase::operator+=(const VectorBase& rhs)
    {
        mData = _mm_add_ps(mData, rhs.mData);

        return *this;
    }

    inline VectorBase& VectorBase::operator-=(const VectorBase& rhs)
    {
        mData = _mm_sub_ps(mData, rhs.mData);

        return *this;
    }

    inline VectorBase& VectorBase::operator*=(float rhs)
    {
        mData = _mm_mul_ps(mData, _mm_set1_ps(rhs));

        return *this;
    }

    inline VectorBase& VectorBase::operator*=(const VectorBase& rhs)
    {
        mData = _mm_mul_ps(mData, rhs.mData);

        return *this;
    }

    inline VectorBase& VectorBase::operator/=(float rhs)
    {
        mData = _mm_div_ps(mData, _mm_set1_ps(rhs));

        return *this;
    }

    inline VectorBase& VectorBase::operator/=(const VectorBase& rhs)
    {
        mData = _mm_div_ps(mData, rhs.mData);

        return *this;
    }

    inline VectorBase operator*(float lhs, const VectorBase& rhs)
    {
        VectorBase res;
        res.mData = _mm_mul_ps(_mm_set1_ps(lhs), rhs.mData);

        return res;
    }

    inline VectorBase operator/(float lhs, const VectorBase& rhs)
    {
        VectorBase res;
        res.mData = _mm_div_ps(_mm_set1_ps(lhs), rhs.mData);

        return res;
    }

    // -------------------------------------------
    //                  Vector2
    // -------------------------------------------

    // Helper functions
    inline VectorData GetSum2(VectorData data)
    {
        // data = x / y / ? / ?

        // temp = y / y / y / y
        VectorData temp = _mm_shuffle_ps(data, data, _MM_SHUFFLE(1, 1, 1, 1));
        // temp = x+y / ? / ? / ?
        temp = _mm_add_ss(temp, data);

        VectorData res;
        res = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));

        return res;
    }

    inline Vector2::Vector2()
    {}

    inline Vector2::Vector2(float x, float y) :
        VectorBase(x, y, 0.0f, 0.0f)
    {}

    inline Vector2::Vector2(const VectorBase& vec)
    {
        mData = vec.mData;
    }

    inline Vector2& Vector2::operator=(const VectorBase& vec)
    {
        mData = vec.mData;

        return *this;
    }

    inline Vector2::operator Vector3() const
    {
        VectorData zero = _mm_setzero_ps();

        Vector3 v3(mData);
        v3.mData = _mm_shuffle_ps(mData, zero, _MM_SHUFFLE(0, 0, 1, 0));

        return v3;
    }

    inline Vector2::operator Vector4() const
    {
        VectorData zero = _mm_setzero_ps();

        Vector4 v4(mData);
        v4.mData = _mm_shuffle_ps(mData, zero, _MM_SHUFFLE(0, 0, 1, 0));

        return v4;
    }

    inline Float2 Vector2::GetFloat2() const
    {
        float f[4];
        _mm_store_ps(f, mData);

        Float2 f2;
        f2.x = f[0];
        f2.y = f[1];

        return f2;
    }

    inline VectorBase Vector2::Length() const
    {
        VectorData length = _mm_mul_ps(mData, mData);

        VectorBase res;
        res.mData = _mm_sqrt_ps(GetSum2(length));

        return res;
    }

    inline VectorBase Vector2::SquareLength() const
    {
        VectorData length = _mm_mul_ps(mData, mData);

        VectorBase res;
        res.mData = GetSum2(length);

        return res;
    }

    inline VectorBase Vector2::Dot(const Vector2& rhs) const
    {
        VectorData temp;
        temp = _mm_mul_ps(mData, rhs.mData);

        VectorBase res;
        res.mData = GetSum2(temp);

        return res;
    }

    inline VectorBase Vector2::Dot(const Vector2& lhs, const Vector2& rhs)
    {
        VectorData temp;
        temp = _mm_mul_ps(lhs.mData, rhs.mData);

        VectorBase res;
        res.mData = GetSum2(temp);

        return res;
    }

    inline void Vector2::Normalize()
    {
        VectorData length = _mm_mul_ps(mData, mData);
        length = _mm_sqrt_ps(GetSum2(length));

        // TODO: check 0
        mData = _mm_div_ps(mData, length);
    }

    inline VectorBase Vector2::Normalized() const
    {
        VectorData length = _mm_mul_ps(mData, mData);
        length = _mm_sqrt_ps(GetSum2(length));

        // TODO: check 0
        VectorBase res;
        res.mData = _mm_div_ps(mData, length);

        return res;
    }

    inline Vector2::Vector2(VectorData vData)
    {
        mData = vData;
    }

    // -------------------------------------------
    //                  Vector3
    // -------------------------------------------

    // Helper functions
    inline VectorData GetSum3(VectorData data)
    {
        // data = x / y / z / ?

        // temp = y / y / y / y
        VectorData temp = _mm_shuffle_ps(data, data, _MM_SHUFFLE(1, 1, 1, 1));
        // temp = x+y / ? / ? / ?
        temp = _mm_add_ss(temp, data);
        // temp2 = z / z / z / z
        VectorData temp2 = _mm_shuffle_ps(data, data, _MM_SHUFFLE(2, 2, 2, 2));
        // temp = x+y+z / ? / ? / ?
        temp = _mm_add_ss(temp, temp2);

        VectorData res;
        res = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));

        return res;
    }

    inline Vector3::Vector3()
    {}

    inline Vector3::Vector3(float x, float y, float z) :
        VectorBase(x, y, z, 0.0f)
    {}

    inline Vector3::Vector3(const VectorBase& vec)
    {
        mData = vec.mData;
    }

    inline Vector3& Vector3::operator=(const VectorBase& vec)
    {
        mData = vec.mData;

        return *this;
    }

    inline Vector3::operator Vector2() const
    {
        return Vector2(mData);
    }

    inline Vector3::operator Vector4() const
    {
        VectorData zero = _mm_setzero_ps();

        Vector4 v4(mData);

        VectorData temp;
        // temp = z / z / 0 / 0
        temp = _mm_shuffle_ps(zero, mData, _MM_SHUFFLE(0, 0, 2, 2));
        // v4.mData = x / y / z / 0
        v4.mData = _mm_shuffle_ps(mData, temp, _MM_SHUFFLE(2, 0, 1, 0));

        return v4;
    }

    inline Float3 Vector3::GetFloat3() const
    {
        float f[4];
        _mm_store_ps(f, mData);

        Float3 f3;
        f3.x = f[0];
        f3.y = f[1];
        f3.z = f[2];

        return f3;
    }

    inline VectorBase Vector3::Length() const
    {
        VectorData length = _mm_mul_ps(mData, mData);

        VectorBase res;
        res.mData = _mm_sqrt_ps(GetSum3(length));

        return res;
    }

    inline VectorBase Vector3::SquareLength() const
    {
        VectorData length = _mm_mul_ps(mData, mData);

        VectorBase res;
        res.mData = GetSum3(length);

        return res;
    }

    inline VectorBase Vector3::Dot(const Vector3& rhs) const
    {
        VectorData temp;
        temp = _mm_mul_ps(mData, rhs.mData);

        VectorBase res;
        res.mData = GetSum3(temp);

        return res;
    }

    inline VectorBase Vector3::Dot(const Vector3& lhs, const Vector3& rhs)
    {
        VectorData temp;
        temp = _mm_mul_ps(lhs.mData, rhs.mData);

        VectorBase res;
        res.mData = GetSum3(temp);

        return res;
    }

    inline void Vector3::Normalize()
    {
        VectorData length = _mm_mul_ps(mData, mData);
        length = _mm_sqrt_ps(GetSum3(length));

        // TODO: check 0
        mData = _mm_div_ps(mData, length);
    }

    inline VectorBase Vector3::Normalized() const
    {
        VectorData length = _mm_mul_ps(mData, mData);
        length = _mm_sqrt_ps(GetSum3(length));

        // TODO: check 0
        VectorBase res;
        res.mData = _mm_div_ps(mData, length);

        return res;
    }

    inline VectorBase Vector3::Cross(const Vector3& rhs) const
    {
        VectorBase res;

        // y1 / z1 / x1 / ??
        VectorData leftMul = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(0, 0, 2, 1));
        // z2 / x2 / y2 / ??
        VectorData rightMul = _mm_shuffle_ps(rhs.mData, rhs.mData, _MM_SHUFFLE(0, 1, 0, 2));

        res.mData = _mm_mul_ps(leftMul, rightMul);

        // z1 / x1 / y1 / ??
        leftMul = _mm_shuffle_ps(mData, mData, _MM_SHUFFLE(0, 1, 0, 2));
        // y2 / z2 / x2 / ??
        rightMul = _mm_shuffle_ps(rhs.mData, rhs.mData, _MM_SHUFFLE(0, 0, 2, 1));

        leftMul = _mm_mul_ps(leftMul, rightMul);
        res.mData = _mm_sub_ps(res.mData, leftMul);

        return res;
    }

    inline VectorBase Vector3::Cross(const Vector3& lhs, const Vector3& rhs)
    {
        VectorBase res;

        // y1 / z1 / x1 / ??
        VectorData leftMul = _mm_shuffle_ps(lhs.mData, lhs.mData, _MM_SHUFFLE(0, 0, 2, 1));
        // z2 / x2 / y2 / ??
        VectorData rightMul = _mm_shuffle_ps(rhs.mData, rhs.mData, _MM_SHUFFLE(0, 1, 0, 2));

        res.mData = _mm_mul_ps(leftMul, rightMul);

        // z1 / x1 / y1 / ??
        leftMul = _mm_shuffle_ps(lhs.mData, lhs.mData, _MM_SHUFFLE(0, 1, 0, 2));
        // y2 / z2 / x2 / ??
        rightMul = _mm_shuffle_ps(rhs.mData, rhs.mData, _MM_SHUFFLE(0, 0, 2, 1));

        leftMul = _mm_mul_ps(leftMul, rightMul);
        res.mData = _mm_sub_ps(res.mData, leftMul);

        return res;
    }

    inline Vector3::Vector3(VectorData vData)
    {
        mData = vData;
    }

    // -------------------------------------------
    //                  Vector4
    // -------------------------------------------

    // Helper functions
    inline VectorData GetSum4(VectorData data)
    {
        // data = d / c / b / a

        // temp = c / c / a / a
        VectorData temp = _mm_shuffle_ps(data, data, _MM_SHUFFLE(3, 3, 1, 1));
        // temp = c+d / ? / a+b / ?
        temp = _mm_add_ps(temp, data);
        // temp2 = a+b / a+b / a+b / a+b
        VectorData temp2 = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2));
        // temp = a+b+c+d / ? / ? / ?
        temp = _mm_add_ss(temp2, temp);

        VectorData res;
        res = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));

        return res;
    }

    inline Vector4::Vector4()
    {}

    inline Vector4::Vector4(float x, float y, float z, float w) :
        VectorBase(x, y, z, w)
    {}

    inline Vector4::Vector4(const VectorBase& vec)
    {
        mData = vec.mData;
    }

    inline Vector4& Vector4::operator=(const VectorBase& vec)
    {
        mData = vec.mData;

        return *this;
    }

    inline Vector4::operator Vector2() const
    {
        return Vector2(mData);
    }

    inline Vector4::operator Vector3() const
    {
        return Vector3(mData);
    }

    inline Float4 Vector4::GetFloat4() const
    {
        Float4 f4;
        _mm_store_ps((float*)&f4, mData);

        return f4;
    }

    inline VectorBase Vector4::Length() const
    {
        VectorData length = _mm_mul_ps(mData, mData);

        VectorBase res;
        res.mData = _mm_sqrt_ps(GetSum4(length));

        return res;
    }

    inline VectorBase Vector4::SquareLength() const
    {
        VectorData length = _mm_mul_ps(mData, mData);

        VectorBase res;
        res.mData = GetSum4(length);

        return res;
    }

    inline VectorBase Vector4::Dot(const Vector4& rhs) const
    {
        VectorData temp;
        temp = _mm_mul_ps(mData, rhs.mData);

        VectorBase res;
        res.mData = GetSum4(temp);

        return res;
    }

    inline VectorBase Vector4::Dot(const Vector4& lhs, const Vector4& rhs)
    {
        VectorData temp;
        temp = _mm_mul_ps(lhs.mData, rhs.mData);

        VectorBase res;
        res.mData = GetSum4(temp);

        return res;
    }

    inline void Vector4::Normalize()
    {
        VectorData length = _mm_mul_ps(mData, mData);
        length = _mm_sqrt_ps(GetSum4(length));

        // TODO: check 0
        mData = _mm_div_ps(mData, length);
    }

    inline VectorBase Vector4::Normalized() const
    {
        VectorData length = _mm_mul_ps(mData, mData);
        length = _mm_sqrt_ps(GetSum4(length));

        // TODO: check 0
        VectorBase res;
        res.mData = _mm_div_ps(mData, length);

        return res;
    }

    inline Vector4::Vector4(const VectorData vData)
    {
        mData = vData;
    }
} // namespace cube
