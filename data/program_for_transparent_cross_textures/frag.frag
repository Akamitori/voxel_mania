#version 330 core
out vec4 FragColor;

smooth in vec4 theColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;


void main() {
    vec4 texColor=texture(ourTexture, TexCoord);
    
    if(texColor.a < 1){
        discard;
    }
    
    FragColor= texColor;
}