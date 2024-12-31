#include "Trigonometry.h"
#include <cmath>

float DegreeToRadians(float degrees) {
    return TO_RADIANS_MULTIPLIER * degrees;
}

float RadiansToDegrees(float radians) {
    return TO_DEGREES_MULTIPLIER * radians;
}

float normalize_angle(float angle) {
    return RadiansToDegrees(normalize_radians(DegreeToRadians(angle)));
}

float normalize_radians(float radians) {
    if (fabs(radians) > PI) {
        radians += PI;

        radians -= floor(radians / TWOPI) * TWOPI;

        radians -= PI;
    }

    return radians;
}
