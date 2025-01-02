//
// Created by PETROS on 15/10/2024.
//

#include "Camera.h"

#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>


#include "math_ops.h"
#include "Trigonometry.h"


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

std::ostream &operator<<(std::ostream &lhs, const Vector3D &vector) {
    lhs << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
    return lhs;
}

glm::vec3 from_vector3D(const Vector3D &vec) {
    return glm::vec3{vec.x, vec.y, vec.z};
}

void RotateCamera(Camera &camera, const short azimuth_modifier, const short elevation_modifier) {
    assert(("Rotation invoked without any rotation taking place", azimuth_modifier != 0 || elevation_modifier != 0));
    assert(
        ("Elevation is 90 degrees which is not covered", !math_ops::is_equal(camera.max_elevation_rotation, glm::radians
            (90.f))));

    if (azimuth_modifier != 0) {
        camera.azimuth += static_cast<float>(azimuth_modifier) * camera.rotation_speed_y;
        camera.azimuth = normalize_radians(camera.azimuth);
    }

    if (elevation_modifier != 0) {
        camera.elevation += static_cast<float>(elevation_modifier) * camera.rotation_speed_x;
        camera.elevation = normalize_radians(camera.elevation);
        camera.elevation = std::clamp(camera.elevation, -camera.max_elevation_rotation, camera.max_elevation_rotation);
    }

    if (math_ops::is_equal(camera.azimuth, 0) && math_ops::is_equal(camera.elevation, 0)) {
        camera.right = camera.basis_right;
        camera.up = camera.basis_up;
        camera.forward = camera.basis_forward;
        return;
    }

    const float cos_azi = cos(camera.azimuth);
    const float sin_azi = sin(camera.azimuth);

    const float cos_elev = cos(camera.elevation);
    const float sin_elev = sin(camera.elevation);

    camera.forward = normalize(
        (camera.basis_forward * cos_azi + camera.basis_right * sin_azi) * cos_elev + camera.basis_up * sin_elev
    );
    camera.right = normalize(cross(camera.forward, camera.basis_up));
    camera.up = normalize(cross(camera.right, camera.forward));
}

glm::mat4 CameraLookAtMatrix(const Camera &camera) {
    const Vector3D camera_target = camera.position + camera.forward;
    auto cam_pos_glm = from_vector3D(camera.position);
    auto camera_target_glm = from_vector3D(camera_target);
    auto camera_up_glm = from_vector3D(camera.up);
    return glm::lookAt(cam_pos_glm, camera_target_glm, camera_up_glm);
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
