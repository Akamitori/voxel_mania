#ifndef VECTOR2D_H
#define VECTOR2D_H

#include "../export.h"

extern "C" {
struct EXPORTED Vector2D {
    float x;
    float y;
};

EXPORTED float dot(const Vector2D *a, const Vector2D *b);

EXPORTED void scale(Vector2D *a, float scale);

EXPORTED Vector2D negate(const Vector2D *a);

EXPORTED Vector2D add(const Vector2D *a, const Vector2D *b);

EXPORTED Vector2D subtract(const Vector2D *a, const Vector2D *b);

EXPORTED void add_store(Vector2D *a, const Vector2D *b);

EXPORTED void sub_store(Vector2D *a, const Vector2D *b);
}
#endif //VECTOR2D_H
