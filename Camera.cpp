//
// Created by PETROS on 15/10/2024.
//

#include "Camera.h"

#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>


#include "math_ops.h"


//TODO unify those functions after I move to SDL2
void MoveCameraZ(Camera &camera, const float modifier) {
    camera.position += camera.forward * modifier * camera.camera_speed;
}

void MoveCameraX(Camera &camera, const float modifier) {
    camera.position += camera.right * modifier * camera.camera_speed;
}

void MoveCameraY(Camera &camera, float modifier) {
    camera.position += camera.up * modifier * camera.camera_speed;
}


std::ostream &operator<<(std::ostream &lhs, const glm::vec3 &vector) {
    lhs << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
    return lhs;
}

void RotateCamera(Camera &camera, const short azimuth_modifier, const short elevation_modifier) {
    assert(("Rotation invoked without any rotation taking place", azimuth_modifier != 0 || elevation_modifier != 0));
    assert(
        ("Elevation is 90 degrees which is not covered", !math_ops::is_equal(camera.max_elevation_rotation, glm::radians
            (90.f))));

    if (azimuth_modifier != 0) {
        camera.azimuth += static_cast<float>(azimuth_modifier) * camera.rotation_speed_y;
        camera.azimuth = math_ops::normalize_radians(camera.azimuth);
    }

    if (elevation_modifier != 0) {
        camera.elevation += static_cast<float>(elevation_modifier) * camera.rotation_speed_x;
        camera.elevation = math_ops::normalize_radians(camera.elevation);
        camera.elevation = std::clamp(camera.elevation, -camera.max_elevation_rotation, camera.max_elevation_rotation);
    }

    if (math_ops::is_equal(camera.azimuth, 0) && math_ops::is_equal(camera.elevation, 0)) {
        camera.right = camera.basis_right;
        camera.up = camera.basis_up;
        camera.forward = camera.basis_forward;
        return;
    }

    float cos_azi = glm::cos(camera.azimuth);
    float sin_azi = glm::sin(camera.azimuth);

    float cos_elev = glm::cos(camera.elevation);
    float sin_elev = glm::sin(camera.elevation);

    camera.forward = glm::normalize(
        cos_elev * (cos_azi * camera.basis_forward + sin_azi * camera.basis_right) + sin_elev * camera.basis_up);

    camera.right = glm::normalize(glm::cross(camera.forward, camera.basis_up));
    camera.up = glm::normalize(glm::cross(camera.right, camera.forward));
}

glm::mat4 CameraLookAtMatrix(const Camera &camera) {
    const glm::vec3 camera_target = camera.position + camera.forward;
    return glm::lookAt(camera.position, camera_target, camera.up);
}

glm::mat4 PerspectiveMatrix(const float FOV, const float z_near, const float z_far, const float aspect) {
    glm::mat4 perspectiveMatrix(0);

    const float tan_fov_2 = glm::tan(glm::radians(FOV / 2));
    const float tan_fov_2_invert = 1 / tan_fov_2;
    const float aspect_inverse = 1 / aspect;

    const float z_diff = 1 / (z_far - z_near);
    perspectiveMatrix[0].x = tan_fov_2_invert * aspect_inverse;
    perspectiveMatrix[1].y = tan_fov_2_invert;
    perspectiveMatrix[2].z = -(z_far + z_near) * z_diff;
    perspectiveMatrix[2].w = -1;
    perspectiveMatrix[3].z = -2 * z_near * z_far * z_diff;
    return perspectiveMatrix;
}

void PerspectiveMatrixUpdate(glm::mat4 &perspectiveMatrix, float FOV, float aspect) {
    const float aspect_inverse = 1 / aspect;
    const float tan_fov_2_invert = 1.f / glm::tan(glm::radians(FOV / 2));
    perspectiveMatrix[0].x = tan_fov_2_invert * aspect_inverse;
    perspectiveMatrix[1].y = tan_fov_2_invert;
}
