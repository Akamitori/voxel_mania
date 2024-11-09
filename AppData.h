//
// Created by PETROS on 19/11/2024.
//

#ifndef APPDATA_H
#define APPDATA_H

#include "Camera.h"

struct AppData {
    Camera camera{};
    glm::mat4 camera_matrix{};
    glm::mat4 perspective_matrix{};
    glm::mat4 view_projection_matrix{};
    float FOV = 45;
    bool view_dirty{};
};



#endif //APPDATA_H
