#version 330 core
    out vec4 FragColor;

smooth in vec4 theColor;

void main() {
    FragColor = theColor;
}