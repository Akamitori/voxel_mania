//
// Created by PETROS on 26/01/2026.
//

#ifndef VOXEL_MANIA_MAIN_CONTAINERS_H
#define VOXEL_MANIA_MAIN_CONTAINERS_H

#include "vector_container.h"
#include "Renderer.h"

struct model_instance {
    int mesh_id{};
    Transform transform{};
};

VECTOR_IMPLEMENTATION_STATIC(model_instance)


#endif //VOXEL_MANIA_MAIN_CONTAINERS_H
