//
// Created by PETROS on 22/10/2024.
//

#ifndef CUBE_H
#define CUBE_H

struct cube {
    static constexpr int vertices_count = 8 * 3;
    static constexpr float vertex_data[vertices_count]{
        -0.5f, 0.5f, -0.5f, //0

        -0.5f, -0.5f, -0.5f, //1

        0.5f, 0.5f, -0.5f, //2
        0.5f, -0.5f, -0.5f, //3

        -0.5f, 0.5f, 0.5f, //4
        -0.5f, -0.5f, 0.5f, //5

        0.5f, 0.5f, 0.5f, //6
        0.5f, -0.5f, 0.5f //7
    };

    //static constexpr int vertices_count = 8 * 3;

    static constexpr int vertices_count_uv = 120;
    static constexpr float vertex_data_uv_3_part_texture[vertices_count_uv]{
        //vertices              //uv
        -0.5f, 0.5f, -0.5f,         0.25, 1, //0
        -0.5f, -0.5f, -0.5f,        0.25, 0, //1
        0.5f, 0.5f, -0.5f,          0, 1, //2
        0.5f, -0.5f, -0.5f,         0, 0, //3
        
        - 0.5f,0.5f, 0.5f,          0.5, 1, //4
        - 0.5f,-0.5f, 0.5f,         0.5, 0, //5
        0.5f, 0.5f, 0.5f,           0.75, 1, //6
        0.5f, -0.5f, 0.5f,          0.75, 0, //7

        
        // right face (x = +0.5)
        0.5f,  0.5f,  0.5f,          0.5f,  1, // 8  (front-top)
        0.5f, -0.5f,  0.5f,          0.25f,  1, // 9  (front-bottom)
        0.5f, -0.5f, -0.5f,          0.25f, 0, // 10 (back-bottom)
        0.5f,  0.5f, -0.5f,          0.5f, 0, // 11 (back-top)


        -0.5f, 0.5f, 0.5f,          0.25f,1, //12
        -0.5f, -0.5f, 0.5f,         0.5f,1,//13
        -0.5f, 0.5f, -0.5f,         0.25,0.f,//14
        -0.5f, -0.5f, -0.5f,        0.5,0.f,//15
        
        -0.5f, -0.5f, 0.5f,         0.25f,1,//  16
        -0.5f, -0.5f, -0.5f,        0.25,0.f,// 17
        0.5f, -0.5f, -0.5f,         0.5f,0.f,//  18
        0.5f, -0.5f, 0.5f,          0.5f,1.0f,//  19

        
        0.5f, 0.5f, 0.5f,           0.5f, 1.f,//20
        0.5f, 0.5f, -0.5f,          0.5f,0.f,//21
        -0.5f, 0.5f, 0.5f,          0.25f, 1.f,//22
        -0.5f, 0.5f, -0.5f,         0.25f, 0.f,//23
    };

    static constexpr float vertex_data_uv_1_part_texture[vertices_count_uv]{
        //vertices              //uv
        -0.5f, 0.5f, -0.5f,         1, 1, //0
        -0.5f, -0.5f, -0.5f,        1, 0, //1
        0.5f, 0.5f, -0.5f,          0, 1, //2
        0.5f, -0.5f, -0.5f,         0, 0, //3
        
        - 0.5f,0.5f, 0.5f,          0, 1, //4
        - 0.5f,-0.5f, 0.5f,         0, 0, //5
        0.5f, 0.5f, 0.5f,           1, 1, //6
        0.5f, -0.5f, 0.5f,          1, 0, //7

        
        // right face (x = +0.5)
        0.5f,  0.5f,  0.5f,          1,  1, // 8  (front-top)
        0.5f, -0.5f,  0.5f,          0,  1, // 9  (front-bottom)
        0.5f, -0.5f, -0.5f,          0, 0, // 10 (back-bottom)
        0.5f,  0.5f, -0.5f,          1, 0, // 11 (back-top)


        -0.5f, 0.5f, 0.5f,          0,1, //12
        -0.5f, -0.5f, 0.5f,         1,1,//13
        -0.5f, 0.5f, -0.5f,         0,0.f,//14
        -0.5f, -0.5f, -0.5f,        1,0.f,//15
        
        -0.5f, -0.5f, 0.5f,         0,1,//  16
        -0.5f, -0.5f, -0.5f,        0,0.f,// 17
        0.5f, -0.5f, -0.5f,         1,0.f,//  18
        0.5f, -0.5f, 0.5f,          1,1.0f,//  19

        
        0.5f, 0.5f, 0.5f,           1, 1.f,//20
        0.5f, 0.5f, -0.5f,          1,0.f,//21
        -0.5f, 0.5f, 0.5f,          0, 1.f,//22
        -0.5f, 0.5f, -0.5f,         0, 0.f,//23
    };

    //top face is in the range 0.25,0.75
    // and 1 for max

    static constexpr int vertex_indices_count_uv=36;
    static constexpr int vertex_indices_uvs[36]{
        //bottom face
        0, 3, 1,
        0, 2, 3,

        // top face
        7, 4, 5,
        7, 6, 4,

        // right face
        8, 9, 10,
        8, 10, 11,

        // left face
        12, 14, 15,
        12, 15, 13,

        // front face
        16, 17, 18,
        16, 18, 19,

        // back face
        20, 21, 22,
        22, 21, 23,

    };


    static constexpr int indices_count = 12 * 3;
    static constexpr int vertex_indices[indices_count]{

        // bottom face
        0, 3, 1,
        0, 2, 3,

        // top face
        7, 4, 5,
        7, 6, 4,

        // right face
        6, 7, 3,
        6, 3, 2,

        // left face
        4, 0, 1,
        4, 1, 5,

        // front face
        5, 1, 3,
        5, 3, 7,

        // back face
        6, 2, 4,
        4, 2, 0,
    };
};


#endif //CUBE_H
