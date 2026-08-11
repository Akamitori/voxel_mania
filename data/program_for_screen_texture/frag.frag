#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D screenTexture;

float gamma_correct_color_component(float color_component){

    if (color_component<= 0.0031308){
        return 12.92*color_component;
    }

    return 1.055 * pow(color_component, 1/2.4) - 0.055;
}

vec4 gamma_correct_color(vec4 color){
    color.x=gamma_correct_color_component(color.x);
    color.y=gamma_correct_color_component(color.y);
    color.z=gamma_correct_color_component(color.z);
    return color;
}

void main()
{
    vec4 mapped_textured_color= texture(screenTexture, TexCoord);
    FragColor = gamma_correct_color(mapped_textured_color);
}