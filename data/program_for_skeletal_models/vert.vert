#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in vec4 Weights;

layout (std140) uniform ViewMatrices{
    mat4 perspective_projection_matrix;// 16*4 bytes
    mat4 look_at_matrix;// 16*4 bytes
};

uniform mat4 model_matrix;
uniform mat3 model_matrix_for_normals;
uniform vec4[3] cascade_planes_object_space;
uniform mat4 m_0_shadow;

const int MAX_BONES = 200;
uniform mat4 gBones[MAX_BONES];

out VS_OUT{
    vec2 TexCoord;
    vec3 Normal;
    vec3 FragPosWorldSpace;
    vec3 u_products_for_interpolation;
    vec3 cascade_coord_0;
    flat ivec4 BoneIDs;
    vec4 Weights;
} vs_out;

void main() {

    mat4 BoneTransform=mat4(0.0f);
    
    BoneTransform += gBones[BoneIDs[0]] * Weights[0];
    BoneTransform += gBones[BoneIDs[1]] * Weights[1];
    BoneTransform += gBones[BoneIDs[2]] * Weights[2];
    BoneTransform += gBones[BoneIDs[3]] * Weights[3];
    
    vec4 PosL = BoneTransform * vec4(aPos, 1.0);
    
    gl_Position = perspective_projection_matrix*look_at_matrix*model_matrix * PosL;

    // this is important for light calculations
    vs_out.FragPosWorldSpace=vec3(model_matrix*PosL);
    vs_out.TexCoord = aTexCoord;

    // Normal = mat3(transpose(inverse(model_matrix))) * anormal;
    // we could use the inverse transpose blah blah but we just get the matrix from the CPU
    mat3 BoneNormalMatrix = mat3(transpose(inverse(BoneTransform)));
    vec3 SkinnedNormal = normalize(BoneNormalMatrix * aNormal);
    vs_out.Normal = normalize(model_matrix_for_normals * SkinnedNormal);

    float[3] u_products_for_interpolation;

    for (int i=0;i< 3;++i){
        vec4 plane=cascade_planes_object_space[i];
        u_products_for_interpolation[i]=dot(plane, PosL);
    }

    vs_out.u_products_for_interpolation.x=u_products_for_interpolation[0];
    vs_out.u_products_for_interpolation.y=u_products_for_interpolation[1];
    vs_out.u_products_for_interpolation.z=u_products_for_interpolation[2];

    vs_out.cascade_coord_0=  vec3(m_0_shadow * model_matrix * PosL);
    
    vs_out.BoneIDs= BoneIDs;
    vs_out.Weights= Weights;
}