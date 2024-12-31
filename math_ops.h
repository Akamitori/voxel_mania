#ifndef MATH_OPS_H
#define MATH_OPS_H

namespace math_ops {
    bool is_equal(float a, float b, float EPSILON = 0.000001f);

    bool is_equal(double a, double b, double EPSILON = 0.000001f);

    float lerp(float value, float start, float end);
    double lerp(double value, double start, double end);
}


#endif //MATH_OPS_H
