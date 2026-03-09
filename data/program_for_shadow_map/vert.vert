#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model_matrix;
uniform int light_space_matrix_index;


const int MAX_DIRECTIONAL_LIGHTS=1;
const int MAX_SHADOW_CASCADES=4;

struct DirectionalLight{
    vec3 direction;// 16
    vec3 ambient;// 16
    vec3 diffuse;// 16
    vec3 specular;// 16
// no padding
};

layout (std140) uniform Directional_Lights{
    int numDirectionalLights;// 16 
    DirectionalLight directional_lights[MAX_DIRECTIONAL_LIGHTS];// 64*max lights
    mat4 mvp_matrix_per_cascade[MAX_DIRECTIONAL_LIGHTS*MAX_SHADOW_CASCADES];// 64*max lights
};


// we can do as many passes as we want as long as we write into the proper texture
void main() {
    mat4 light_space_matrix=mvp_matrix_per_cascade[light_space_matrix_index];
    gl_Position = light_space_matrix*model_matrix* vec4(aPos, 1.0);
}
                                                  