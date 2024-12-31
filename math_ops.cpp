//
// Created by PETROS on 29/10/2024.
//

#include "math_ops.h"

#include <cmath>

// TODO revisit this function
// source : https://randomascii.wordpress.com/2012/09/09/game-developer-magazine-floating-point/
bool math_ops::is_equal(const float a, const float b, const float EPSILON) {
    return std::abs(a - b) <= EPSILON;
}

// TODO revisit this function
// source : https://randomascii.wordpress.com/2012/09/09/game-developer-magazine-floating-point/
bool math_ops::is_equal(const double a, const double b, const double EPSILON) {
    return std::abs(a - b) <= EPSILON;
}

float math_ops::lerp(float value, float start, float end) {
    return start + value * (end - start);
}

double math_ops::lerp(double value, double start, double end) {
    return start + value * (end - start);
}
