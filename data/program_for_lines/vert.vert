// line vert
#version 330 core
layout (location = 0) in vec3 aPos;

layout (std140) uniform ViewMatrices {
    mat4 perspective_projection_matrix;
    mat4 look_at_matrix;
};

void main() {
    gl_Position = perspective_projection_matrix * look_at_matrix * vec4(aPos, 1.0);
}