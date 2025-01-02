#include "Vector3D.h"

#include <cmath>

float dot(const Vector3D *a, const Vector3D *b) {
    return a->x * b->x + a->x * b->x + a->z * b->z;
}

float magnitude(const Vector3D &v) {
    return (sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
}

Vector3D normalize(const Vector3D &v) {
    return (v / magnitude(v));
}

Vector3D cross(const Vector3D &a, const Vector3D &b) {
    return (Vector3D(a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x));
}

Vector3D &operator *=(Vector3D &v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

Vector3D &operator /=(Vector3D &v, float s) {
    s = 1.0F / s;
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

Vector3D &operator +=(Vector3D &v1, const Vector3D &v2) {
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    return v1;
}

Vector3D &operator -=(Vector3D &v1, const Vector3D &v2) {
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    return v1;
}

Vector3D operator *(const Vector3D &v, float s) {
    return (Vector3D(v.x * s, v.y * s, v.z * s));
}

Vector3D operator /(const Vector3D &v, float s) {
    s = 1.0F / s;
    return (Vector3D(v.x * s, v.y * s, v.z * s));
}

Vector3D operator -(const Vector3D &v) {
    return (Vector3D(-v.x, -v.y, -v.z));
}

Vector3D operator +(const Vector3D &a, const Vector3D &b) {
    return (Vector3D(a.x + b.x, a.y + b.y, a.z + b.z));
}

Vector3D operator -(const Vector3D &a, const Vector3D &b) {
    return (Vector3D(a.x - b.x, a.y - b.y, a.z - b.z));
}
