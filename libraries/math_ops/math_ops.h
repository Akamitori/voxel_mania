#ifndef MATH_OPS_H
#define MATH_OPS_H

#include <stdint.h>
#include "export.h"

namespace math_ops {
    EXPORTED bool is_equal(float a, float b, float EPSILON = 0.000001f);

    EXPORTED bool is_equal(double a, double b, double EPSILON = 0.000001f);

    EXPORTED float lerp(float value, float start, float end);
    EXPORTED double lerp(double value, double start, double end);
    EXPORTED double clamp(double value, double lower,double upper);
    
    EXPORTED uint8_t max(uint8_t a, uint8_t b);
}


#endif //MATH_OPS_H
