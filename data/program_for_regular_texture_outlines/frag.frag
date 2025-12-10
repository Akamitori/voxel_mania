#version 330 core
out vec4 FragColor;

uniform sampler2D ourTexture;
uniform vec4 voxel_color;


void main() {
    FragColor = vec4(0.0, 1.0, 1.0, 1.0);  // Solid teal

}