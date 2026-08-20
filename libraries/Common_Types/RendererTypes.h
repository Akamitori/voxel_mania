//
// Created by PETROS on 20/08/2026.
//

#ifndef VOXEL_MANIA_RENDERERTYPES_H
#define VOXEL_MANIA_RENDERERTYPES_H

#include <cstdint>

#include "Vector3D.h"
#include "export.h"
#include "GL/glew.h"

struct EXPORTED Material {
    float shininess;
};

struct EXPORTED PointLight {
    alignas(16)Vector3D position;
    alignas(16)Vector3D ambient;
    alignas(16)Vector3D diffuse;
    alignas(16)Vector3D specular;
    float linear_factor;
    float quadric_factor;
};

struct EXPORTED DirectionalLight {
    alignas(16)Vector3D direction;
    alignas(16)Vector3D ambient;
    alignas(16)Vector3D diffuse;
    alignas(16)Vector3D specular;
};

struct EXPORTED SpotLight {
    alignas(16)Vector3D position;
    alignas(16)Vector3D direction;
    alignas(16)Vector3D ambient;
    alignas(16)Vector3D diffuse;
    alignas(16)Vector3D specular;
    float linear_factor;
    float quadric_factor;
    float inner_cutOff;
    float outer_cutOff;
};

enum class TextureWrapMode : uint32_t {
    REPEAT = GL_REPEAT,
    CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE
};

struct EXPORTED Texture_Parameters {
    TextureWrapMode wrap_mode_s = TextureWrapMode::REPEAT;
    TextureWrapMode wrap_mode_t = TextureWrapMode::REPEAT;
    bool convert_from_srgb_to_linear_space = false;
};

struct EXPORTED Transform {
    Vector3D Position{0, 0, 0};
    Vector3D Rotation{0, 0, 0};
    Vector3D Scale{1, 1, 1};
};

struct EXPORTED Vertex {
    Vector3D Position;
    Vector3D Normal;
    float UV_x;
    float UV_y;
};

constexpr int MAX_NUM_BONES_PER_VERTEX = 4;

struct EXPORTED VertexBoneData {
    uint32_t BoneIds[MAX_NUM_BONES_PER_VERTEX] = {};
    float Weights[MAX_NUM_BONES_PER_VERTEX] = {0.0f};
};


#endif //VOXEL_MANIA_RENDERERTYPES_H