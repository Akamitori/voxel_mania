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

float math_ops::lerp(const float start, const float end, const float factor) {
    return start + factor * (end - start);
}

double math_ops::lerp(const double factor, const double start, const double end) {
    return start + factor * (end - start);
}

double math_ops::clamp(const double value, const double lower, const double upper) {
    return fmin(upper, fmax(lower, value));
}

uint8_t math_ops::max(const uint8_t a, const uint8_t b) {
    return a > b ? a : b;
}

float math_ops::max(const float a, const float b) {
    return a > b ? a : b;
}

float math_ops::min(const float a, const float b) {
    return a < b ? a : b;
}

float math_ops::ceil(const float a) {
    return static_cast<float>(static_cast<int>(a) + 1);
}

int math_ops::ceil_to_int(const float a) {
    return static_cast<int>(a)+1;
}

float math_ops::floor(const float a) {
    return static_cast<float>(static_cast<int>(a));
}

int math_ops::floor_to_int(const float a) {
    return static_cast<int>(a);
}

float math_ops::round(const float a) {
    const float a_c=ceil(a);
    const float a_f=floor(a);
    const float dif= a-a_f;
    const float mod= dif>=0.5;
    return mod*a_c+(1-mod)*a_f;
}

int math_ops::round_to_int(const float a) {
    const float a_c=ceil(a);
    const float a_f=floor(a);
    const float dif= a-a_f;
    const float mod= dif>=0.5;
    return (int)(mod*a_c+(1-mod)*a_f);
}
