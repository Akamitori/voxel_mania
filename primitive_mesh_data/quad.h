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

    static constexpr int vertices_count_uv = 8 * 8;
    static constexpr float vertex_data_uv_1_part_texture[vertices_count_uv]{
        -0.5f, 0, 0.5f,   0,-1,0,      0,1,
        -0.5f, 0, -0.5f,  0,-1,0,      0,0.f,
        0.5f, 0, -0.5f,   0,-1,0,      1,0.f,
        0.5f, 0, 0.5f,    0,-1,0,      1,1.0f,

        -0.5f, 0, 0.5f,   0,1,0,      0,1,
        -0.5f, 0, -0.5f,  0,1,0,      0,0.f,
        0.5f, 0, -0.5f,   0,1,0,      1,0.f,
        0.5f, 0, 0.5f,    0,1,0,      1,1.0f,
    };

    static constexpr int vertex_indices_count_uv=12;
    static constexpr int vertex_indices_uvs[vertex_indices_count_uv]{
        // front face
        0, 1, 2,
        0, 2, 3,

        //back face
        6,5,4,
        7,6,4
    };

};


#endif //QUAD_H
