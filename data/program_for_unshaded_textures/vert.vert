#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform vec3 position;

layout (std140) uniform ViewMatrices{
    mat4 perspective_projection_matrix;// 16*4 bytes
    mat4 look_at_matrix;// 16*4 bytes
};

out vec2 TexCoord;
out vec3 Normal;

void main() {

    mat4 model_matrix=mat4(1.0);
    model_matrix[3]=vec4(position, 1);

    vec4 final_pos=perspective_projection_matrix*look_at_matrix*model_matrix * vec4(aPos, 1.0);

    gl_Position = final_pos;
    
    TexCoord = aTexCoord;
}