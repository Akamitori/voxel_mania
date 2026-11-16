//
// Created by PETROS on 15/10/2024.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "Vector3D.h"
#include "renderer_export.h"
#include "Matrix4D.h"

struct Matrix4D;

struct Camera {
    const Vector3D basis_right{1, 0, 0.f};
    const Vector3D basis_forward{0, 1, 0};
    const Vector3D basis_up{0.0f, 0, -1.0f};
    Vector3D position{0.0f, -4, 0.0f};
    Vector3D forward{basis_forward};
    Vector3D right{basis_right};
    Vector3D up{basis_up};
    Matrix4D Camera_Matrix{};
    Matrix4D Camera_Matrix_Inverse{};

    float camera_speed{0.15f};
    float azimuth = 0;
    float elevation = 0;
    float rotation_speed_y{0.05};
    float rotation_speed_x{0.05};
    // 45 degrees to radians= 0.785398163f
    const float max_elevation_rotation{0.785398163f};
};

RENDERER_EXPORT void MoveCameraZ(Camera &camera, float modifier);

RENDERER_EXPORT void MoveCameraX(Camera &camera, float modifier);

RENDERER_EXPORT void MoveCameraY(Camera &camera, float modifier);

RENDERER_EXPORT Matrix4D CameraLookAtMatrix(const Camera &camera);

RENDERER_EXPORT void RotateCamera(Camera &camera, short azimuth_modifier, short elevation_modifier);

RENDERER_EXPORT void PrintPosition(const Camera &camera); 




#endif //CAMERA_H
