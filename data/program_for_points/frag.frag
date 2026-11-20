#version 330 core
out vec4 FragColor;

in vec3 PointColor;
in vec3 Normal;


void main() {
    FragColor= vec4(PointColor, 1);
}