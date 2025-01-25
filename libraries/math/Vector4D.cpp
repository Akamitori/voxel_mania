#include "Vector4D.h"

float dot(const Vector4D &v1, const Vector4D &v2) {
    return v1.x * v2.x +
           v1.y * v2.y +
           v1.z * v2.z +
           v1.w * v2.w;
}

Vector4D &operator *=(Vector4D &v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    v.w *= s;
    return v;
}

Vector4D &operator /=(Vector4D &v, float s) {
    s = 1.0F / s;
    v.x *= s;
    v.y *= s;
    v.z *= s;
    v.w *= s;
    return v;
}

Vector4D &operator +=(Vector4D &v1, const Vector4D &v2) {
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    v1.w += v2.w;
    return v1;
}

Vector4D &operator -=(Vector4D &v1, const Vector4D &v2) {
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    v1.w -= v2.w;
    return v1;
}

Vector4D operator *(const Vector4D &v, float s) {
    return (Vector4D(v.x * s, v.y * s, v.z * s, v.w * s));
}

Vector4D operator +(const Vector4D &a, const Vector4D &b) {
    return (Vector4D(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w));
}
