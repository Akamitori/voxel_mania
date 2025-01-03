#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H


#include "export.h"

struct Matrix3D;
struct Matrix4D;

struct Vector3D;

EXPORTED Matrix3D rotation_x_matrix3D(float t);
EXPORTED Matrix4D rotation_x_matrix4D(float t);

EXPORTED Matrix3D rotation_y_matrix3D(float t);
EXPORTED Matrix4D rotation_y_matrix4D(float t);

EXPORTED Matrix3D rotation_z_matrix3D(float t);
EXPORTED Matrix4D rotation_z_matrix4D(float t);

EXPORTED Matrix3D scale_matrix3D(const Matrix3D &m, const Vector3D& v);
EXPORTED Matrix4D scale_matrix4D(const Matrix4D &m, const Vector3D& v);

EXPORTED Matrix4D translate(const Matrix4D &m, const Vector3D& v);

EXPORTED Matrix4D look_at(const Vector3D& eye, const Vector3D& center, const Vector3D& up);

      
#endif //TRANSFORMATIONS_H
