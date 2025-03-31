#ifndef TRIGONOMETRY_H
#define TRIGONOMETRY_H

#include "export.h"

constexpr float PI = 3.14159265358979323846264338327950288;
constexpr float TWOPI = 6.28318530717958647692528676655900576;
constexpr float PI_angle = 180;
constexpr float TO_DEGREES_MULTIPLIER = 57.295779513082320876798154814105;
constexpr float TO_RADIANS_MULTIPLIER = 0.01745329251994329576923690768489;

EXPORTED float DegreeToRadians(float degrees);

EXPORTED float RadiansToDegrees(float radians);

EXPORTED float normalize_radians(float radians);

EXPORTED float normalize_angle(float angle);

EXPORTED float fabs_const(float value);

#endif //TRIGONOMETRY_H
