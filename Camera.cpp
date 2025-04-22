//
// Created by PETROS on 15/10/2024.
//
#include "Camera.h"
#include <algorithm>
#include <cassert>
#include <iostream>

#include "Matrix4D.h"
#include "math_ops.h"
#include "Transformations.h"
#include <Trigonometry.h>

std::ostream &operator<<(std::ostream &lhs, const Vector3D &vector) {
    lhs << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
    return lhs;
}

//TODO unify those functions after I move to SDL2
void MoveCameraZ(Camera &camera, const float modifier) {
    camera.position += camera.forward * modifier * camera.camera_speed;
    std::cout << camera.position << std::endl;
}

void MoveCameraX(Camera &camera, const float modifier) {
    camera.position += camera.right * modifier * camera.camera_speed;
    std::cout << camera.position << std::endl;
}

void MoveCameraY(Camera &camera, const float modifier) {
    camera.position += camera.up * modifier * camera.camera_speed;
    std::cout << camera.position << std::endl;
}


void RotateCamera(Camera &camera, const short azimuth_modifier, const short elevation_modifier) {
    assert(("Rotation invoked without any rotation taking place", azimuth_modifier != 0 || elevation_modifier != 0));
    assert(
        ("Elevation is 90 degrees which is not covered", !math_ops::is_equal(camera.max_elevation_rotation,
            DegreeToRadians
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
    camera.right = normalize(cross(camera.basis_up, camera.forward));
    camera.up = normalize(cross(camera.forward, camera.right));
}


Matrix4D CameraLookAtMatrix(const Camera &camera) {
    const Vector3D camera_target = camera.position + camera.forward;

    const Vector3D center = camera_target;
    const Vector3D eye = camera.position;

    const Vector3D f = normalize(center - eye); // Forward vector (in world space)
    const Vector3D s = normalize(cross(camera.up, f)); // Right
    const Vector3D u = cross(f, s);

    Matrix4D mat{
        {s.x, u.x, f.x, 0},
        {s.y, u.y, f.y, 0},
        {s.z, u.z, f.z, 0},
        {-dot(s, eye), -dot(u, eye), -dot(f, eye), 1}
    };
    return mat;
}

Matrix4D PerspectiveMatrix(const float FOV, const float z_near, const float z_far, const float aspect) {
    Matrix4D perspectiveMatrix(0);

    const float tan_fov_2_invert = 1 / tan(DegreeToRadians(FOV) * 0.5f);

    const float z_diff = 1 / (z_near - z_far);
    perspectiveMatrix[0].x = tan_fov_2_invert / aspect;
    perspectiveMatrix[1].y = -tan_fov_2_invert;
    perspectiveMatrix[2].z = (z_far + z_near) * z_diff;
    perspectiveMatrix[2].w = 1;
    perspectiveMatrix[3].z = -2 * z_near * z_far * z_diff;
    return perspectiveMatrix;
}

void PerspectiveMatrixUpdate(Matrix4D &perspectiveMatrix, const float FOV, const float aspect) {
    const float aspect_inverse = 1 / aspect;
    const float tan_fov_2_invert = 1.f / tan(DegreeToRadians(FOV / 2));
    perspectiveMatrix[0].x = tan_fov_2_invert * aspect_inverse;
    perspectiveMatrix[1].y = -tan_fov_2_invert;
}
