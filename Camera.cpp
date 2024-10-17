//
// Created by PETROS on 15/10/2024.
//

#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

constexpr float camera_speed = 0.05f;

void MoveCameraZ(Camera &camera, float modifier) {
    camera.cameraPos += camera.camera_front * modifier * camera_speed;
}

void MoveCameraX(Camera &camera, float modifier) {
    glm::vec3 camera_right = glm::normalize(glm::cross(camera.camera_front, camera.camera_up));
    camera.cameraPos += camera_right * modifier * camera_speed;
}

glm::mat4 CameraLookAtMatrix(Camera &camera) {
    return glm::lookAt(camera.cameraPos, camera.cameraPos + camera.camera_front, camera.camera_up);
}

glm::mat4 PerspectiveMatrix(float FOV, float z_near, float z_far, float width, float height) {
    glm::mat4 perspectiveMatrix(0);

    float tan_fov_2=glm::tan (glm::radians(FOV/2));
    float tan_fov_2_invert=1/tan_fov_2;
    float aspect_inverse=height/width;

    float z_diff=1/(z_near-z_far);
    perspectiveMatrix[0].x=tan_fov_2_invert*aspect_inverse;
    perspectiveMatrix[1].y=tan_fov_2_invert;
    perspectiveMatrix[2].z=(z_far+z_near)*z_diff;
    perspectiveMatrix[2].w=-1;
    perspectiveMatrix[3].z=-2*z_near*z_far*z_diff;
    return perspectiveMatrix;
}

void PerspectiveMatrixUpdate(glm::mat4 &perspectiveMatrix, float FOV,float width, float height) {
    float aspect_inverse=height/width;
    float tan_fov_2_invert=1.f/glm::tan(glm::radians(FOV/2));
    perspectiveMatrix[0].x=tan_fov_2_invert*aspect_inverse;
    perspectiveMatrix[1].y=tan_fov_2_invert;
}

