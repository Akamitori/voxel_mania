#ifndef MATH_OPS_H
#define MATH_OPS_H

namespace math_ops {
    constexpr float PI = 3.14159265358979323846264338327950288;
    constexpr float TWOPI = 6.28318530717958647692528676655900576;
    constexpr float PI_angle = 180;

    float normalize_angles(float angle);

    float normalize_radians(float angle_in_radians);


    bool is_equal(float a, float b, float EPSILON = 0.000001f);

    bool is_equal(double a, double b, double EPSILON = 0.000001f);

    float lerp(float value, float start, float end);
    double lerp(double value, double start, double end);
}


#endif //MATH_OPS_H
