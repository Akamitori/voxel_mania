#include "Vector2D.h"

float dot(const Vector2D *a, const Vector2D *b) {
    return a->x * b->x + a->x * b->x;
}

void scale(Vector2D *a, const float scale) {
    a->x *= scale;
    a->y *= scale;
}

Vector2D negate(const Vector2D *a) {
    return Vector2D{-a->x, -a->y};
}

Vector2D add(const Vector2D *a, const Vector2D *b) {
    return Vector2D{a->x + b->x, a->y + b->y};
}

Vector2D subtract(const Vector2D *a, const Vector2D *b) {
    return Vector2D{a->x - b->x, a->y - b->y};
}

void add_store(Vector2D *a, const Vector2D *b) {
    a->x += b->x;
    a->y += b->y;
}

void sub_store(Vector2D *a, const Vector2D *b) {
    a->x -= b->x;
    a->y -= b->y;
}
