﻿//
// Created by PETROS on 15/10/2024.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "Vector3D.h"

struct Camera;
struct Matrix4D;

void MoveCameraZ(Camera &camera, float modifier);

void MoveCameraX(Camera &camera, float modifier);

void MoveCameraY(Camera &camera, float modifier);

Matrix4D CameraLookAtMatrix(const Camera &camera);

Matrix4D PerspectiveMatrix(float FOV, float z_near, float z_far, float aspect);

Matrix4D MakeOrthoProjection(float l, float r, float t, float b, float n, float f);

void PerspectiveMatrixUpdate(Matrix4D &perspectiveMatrix, float FOV, float aspect);

void RotateCamera(Camera &camera, short azimuth_modifier, short elevation_modifier);

struct Camera {
    const Vector3D basis_right{1, 0, 0.f};
    const Vector3D basis_forward{0, 1, 0};
    const Vector3D basis_up{0.0f, 0, -1.0f};
    Vector3D position{0.0f, -4, 0.0f};
    Vector3D forward{basis_forward};
    Vector3D right{basis_right};
    Vector3D up{basis_up};

    float camera_speed{0.05f};
    float azimuth = 0;
    float elevation = 0;
    float rotation_speed_y{0.05};
    float rotation_speed_x{0.05};
    // 45 degrees to radians= 0.785398163f
    const float max_elevation_rotation{0.785398163f};
};


#endif //CAMERA_H
