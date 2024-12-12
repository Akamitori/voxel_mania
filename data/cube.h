//
// Created by PETROS on 22/10/2024.
//

#ifndef CUBE_H
#define CUBE_H
#include <array>



struct cube {
    float vertex_data[8 * 3]{
        -0.5f, 0.5f, -0.5f,

        -0.5f, -0.5f, -0.5f,

        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,

        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,

        0.5f, 0.5f, 0.5f,
        0.5f, -0.5f, 0.5f
    };

    int total_vertices=36;

    int vertex_indices[12 * 3]{
        0, 3, 1,
        0, 2, 3,

        7, 4, 5,
        7, 6, 4,

        6, 7, 3,
        6, 3, 2,

        4, 0, 1,
        4, 1, 5,

        5, 1, 3,
        5, 3, 7,

        6, 2, 4,
        4, 2, 0,
    };
};


#endif //CUBE_H
