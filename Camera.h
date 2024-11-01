//
// Created by PETROS on 15/10/2024.
//

#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "math_ops.h"


struct Camera;

void MoveCameraZ(Camera &camera, float modifier);

void MoveCameraX(Camera &camera, float modifier);

void MoveCameraY(Camera &camera, float modifier);

glm::mat4 CameraLookAtMatrix(const Camera &camera);

glm::mat4 PerspectiveMatrix(float FOV, float z_near, float z_far, float aspect);

void PerspectiveMatrixUpdate(glm::mat4 &perspectiveMatrix, float FOV, float aspect);

void RotateCamera(Camera & camera, short azimuth_modifier, short elevation_modifier);


struct Camera {
    const glm::vec3 basis_forward{0, 0, 1.f};
    const glm::vec3 basis_right{1, 0, 0.f};
    const glm::vec3 basis_up{0.0f, -1.0f, 0.0f};
    glm::vec3 position{0.0f, 0.0f, -4.0f};
    glm::vec3 forward{basis_forward};
    glm::vec3 right{basis_right};
    glm::vec3 up{basis_up};

    float camera_speed{0.05f};
    float azimuth = 0;
    float elevation = 0;
    float rotation_speed_y{0.05};
    float rotation_speed_x{0.05};
    const float max_elevation_rotation{glm::radians(45.f)};
};


#endif //CAMERA_H
