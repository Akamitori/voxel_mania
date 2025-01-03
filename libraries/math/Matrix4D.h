#ifndef MATRIX4D_H
#define MATRIX4D_H

#include "Vector4D.h"
#include "export.h"


struct EXPORTED Matrix4D {
    Vector4D column_vectors[4];
    
    Matrix4D();

    Matrix4D(const Matrix4D& m);

    Matrix4D(const Vector4D &a, const Vector4D &b, const Vector4D &c, const Vector4D &p);

    Matrix4D(
        float n00, float n01, float n02, float n03,
        float n10, float n11, float n12, float n13,
        float n20, float n21, float n22, float n23,
        float n30, float n31, float n32, float n33
    );

    explicit Matrix4D(float a);
    
    Vector4D &operator[](int index);

    const Vector4D &operator[](int index) const;

    Matrix4D& operator=(const Matrix4D& b);
};

EXPORTED Matrix4D inverse(const Matrix4D &M);
EXPORTED Matrix4D operator *(const Matrix4D& A, const Matrix4D& B);


#endif //MATRIX4D_H
