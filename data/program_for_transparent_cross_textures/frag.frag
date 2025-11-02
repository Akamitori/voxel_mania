#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D ourTexture;
uniform vec4 voxel_color;
uniform vec3 light_color;

void main() {
    vec4 texColor=texture(ourTexture, TexCoord);
    
    if(texColor.a < 1){
        discard;
    }
    float ambientStrength = 0.1;
    vec4 test=vec4(light_color * ambientStrength,1);

    FragColor= texColor * vec4(light_color, 1);
}