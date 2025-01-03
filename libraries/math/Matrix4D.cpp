#include "Matrix4D.h"
#include "Vector3D.h"

Matrix4D::Matrix4D() : Matrix4D{0} {
}

Matrix4D::Matrix4D(const Vector4D &a, const Vector4D &b, const Vector4D &c, const Vector4D &p) : column_vectors{
    a, b, c, p
} {
}

Matrix4D::Matrix4D(
    float n00, float n01, float n02, float n03,
    float n10, float n11, float n12, float n13,
    float n20, float n21, float n22, float n23,
    float n30, float n31, float n32, float n33)
    : column_vectors{
        Vector4D{n00, n10, n20, n30},
        Vector4D{n01, n11, n21, n31},
        Vector4D{n02, n12, n22, n32},
        Vector4D{n03, n13, n23, n33},
    } {
}

Matrix4D::Matrix4D(float a)
    : column_vectors{
        Vector4D{a, 0, 0, 0},
        Vector4D{0, a, 0, 0},
        Vector4D{0, 0, a, 0},
        Vector4D{0, 0, 0, a}
    } {
}

Matrix4D::Matrix4D(const Matrix4D &m) :Matrix4D(m[0],m[1],m[2],m[3]) {
    
}


const Vector4D &Matrix4D::operator[](const int index) const {
    return column_vectors[index];
}

Matrix4D& Matrix4D::operator=(const Matrix4D &b) {
    this->column_vectors[0]=b.column_vectors[0];
    this->column_vectors[1]=b.column_vectors[1];
    this->column_vectors[2]=b.column_vectors[2];
    this->column_vectors[3]=b.column_vectors[3];
    return (*this);
}

Vector4D &Matrix4D::operator[](int index) {
    return column_vectors[index];
}

Matrix4D inverse(const Matrix4D &M) {
    const Vector3D &a = reinterpret_cast<const Vector3D &>(M[0]);
    const Vector3D &b = reinterpret_cast<const Vector3D &>(M[1]);
    const Vector3D &c = reinterpret_cast<const Vector3D &>(M[2]);
    const Vector3D &d = reinterpret_cast<const Vector3D &>(M[3]);

    const float &x = M[0].w;
    const float &y = M[1].w;
    const float &z = M[2].w;
    const float &w = M[3].w;

    Vector3D s = cross(a, b);
    Vector3D t = cross(c, d);
    Vector3D u = a * y - b * x;
    Vector3D v = c * w - d * z;

    const float invDet = 1.0F / (dot(s, v) + dot(t, u));
    s *= invDet;
    t *= invDet;
    u *= invDet;
    v *= invDet;

    const Vector3D r0 = cross(b, v) + t * y;
    const Vector3D r1 = cross(v, a) - t * x;
    const Vector3D r2 = cross(d, u) + s * w;
    const Vector3D r3 = cross(u, c) - s * z;

    return {
        r0.x, r0.y, r0.z, -dot(b, t),
        r1.x, r1.y, r1.z, dot(a, t),
        r2.x, r2.y, r2.z, -dot(d, s),
        r3.x, r3.y, r3.z, dot(c, s)
    };
}

Matrix4D operator *(const Matrix4D &A, const Matrix4D &B) {
    return Matrix4D{
        A[0].x * B[0].x + A[1].x * B[0].y + A[2].x * B[0].z + A[3].x * B[0].w,
        A[0].x * B[1].x + A[1].x * B[1].y + A[2].x * B[1].z + A[3].x * B[1].w,
        A[0].x * B[2].x + A[1].x * B[2].y + A[2].x * B[2].z + A[3].x * B[2].w,
        A[0].x * B[3].x + A[1].x * B[3].y + A[2].x * B[3].z + A[3].x * B[3].w,

        A[0].y * B[0].x + A[1].y * B[0].y + A[2].y * B[0].z + A[3].y * B[0].w,
        A[0].y * B[1].x + A[1].y * B[1].y + A[2].y * B[1].z + A[3].y * B[1].w,
        A[0].y * B[2].x + A[1].y * B[2].y + A[2].y * B[2].z + A[3].y * B[2].w,
        A[0].y * B[3].x + A[1].y * B[3].y + A[2].y * B[3].z + A[3].y * B[3].w,

        A[0].z * B[0].x + A[1].z * B[0].y + A[2].z * B[0].z + A[3].z * B[0].w,
        A[0].z * B[1].x + A[1].z * B[1].y + A[2].z * B[1].z + A[3].z * B[1].w,
        A[0].z * B[2].x + A[1].z * B[2].y + A[2].z * B[2].z + A[3].z * B[2].w,
        A[0].z * B[3].x + A[1].z * B[3].y + A[2].z * B[3].z + A[3].z * B[3].w,

        A[0].w * B[0].x + A[1].w * B[0].y + A[2].w * B[0].z + A[3].w * B[0].w,
        A[0].w * B[1].x + A[1].w * B[1].y + A[2].w * B[1].z + A[3].w * B[1].w,
        A[0].w * B[2].x + A[1].w * B[2].y + A[2].w * B[2].z + A[3].w * B[2].w,
        A[0].w * B[3].x + A[1].w * B[3].y + A[2].w * B[3].z + A[3].w * B[3].w
    };
}


