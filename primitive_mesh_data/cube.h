//
// Created by PETROS on 22/10/2024.
//

#ifndef CUBE_H
#define CUBE_H
#include <array>


struct cube {
    static constexpr int vertices_count = 8 * 3;
    static constexpr float vertex_data[vertices_count]{
        -0.5f, 0.5f, -0.5f,

        -0.5f, -0.5f, -0.5f,

        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,

        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,

        0.5f, 0.5f, 0.5f,
        0.5f, -0.5f, 0.5f
    };

    static constexpr int indices_count = 12 * 3;
    static constexpr int vertex_indices[indices_count]{
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
