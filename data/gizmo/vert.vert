#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

layout (std140) uniform ViewMatrices{
    mat4 perspective_projection_matrix;
    mat4 look_at_matrix;
};

uniform mat4 model_matrix;

out vec3 vColor;

void main() {
    gl_Position = perspective_projection_matrix * look_at_matrix * model_matrix * vec4(aPos, 1.0);
    vColor = aColor;
}