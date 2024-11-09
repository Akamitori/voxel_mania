//
// Created by PETROS on 29/10/2024.
//

#ifndef MATH_OPS_H
#define MATH_OPS_H
#include <glm/gtc/constants.hpp>


namespace math_ops {
    constexpr float PI = glm::pi<float>();
    constexpr float TWOPI =  glm::two_pi<float>();
    constexpr float PI_angle = 180;

    float normalize_angles(float angle);

    float normalize_radians(float angle_in_radians);

    bool is_equal(float a, float b, float EPSILON = 0.000001f);
}


#endif //MATH_OPS_H
