#ifndef VECTOR2D_H
#define VECTOR2D_H

struct Vector2D {
    double x;
    double y;

    explicit Vector2D(const double x, const double y) : x{x}, y{y} {
    }

    friend double Dot(const Vector2D &a, const Vector2D &b);
};

inline double Dot(const Vector2D &a, const Vector2D &b) {
    return a.x * b.x + a.x * b.x;
}


#endif //VECTOR2D_H
