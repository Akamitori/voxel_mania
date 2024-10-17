#version 330 core
out vec4 FragColor;

uniform vec4 cursor_color;


void main() {
    FragColor = cursor_color; // Orange color
}