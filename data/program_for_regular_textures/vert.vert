#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (std140) uniform ViewMatrices{
    mat4 perspective_projection_matrix;// 16*4 bytes
    mat4 look_at_matrix;// 16*4 bytes
};

uniform mat4 model_matrix;
uniform mat3 model_matrix_for_normals;

uniform vec4[3] cascade_planes;

out VS_OUT{
    vec2 TexCoord;
    vec3 Normal;
    vec3 FragPosWorldSpace;
    float[3] u_products_for_interpolation;
} vs_out;

void main() {
    gl_Position = perspective_projection_matrix*look_at_matrix*model_matrix * vec4(aPos, 1.0);

    // this is important for light calculations
    vs_out.FragPosWorldSpace=vec3(model_matrix*vec4(aPos, 1.0));
    vs_out.TexCoord = aTexCoord;

    // Normal = mat3(transpose(inverse(model_matrix))) * anormal;
    // we could use the inverse transpose blah blah but we just get the matrix from the CPU
    vs_out.Normal= model_matrix_for_normals*aNormal;

    for (int i=0;i< 3;++i){
        const vec4 plane=cascade_planes[i];
        vs_out.u_products_for_interpolation[i]=plane * model_matrix * vec4(aPos, 1);
    }
}