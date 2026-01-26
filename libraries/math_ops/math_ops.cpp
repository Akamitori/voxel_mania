//
// Created by PETROS on 29/10/2024.
//

#include "math_ops.h"

#include <math.h>

// TODO revisit this function
// source : https://randomascii.wordpress.com/2012/09/09/game-developer-magazine-floating-point/
bool math_ops::is_equal(const float a, const float b, const float EPSILON) {
    return fabs((double) a - (double) b) <= EPSILON;
}

// TODO revisit this function
// source : https://randomascii.wordpress.com/2012/09/09/game-developer-magazine-floating-point/
bool math_ops::is_equal(const double a, const double b, const double EPSILON) {
    return fabs(a - b) <= EPSILON;
}

float math_ops::lerp(const float value, const float start, const float end) {
    return start + value * (end - start);
}

double math_ops::lerp(const double value, const double start, const double end) {
    return start + value * (end - start);
}

double math_ops::clamp(const double value, const double lower, const double upper) {
    return fmin(upper, fmax(lower, value));
}

uint8_t math_ops::max(const uint8_t a, const uint8_t b) {
    return a > b ? a : b;
}
