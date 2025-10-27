//
// Created by PETROS on 22/10/2024.
//

#ifndef QUAD_H
#define QUAD_H


struct quad {
    static constexpr int vertices_count = 4 * 3;
    
    static constexpr float vertex_data_no_uvs[vertices_count]{
        -0.5f, 0, 0.5f,         
        -0.5f, 0, -0.5f,        
        0.5f, 0, -0.5f,         
        0.5f, 0, 0.5f,          
    };

    static constexpr int vertices_count_uv = 4 * 5;
    static constexpr float vertex_data_uv_1_part_texture[vertices_count_uv]{
        -0.5f, 0, 0.5f,         0,1,
        -0.5f, 0, -0.5f,        0,0.f,
        0.5f, 0, -0.5f,         1,0.f,
        0.5f, 0, 0.5f,          1,1.0f,
    };

    static constexpr int vertex_indices_count_uv=12;
    static constexpr int vertex_indices_uvs[vertex_indices_count_uv]{
        // front face
        0, 1, 2,
        0, 2, 3,

        2,1,0,
        3,2,0
    };

};


#endif //QUAD_H
