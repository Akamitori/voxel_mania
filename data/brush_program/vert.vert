#version 330 core

layout (location = 0) in vec3 aPos;


layout (std140) uniform ViewMatrices{
    mat4 perspective_projection_matrix;// 16*4 bytes
    mat4 look_at_matrix;// 16*4 bytes
};



out vec3 PointColor;
uniform vec3 color;
uniform vec3 translation;

void main() {
    gl_Position = perspective_projection_matrix*look_at_matrix * vec4(aPos+translation, 1.0);
    PointColor = color;
}