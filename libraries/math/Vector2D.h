#ifndef VECTOR2D_H
#define VECTOR2D_H

#include "linearalgebra_export.h"

struct Vector2D {
    float x;
    float y;
};

LINEARALGEBRA_EXPORT float dot(const Vector2D *a, const Vector2D *b);
#endif //VECTOR2D_H
