#include <cmath>
#include "Transformations.h"
#include "Matrix4D.h"
#include "Matrix3D.h"
#include "Vector3D.h"
#include "Plane.h"

Matrix3D rotation_x_matrix3D(const float t) {
    float c = cos(t);
    float s = sin(t);

    return {
        1.f, 0.f, 0.f,
        0.f, c, -s,
        0.f, s, c
    };
}

Matrix4D rotation_x_matrix4D(const float t) {
    float c = cos(t);
    float s = sin(t);

    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, c, -s, 0.f,
        0.f, s, c, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}

Matrix3D rotation_y_matrix3D(const float t) {
    float c = cos(t);
    float s = sin(t);

    return {
        c, 0.f, s,
        0.f, 1.f, 0.f,
        -s, 0.f, c
    };
}

Matrix4D rotation_y_matrix4D(const float t) {
    float c = cos(t);
    float s = sin(t);
    return {
        c, 0.f, s, 0.f,
        0.f, 1.f, 0.f, 0.f,
        -s, 0.f, c, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
}

Matrix3D rotation_z_matrix3D(const float t) {
    float c = cos(t);
    float s = sin(t);

    return {
        c, -s, 0.f,
        s, c, 0.f,
        0.f, 0.f, 1.f
    };
}

Matrix4D rotation_z_matrix4D(const float t) {
    float c = cos(t);
    float s = sin(t);

    return {
        c, -s, 0.f, 0.f,
        s, c, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
}

Matrix3D scale_matrix3D(const Matrix3D &m, const Vector3D &v) {
    Matrix3D result{};
    result[0] = m[0] * v.x;
    result[1] = m[1] * v.y;
    result[2] = m[2] * v.z;
    return result;
}

Matrix4D scale_matrix4D(const Matrix4D &m, const Vector3D &v) {
    Matrix4D result{};
    result[0] = m[0] * v.x;
    result[1] = m[1] * v.y;
    result[2] = m[2] * v.z;
    result[3] = m[3];
    return result;
}

Matrix4D translate(const Matrix4D &m, const Vector3D &v) {
    Matrix4D result{m};
    result[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
    return result;
}

Vector3D transform_vector(const Matrix4D &m, const Vector3D &v) {
    const Vector4D column_vector_0 = m[0];
    const Vector4D column_vector_1 = m[1];
    const Vector4D column_vector2 = m[2];

    return Vector3D{
        column_vector_0.x * v.x + column_vector_1.x * v.y + column_vector2.x * v.z,
        column_vector_0.y * v.x + column_vector_1.y * v.y + column_vector2.y * v.z,
        column_vector_0.z * v.x + column_vector_1.z * v.y + column_vector2.z * v.z
    };
}

Vector3D transform_point(const Matrix4D &m, const Vector3D &p) {
    const Vector4D column_vector_0 = m[0];
    const Vector4D column_vector_1 = m[1];
    const Vector4D column_vector_2 = m[2];
    const Vector4D column_vector_3 = m[3];

    return Vector3D{
        column_vector_0.x * p.x + column_vector_1.x * p.y + column_vector_2.x * p.z + column_vector_3.x,
        column_vector_0.y * p.x + column_vector_1.y * p.y + column_vector_2.y * p.z + column_vector_3.y,
        column_vector_0.z * p.x + column_vector_1.z * p.y + column_vector_2.z * p.z + column_vector_3.z
    };
}

Plane transform_plane(const Matrix4D &H, const Plane &f) {
    return {
        f.normal.x * H[0].x + f.normal.y * H[0].y + f.normal.z * H[0].z,
        f.normal.x * H[1].x + f.normal.y * H[1].y + f.normal.z * H[1].z,
        f.normal.x * H[2].x + f.normal.y * H[2].y + f.normal.z * H[2].z,
        f.normal.x * H[3].x + f.normal.y * H[3].y + f.normal.z * H[3].z + f.w
    };
}
