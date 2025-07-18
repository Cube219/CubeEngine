#pragma once

#include "Vector.h"

namespace cube
{
    class Matrix
    {
    public:
        static Matrix Zero();
        static Matrix Identity();

        Matrix();
        Matrix(const Vector4& row1, const Vector4& row2, const Vector4& row3, const Vector4& row4);
        Matrix(float v[16]);
        Matrix(float v11, float v12, float v13, float v14,
            float v21, float v22, float v23, float v24,
            float v31, float v32, float v33, float v34,
            float v41, float v42, float v43, float v44);

        ~Matrix();

        Matrix(const Matrix& other);
        Matrix& operator=(const Matrix& rhs);
        Vector4& operator[](int i);

        Matrix operator+ (const Matrix& rhs) const;
        Matrix operator- (const Matrix& rhs) const;
        Matrix operator* (float rhs) const;
        Matrix operator* (const Matrix& rhs) const;
        Matrix operator/ (float rhs) const;

        Matrix& operator+= (const Matrix& rhs);
        Matrix& operator-= (const Matrix& rhs);
        Matrix& operator*= (float rhs);
        Matrix& operator*= (const Matrix& rhs);
        Matrix& operator/= (float rhs);

        Vector4 GetCol(int index) const;
        Vector4 GetRow(int index) const;

        void SetCol(int index, const Vector4& col);
        void SetRow(int index, const Vector4& row);

        void Transpose();
        Matrix Transposed() const;

        void Inverse(); // TODO: Implement
        Matrix Inversed() const; // TODO: Implement

    private:
        Vector4 mRows[4];

        friend Matrix operator* (float lhs, const Matrix& rhs);
        friend Vector4 operator* (const Vector4& lhs, const Matrix& rhs);

        template <int removeRow, int removeCol>
        float GetSubMatrixDet() const;
    };
} // namespace cube

#ifndef CUBE_MATRIX_IMPLEMENTATION

#if CUBE_VECTOR_USE_SSE
#include "MatrixImpl/MatrixSSE.inl"
#else
#include "MatrixImpl/MatrixArray.inl"
#endif

#endif // !CUBE_MATRIX_IMPLEMENTATION
