#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D ourTexture;
uniform vec4 voxel_color;


void main() {
    vec4 texColor=texture(ourTexture, TexCoord);
    FragColor= texColor * voxel_color;
}