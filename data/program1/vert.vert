#version 330 core

layout(location = 0) in vec3 aPos;
uniform mat4 perspectiveMatrix;


void main() {
    gl_Position = perspectiveMatrix*vec4(aPos, 1.0);
}