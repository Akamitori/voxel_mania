//
// Created by PETROS on 15/10/2024.
//

#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

constexpr float camera_speed = 0.05f;

void MoveCameraZ(Camera &camera, const float modifier) {
    camera.position += camera.basis_front * modifier * camera_speed;
    camera.target = camera.position + camera.basis_front;
}

void MoveCameraX(Camera &camera, const float modifier) {
    camera.position += camera.basis_right * modifier * camera_speed;
    camera.target = camera.position + camera.basis_front;
}

glm::mat4 CameraLookAtMatrix(const Camera &camera) {
    return glm::lookAt(camera.position, camera.target , camera.basis_up);
}

glm::mat4 PerspectiveMatrix(const float FOV, const float z_near, const float z_far, const float aspect) {
    glm::mat4 perspectiveMatrix(0);

    const float tan_fov_2=glm::tan (glm::radians(FOV/2));
    const float tan_fov_2_invert=1/tan_fov_2;
    const float aspect_inverse=1/aspect;

    const float z_diff=1/(z_far-z_near);
    perspectiveMatrix[0].x=tan_fov_2_invert*aspect_inverse;
    perspectiveMatrix[1].y=tan_fov_2_invert;
    perspectiveMatrix[2].z=-(z_far+z_near)*z_diff;
    perspectiveMatrix[2].w=-1;
    perspectiveMatrix[3].z=-2*z_near*z_far*z_diff;
    return perspectiveMatrix;
}

void PerspectiveMatrixUpdate(glm::mat4 &perspectiveMatrix, float FOV,float aspect) {
    const float aspect_inverse=1/aspect;
    const float tan_fov_2_invert=1.f/glm::tan(glm::radians(FOV/2));
    perspectiveMatrix[0].x=tan_fov_2_invert*aspect_inverse;
    perspectiveMatrix[1].y=tan_fov_2_invert;
}

