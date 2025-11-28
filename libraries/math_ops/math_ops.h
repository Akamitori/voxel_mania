#ifndef MATH_OPS_H
#define MATH_OPS_H

#include "export.h"

namespace math_ops {
    EXPORTED bool is_equal(float a, float b, float EPSILON = 0.000001f);

    EXPORTED bool is_equal(double a, double b, double EPSILON = 0.000001f);

    EXPORTED float lerp(float value, float start, float end);
    EXPORTED double lerp(double value, double start, double end);
}


#endif //MATH_OPS_H
