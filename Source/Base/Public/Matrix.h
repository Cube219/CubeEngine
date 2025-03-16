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
        Matrix(const VectorBase& row1, const VectorBase& row2, const VectorBase& row3, const VectorBase& row4);
        Matrix(float v[16]);
        Matrix(float v11, float v12, float v13, float v14,
            float v21, float v22, float v23, float v24,
            float v31, float v32, float v33, float v34,
            float v41, float v42, float v43, float v44);

        ~Matrix();

        Matrix(const Matrix& other);
        Matrix& operator=(const Matrix& rhs);
        VectorBase& operator[](int i);

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

        VectorBase GetCol(int index) const;
        VectorBase GetRow(int index) const;

        void SetCol(int index, const VectorBase& col);
        void SetRow(int index, const VectorBase& row);

        void Transpose();
        Matrix Transposed() const;

        void Inverse(); // TODO: 차후 구현
        Matrix Inversed() const; // TODO: 차후 구현

    private:
        VectorBase mRows[4];

        friend Matrix operator* (float lhs, const Matrix& rhs);
        friend VectorBase operator* (const Vector4& lhs, const Matrix& rhs);
    };
} // namespace cube

#ifndef CUBE_MATRIX_IMPLEMENTATION

#if CUBE_VECTOR_USE_SSE
#include "MatrixImpl/MatrixSSE.inl"
#else
#include "MatrixImpl/MatrixArray.inl"
#endif

#endif // !MATRIX_IMPLEMENTATION
