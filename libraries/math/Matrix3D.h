#ifndef MATRIX3D_H
#define MATRIX3D_H


#include "Vector3D.h"
#include "export.h"

struct EXPORTED Matrix3D {
    Vector3D column_vectors[3];

    Matrix3D();

    Matrix3D(const Vector3D &a, const Vector3D &b, const Vector3D &c);

    Matrix3D(
        float n00, float n01, float n02,
        float n10, float n11, float n12,
        float n20, float n21, float n22
    );

    explicit Matrix3D(float a);

    Vector3D &operator[](int index);

    Matrix3D &operator=(const Matrix3D &b);

    const Vector3D &operator[](int index) const;
};

EXPORTED Matrix3D inverse(const Matrix3D &M);
EXPORTED Matrix3D operator *(const Matrix3D& A, const Matrix3D& B);
#endif //MATRIX3D_H
