#ifndef MATH_OPS_H
#define MATH_OPS_H

#include <stdint.h>
#include "math_ops_export.h"

namespace math_ops {
    MATH_OPS_EXPORT bool is_equal(float a, float b, float EPSILON = 0.000001f);

    MATH_OPS_EXPORT bool is_equal(double a, double b, double EPSILON = 0.000001f);

    MATH_OPS_EXPORT float lerp(float start, float end, float factor);
    MATH_OPS_EXPORT double lerp(double factor, double start, double end);
    MATH_OPS_EXPORT double clamp(double value, double lower,double upper);
    
    MATH_OPS_EXPORT uint8_t max(uint8_t a, uint8_t b);
    MATH_OPS_EXPORT float max(float a, float b);
    MATH_OPS_EXPORT float min(float a, float b);

    MATH_OPS_EXPORT int max(int a, int b);
    MATH_OPS_EXPORT int min(int a, int b);
    
    MATH_OPS_EXPORT float ceil(float a); 
    MATH_OPS_EXPORT int ceil_to_int(float a);
    
    MATH_OPS_EXPORT float floor(float a);
    MATH_OPS_EXPORT int floor_to_int(float a);
    
    MATH_OPS_EXPORT float round(float a);
    MATH_OPS_EXPORT int round_to_int(float a);
}


#endif //MATH_OPS_H
