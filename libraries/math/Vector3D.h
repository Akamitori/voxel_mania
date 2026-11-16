#ifndef Vector3D_H
#define Vector3D_H

#include "linearalgebra_export.h"

struct LINEARALGEBRA_EXPORT Vector3D {
    float x;
    float y;
    float z;
};

struct Vector4D;

LINEARALGEBRA_EXPORT float dot(const Vector3D &a, const Vector3D &b);

LINEARALGEBRA_EXPORT float magnitude(const Vector3D &v);

LINEARALGEBRA_EXPORT float magnitude_squared(const Vector3D &v);

LINEARALGEBRA_EXPORT Vector3D normalize(const Vector3D &v);

LINEARALGEBRA_EXPORT Vector3D cross(const Vector3D &a, const Vector3D &b);

LINEARALGEBRA_EXPORT float distance_between_point_and_line(const Vector3D &point, const Vector3D &point_line, const Vector3D &direction_line);

LINEARALGEBRA_EXPORT Vector3D &operator *=(Vector3D &v, float s);

LINEARALGEBRA_EXPORT Vector3D &operator /=(Vector3D &v, float s);

LINEARALGEBRA_EXPORT Vector3D &operator +=(Vector3D &v1, const Vector3D &v2);

LINEARALGEBRA_EXPORT Vector3D &operator -=(Vector3D &v1, const Vector3D &v2);

LINEARALGEBRA_EXPORT Vector3D operator *(const Vector3D &v, float s);

LINEARALGEBRA_EXPORT Vector3D operator /(const Vector3D &v, float s);

LINEARALGEBRA_EXPORT Vector3D operator -(const Vector3D &v);

LINEARALGEBRA_EXPORT Vector3D operator +(const Vector3D &a, const Vector3D &b);

LINEARALGEBRA_EXPORT Vector3D operator -(const Vector3D &a, const Vector3D &b);

LINEARALGEBRA_EXPORT Vector4D Vector3D_To_Vector4D(const Vector3D &v, float w);

#endif //Vector3D_H
