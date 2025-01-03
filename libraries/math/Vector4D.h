#ifndef VECTOR4D_H
#define VECTOR4D_H

#include "export.h"

struct Vector4D
{
    float		x, y, z, w;
};

EXPORTED Vector4D& operator *=(Vector4D& v,float s);

EXPORTED Vector4D& operator /=(Vector4D& v,float s);

EXPORTED Vector4D& operator +=(Vector4D& v1, const Vector4D& v2);

EXPORTED Vector4D& operator -=(Vector4D& v1,const Vector4D& v2);

EXPORTED Vector4D operator *(const Vector4D &v, float s);

EXPORTED Vector4D operator +(const Vector4D &a, const Vector4D &b);


#endif //VECTOR4D_H
