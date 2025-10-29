#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform vec4 voxel_color;
uniform vec3 light_color;


void main() {
    vec4 texColor=texture(ourTexture, TexCoord);
    vec3 whatever= vec3(1,1,1);
    vec4 test=vec4(light_color,1);
    FragColor= texColor * voxel_color * test;
}