//
// Created by PETROS on 19/11/2024.
//

#ifndef APPDATA_H
#define APPDATA_H

#include "Matrix4D.h"
#include "Camera.h"
#include "Clipping.h"

struct AppData {
    Camera camera{};
    Matrix4D camera_matrix{};
    Matrix4D look_at_matrix_inverse{};
    Matrix4D perspective_matrix{};
    Matrix4D ortho_projection_matrix{};
    Matrix4D view_projection_matrix{};
    float FOV = 45;
    float z_near=0.1;
    float z_far=100;
    bool view_dirty{};
    int screen_width{};
    int screen_height{};
    Polyhedron frustum_polyhedron;
};



#endif //APPDATA_H
