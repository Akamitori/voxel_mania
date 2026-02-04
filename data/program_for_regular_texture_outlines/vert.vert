#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;


layout (std140) uniform ViewMatrices{
    mat4 perspective_projection_matrix;// 16*4 bytes
    mat4 look_at_matrix;// 16*4 bytes
};

uniform mat4 model_matrix;

void main() {
    mat4 slight_scale_matrix=mat4(1.1);
    slight_scale_matrix[3].w=1;

    gl_Position=perspective_projection_matrix*look_at_matrix*model_matrix *slight_scale_matrix* vec4(aPos, 1.0);
}