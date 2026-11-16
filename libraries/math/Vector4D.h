#ifndef VECTOR4D_H
#define VECTOR4D_H

#include "linearalgebra_export.h"

struct Vector4D {
    float x, y, z, w;
};

LINEARALGEBRA_EXPORT float dot(const Vector4D &v1, const Vector4D &v2);

LINEARALGEBRA_EXPORT Vector4D &operator *=(Vector4D &v, float s);

LINEARALGEBRA_EXPORT Vector4D &operator /=(Vector4D &v, float s);

LINEARALGEBRA_EXPORT Vector4D &operator +=(Vector4D &v1, const Vector4D &v2);

LINEARALGEBRA_EXPORT Vector4D &operator -=(Vector4D &v1, const Vector4D &v2);

LINEARALGEBRA_EXPORT Vector4D operator *(const Vector4D &v, float s);

LINEARALGEBRA_EXPORT Vector4D operator +(const Vector4D &a, const Vector4D &b);

LINEARALGEBRA_EXPORT Vector4D operator /(const Vector4D &v, float s);


#endif //VECTOR4D_H
