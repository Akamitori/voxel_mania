#ifndef VECTOR2D_H
#define VECTOR2D_H

#include "export.h"

struct EXPORTED Vector2D {
    float x;
    float y;
};

EXPORTED float dot(const Vector2D *a, const Vector2D *b);
#endif //VECTOR2D_H
