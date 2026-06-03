#ifndef VOXEL_MANIA_ASSETLOADER_H
#define VOXEL_MANIA_ASSETLOADER_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "vector_container.h"
#include "Vector3D.h"
#include "export.h"



struct TextureData {
    int width{};
    int height{};
    int nrChannels{};
    unsigned char *bytes{};
    char* path{};
    uint64_t path_hash{};
};

constexpr TextureData nil_texture{-1,-1,-1,nullptr, nullptr,0};

struct uv {
    float x;
    float y;
};

struct Vertice {
    Vector3D Position;
    Vector3D Normal;
    uv UVs;
};

struct ModelMesh {
    int diffuse_index;
    int specular_index;
    int emission_index;
    
    Vertice *vertices{};
    size_t vertex_count{};

    uint32_t* indices{};
    size_t index_count{};
};



VECTOR_DECLARATION_LIBRARY(ModelMesh)
VECTOR_DECLARATION_LIBRARY(TextureData)
struct ModelData {
    Vector_TextureData *diffuse_textures;
    Vector_TextureData *specular_textures;
    Vector_ModelMesh* mesh_data{};
};

// assuming the engine supports 3 textures types diffuse, 
// specular
// emission


EXPORTED TextureData LoadTexture(const char *path);
EXPORTED ModelData LoadModel(const char *path);

#endif //VOXEL_MANIA_ASSETLOADER_H
