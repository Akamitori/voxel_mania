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
};

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
    TextureData diffuse;
    TextureData specular;
    TextureData emission;

    Vertice *vertices{};
    size_t vertex_count{};

    uint32_t* indices{};
    size_t index_count{};
};



VECTOR_DECLARATION_LIBRARY(ModelMesh)
struct ModelData {
    Vector_ModelMesh* mesh_data{};
};

// assuming the engine supports 3 textures types diffuse, 
// specular
// emission


EXPORTED TextureData LoadTextureNew(const char *path);
EXPORTED ModelData LoadModel(const char *path);

#endif //VOXEL_MANIA_ASSETLOADER_H
