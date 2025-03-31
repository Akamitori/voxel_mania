#include "Trigonometry.h"
#include <cmath>


float DegreeToRadians(const float degrees) {
    return TO_RADIANS_MULTIPLIER * degrees;
}

float RadiansToDegrees(const float radians) {
    return TO_DEGREES_MULTIPLIER * radians;
}

float normalize_radians(float radians) {
    if (fabs_const(radians) > PI) {
        radians += PI;

        radians -= std::floor(radians / TWOPI) * TWOPI;

        radians -= PI;
    }

    return radians;
}

float normalize_angle(const float angle) {
    return RadiansToDegrees(normalize_radians(DegreeToRadians(angle)));
}

float fabs_const(const float value) {
    return value < 0.0f ? -value : value;
}
