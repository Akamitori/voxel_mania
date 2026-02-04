#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H

#include "export.h"
#include "Vector3D.h"

struct Matrix3D;
struct Matrix4D;

struct Vector3D;
struct Plane;

EXPORTED Matrix3D rotation_x_matrix3D(float t);
EXPORTED Matrix4D rotation_x_matrix4D(float t);

EXPORTED Matrix3D rotation_y_matrix3D(float t);
EXPORTED Matrix4D rotation_y_matrix4D(float t);

EXPORTED Matrix3D rotation_z_matrix3D(float t);
EXPORTED Matrix4D rotation_z_matrix4D(float t);

EXPORTED Matrix3D scale_matrix3D(const Vector3D &v);
EXPORTED Matrix4D scale_matrix4D(const Vector3D &v);

EXPORTED Matrix3D scale_matrix3D_by_vector(const Matrix3D &m, const Vector3D &v);
EXPORTED Matrix4D scale_matrix4D_by_vector(const Matrix4D &m, const Vector3D &v);

EXPORTED Matrix4D translate(const Matrix4D &m, const Vector3D &v);
EXPORTED Matrix4D translation_matrix4D(const Vector3D &v);

EXPORTED Vector3D transform_vector(const Matrix4D &m, const Vector3D &v);
EXPORTED Vector3D transform_point(const Matrix4D &m, const Vector3D &p);
EXPORTED Plane transform_plane(const Matrix4D &H, const Plane &f);

EXPORTED Matrix4D MakeOrthoProjection(float left, float right, float top, float bottom, float near, float far);
EXPORTED Matrix4D LookAtMatrix(Vector3D observer_position, Vector3D obverver_forward, Vector3D observer_up);



#endif //TRANSFORMATIONS_H
