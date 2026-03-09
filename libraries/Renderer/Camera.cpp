//
// Created by PETROS on 15/10/2024.
//
#include "Camera.h"
#include <cassert>
#include <math.h>

#include "Matrix4D.h"
#include "math_ops.h"
#include "Trigonometry.h"
#include "stdio.h"
#include "Transformations.h"

//TODO unify those functions after I move to SDL2
void MoveCameraZ(Camera &camera, const float modifier) {
    camera.position += camera.forward * modifier * camera.camera_speed;
    PrintPosition(camera);
}

void MoveCameraX(Camera &camera, const float modifier) {
    camera.position += camera.right * modifier * camera.camera_speed;
    PrintPosition(camera);
}

void MoveCameraY(Camera &camera, const float modifier) {
    camera.position += camera.up * modifier * camera.camera_speed;
    PrintPosition(camera);
}

void PrintPosition(const Camera &camera) {
    fprintf(stdout, "Camera position: (%f, %f,%f) \n", camera.position.x, camera.position.y, camera.position.z);
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
        camera.elevation = math_ops::clamp(camera.elevation, -camera.max_elevation_rotation, camera.max_elevation_rotation);
    }

    if (math_ops::is_equal(camera.azimuth, 0) && math_ops::is_equal(camera.elevation, 0)) {
        camera.right = camera.basis_right;
        camera.up = camera.basis_up;
        camera.forward = camera.basis_forward;
        return;
    }

    const double cos_azi = cosf(camera.azimuth);
    const double sin_azi = sin(camera.azimuth);

    const double cos_elev = cos(camera.elevation);
    const double sin_elev = sin(camera.elevation);

    camera.forward = normalize(
        (camera.basis_forward * cos_azi + camera.basis_right * sin_azi) * cos_elev + camera.basis_up * sin_elev
    );
    camera.right = normalize(cross(camera.basis_up, camera.forward));
    camera.up = normalize(cross(camera.forward, camera.right));
}


Matrix4D CameraLookAtMatrix(const Camera &camera) {
    return LookAtMatrix(camera.position, camera.forward, camera.up);
}


