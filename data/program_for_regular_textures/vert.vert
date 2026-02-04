#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (std140) uniform ViewMatrices{
    mat4 perspective_projection_matrix;// 16*4 bytes
    mat4 look_at_matrix;// 16*4 bytes
};

uniform mat4 model_matrix;
uniform mat3 model_matrix_for_normals;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main() {

    gl_Position = perspective_projection_matrix*look_at_matrix*model_matrix * vec4(aPos, 1.0);

    // this is important for light calculations
    FragPos=vec3(model_matrix*vec4(aPos, 1.0));
    TexCoord = aTexCoord;
    Normal= model_matrix_for_normals*aNormal;

    // this works fine if we don't scale things
    // use a different shader if we do!
    // Normal = mat3(transpose(inverse(model_matrix))) * anormal;
    //Normal= mat3(model_matrix)*aNormal;
}