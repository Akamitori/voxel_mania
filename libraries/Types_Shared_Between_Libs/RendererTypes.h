//
// Created by PETROS on 20/08/2026.
//

//

#ifndef VOXEL_MANIA_RENDERERTYPES_H
#define VOXEL_MANIA_RENDERERTYPES_H

#include <cstdint>

#include "Vector3D.h"

#include "GL/glew.h"
#include "Quaternion.h"

// THE IDE insists this header is not needed. THE IDE is wrong!
// data type for passing data from loader to renderer
#include "hashmap_common.h"
#include "Matrix4D.h"


struct Vector_Key {
    Vector3D Value;
    double time;
};

struct Quaternion_Key {
    Quaternion Value;
    double time;
};

struct Animation_Channel {
    const char *node_name;

    Vector_Key *    position_keys;
    size_t count_position_keys;

    Quaternion_Key *rotation_keys;
    size_t count_rotation_keys;

    Vector_Key *scaling_keys;
    size_t count_scaling_keys;
    
    size_t node_index;
};

struct Animation {
    const char *Name{};
    double Duration{};
    double Ticks_Per_Second{};
    Animation_Channel *Channels{};
    size_t Channel_count{};
};

struct Material {
    float shininess;
};

struct PointLight {
    alignas(16)Vector3D position;
    alignas(16)Vector3D ambient;
    alignas(16)Vector3D diffuse;
    alignas(16)Vector3D specular;
    float linear_factor;
    float quadric_factor;
};

struct DirectionalLight {
    alignas(16)Vector3D direction;
    alignas(16)Vector3D ambient;
    alignas(16)Vector3D diffuse;
    alignas(16)Vector3D specular;
};

struct SpotLight {
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

struct Texture_Parameters {
    TextureWrapMode wrap_mode_s = TextureWrapMode::REPEAT;
    TextureWrapMode wrap_mode_t = TextureWrapMode::REPEAT;
    bool convert_from_srgb_to_linear_space = false;
};

struct Transform {
    Vector3D Position{0, 0, 0};
    Vector3D Rotation{0, 0, 0};
    Vector3D Scale{1, 1, 1};
};

struct Vertex {
    Vector3D Position;
    Vector3D Normal;
    float UV_x;
    float UV_y;
};

constexpr int MAX_NUM_BONES_PER_VERTEX = 4;

struct VertexBoneData {
    uint32_t BoneIds[MAX_NUM_BONES_PER_VERTEX] = {};
    float Weights[MAX_NUM_BONES_PER_VERTEX] = {0.0f};
};

struct BoneInfo {
    Matrix4D OffsetMatrix{};
};


// common type includes. KEEP IT!


#endif //VOXEL_MANIA_RENDERERTYPES_H
