#include "Trigonometry.h"
#include <cmath>


constexpr float DegreeToRadians(float degrees) {
    return TO_RADIANS_MULTIPLIER * degrees;
}

constexpr float RadiansToDegrees(float radians) {
    return TO_DEGREES_MULTIPLIER * radians;
}

constexpr float normalize_angle(float angle) {
    return RadiansToDegrees(normalize_radians(DegreeToRadians(angle)));
}

constexpr float fabs_const(float value) {
    return (value < 0.0f) ? -value : value;
}

constexpr float normalize_radians(float radians) {
    if (fabs_const(radians) > PI) {
        radians += PI;

        radians -= std::floor(radians / TWOPI) * TWOPI;

        radians -= PI;
    }

    return radians;
}
