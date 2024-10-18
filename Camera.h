//
// Created by PETROS on 15/10/2024.
//

#ifndef CAMERA_H
#define CAMERA_H
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

struct Camera;

void MoveCameraZ(Camera &camera, float modifier);
void MoveCameraX(Camera &camera, float modifier);

glm::mat4 CameraLookAtMatrix(const Camera &camera);
glm::mat4 PerspectiveMatrix(float FOV, float z_near, float z_far, float aspect);
void PerspectiveMatrixUpdate(glm::mat4 &perspectiveMatrix, float FOV,float aspect);

struct Camera {
    glm::vec3 position{0.0f, 0.0f, 3.0f};
    glm::vec3 basis_front{0,0,-1.f};
    glm::vec3 basis_right{1,0,0.f};
    glm::vec3 basis_up{0.0f, 1.0f,  0.0f};
    glm::vec3 target = position + basis_front;
    float cameraSpeed=0.5f;
};


#endif //CAMERA_H
