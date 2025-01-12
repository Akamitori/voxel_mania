#ifndef Vector3D_H
#define Vector3D_H

#include "export.h"

struct EXPORTED Vector3D {
    float x;
    float y;
    float z;
};

EXPORTED float dot(const Vector3D &a, const Vector3D &b);

EXPORTED float magnitude(const Vector3D &v);

EXPORTED float magnitude_squared(const Vector3D &v);

EXPORTED Vector3D normalize(const Vector3D &v);

EXPORTED Vector3D cross(const Vector3D& a, const Vector3D& b);

EXPORTED float distance_between_point_and_line(const Vector3D& point, const Vector3D& point_line, const Vector3D& direction_line);

EXPORTED Vector3D &operator *=(Vector3D &v, float s);

EXPORTED Vector3D &operator /=(Vector3D &v, float s);

EXPORTED Vector3D &operator +=(Vector3D &v1, const Vector3D &v2);

EXPORTED Vector3D &operator -=(Vector3D &v1, const Vector3D &v2);

EXPORTED Vector3D operator *(const Vector3D &v, float s);

EXPORTED Vector3D operator /(const Vector3D &v, float s);

EXPORTED Vector3D operator -(const Vector3D &v);

EXPORTED Vector3D operator +(const Vector3D &a, const Vector3D &b);

EXPORTED Vector3D operator -(const Vector3D &a, const Vector3D &b);


#endif //Vector3D_H
