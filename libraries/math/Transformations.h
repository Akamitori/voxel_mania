#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H

#include "linearalgebra_export.h"

struct Matrix3D;
struct Matrix4D;
struct Vector3D;
struct  Vector4D;
struct Quaternion;
struct Plane;

LINEARALGEBRA_EXPORT Matrix3D rotation_x_matrix3D(float t);
LINEARALGEBRA_EXPORT Matrix4D rotation_x_matrix4D(float t);

LINEARALGEBRA_EXPORT Matrix3D rotation_y_matrix3D(float t);
LINEARALGEBRA_EXPORT Matrix4D rotation_y_matrix4D(float t);

LINEARALGEBRA_EXPORT Matrix3D rotation_z_matrix3D(float t);
LINEARALGEBRA_EXPORT Matrix4D rotation_z_matrix4D(float t);

LINEARALGEBRA_EXPORT Matrix3D scale_matrix3D(const Vector3D &v);
LINEARALGEBRA_EXPORT Matrix4D scale_matrix4D(const Vector3D &v);

LINEARALGEBRA_EXPORT Matrix3D scale_matrix3D_by_vector(const Matrix3D &m, const Vector3D &v);
LINEARALGEBRA_EXPORT Matrix4D scale_matrix4D_by_vector(const Matrix4D &m, const Vector3D &v);

LINEARALGEBRA_EXPORT Matrix4D translate(const Matrix4D &m, const Vector3D &v);
LINEARALGEBRA_EXPORT Matrix4D translation_matrix4D(const Vector3D &v);

LINEARALGEBRA_EXPORT Vector3D transform_vector(const Matrix4D &m, const Vector3D &v);
LINEARALGEBRA_EXPORT Vector3D transform_point(const Matrix4D &m, const Vector3D &p);
LINEARALGEBRA_EXPORT Plane transform_plane(const Matrix4D &H, const Plane &f);

LINEARALGEBRA_EXPORT Matrix4D MakeOrthoProjection(float l, float r, float t, float b, float n, float f);
LINEARALGEBRA_EXPORT Matrix4D LookAtMatrix(Vector3D observer_position, Vector3D observer_forward, Vector3D observer_up);
LINEARALGEBRA_EXPORT Matrix4D PerspectiveProjectionMatrix(float FOV, float z_near, float z_far, float aspect);

LINEARALGEBRA_EXPORT Vector3D Lerp(const Vector3D& v1, const Vector3D& v2,float factor);
LINEARALGEBRA_EXPORT Vector4D Lerp(const Vector4D& v1, const Vector4D& v2,float factor);
LINEARALGEBRA_EXPORT Quaternion Slerp(const Quaternion& q1, const Quaternion& q2,float t);


#endif //TRANSFORMATIONS_H
