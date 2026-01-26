//
// Created by PETROS on 26/01/2026.
//

#ifndef VOXEL_MANIA_MAIN_CONTAINERS_H
#define VOXEL_MANIA_MAIN_CONTAINERS_H

#include "vector_container.h"
#include "Vector3D.h"

struct model_instance {
    int mesh_id{};
    Vector3D pos{};
};

VECTOR_DECLARATION_STATIC(model_instance);

#endif //VOXEL_MANIA_MAIN_CONTAINERS_H
