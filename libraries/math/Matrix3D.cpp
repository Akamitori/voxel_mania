#include "Matrix3D.h"


Matrix3D::Matrix3D(const Vector3D &a, const Vector3D &b, const Vector3D &c) : column_vectors{a, b, c} {
}

Matrix3D::Matrix3D(const float a) : column_vectors{Vector3D{a, 0, 0}, Vector3D{0, a, 0}, Vector3D{0, 0, a}} {
}

Matrix3D::Matrix3D(
    const float n00, const float n01, const float n02,
    const float n10, const float n11, const float n12,
    const float n20, const float n21, const float n22
) : column_vectors{
    {n00, n10, n20},
    {n01, n11, n21},
    {n02, n12, n22}
} {
}

Matrix3D::Matrix3D(): Matrix3D(0) {
}

const Vector3D &Matrix3D::operator[](const int index) const {
    return column_vectors[index];
}

Vector3D &Matrix3D::operator[](const int index) {
    return column_vectors[index];
}

Matrix3D operator *(const Matrix3D& A, const Matrix3D& B) {
    
    return {
        A[0].x * B[0].x + A[1].x * B[0].y + A[2].x * B[0].z,
        A[0].x * B[1].x + A[1].x * B[1].y + A[2].x * B[1].z,
        A[0].x * B[2].x + A[1].x * B[2].y + A[2].x * B[2].z,
        
        A[0].y * B[0].x + A[1].y * B[0].y + A[2].y * B[0].z,
        A[0].y * B[1].x + A[1].y * B[1].y + A[2].y * B[1].z,
        A[0].y * B[2].x + A[1].y * B[2].y + A[2].y * B[2].z,
        
        A[0].z * B[0].x + A[1].z * B[0].y + A[2].z * B[0].z,
        A[0].z * B[1].x + A[1].z * B[1].y + A[2].z * B[1].z,
        A[0].z * B[2].x + A[1].z * B[2].y + A[2].z * B[2].z
    };
}

Matrix3D& Matrix3D::operator=(const Matrix3D &b) {
    if (&b!=this) {
        this->column_vectors[0]=b.column_vectors[0];
        this->column_vectors[1]=b.column_vectors[1];
        this->column_vectors[2]=b.column_vectors[2]; 
    }
    return *this;
}

Matrix3D inverse(const Matrix3D &M) {
    const Vector3D &a = M[0];
    const Vector3D &b = M[1];
    const Vector3D &c = M[2];

    const Vector3D r0 = cross(b, c);
    const Vector3D r1 = cross(c, a);
    const Vector3D r2 = cross(a, b);

    const float invDet = 1.0F / dot(r2, c);

    return {
        r0.x * invDet, r0.y * invDet, r0.z * invDet,
        r1.x * invDet, r1.y * invDet, r1.z * invDet,
        r2.x * invDet, r2.y * invDet, r2.z * invDet
    };
}
