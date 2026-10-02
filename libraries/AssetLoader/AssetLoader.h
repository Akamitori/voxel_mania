#ifndef VOXEL_MANIA_ASSETLOADER_H
#define VOXEL_MANIA_ASSETLOADER_H


#include <cstdint>

#include "vector_container.h"
#include "Vector3D.h"
#include "export.h"
#include "Matrix4D.h"
#include "RendererTypes.h"


struct TextureData {
    int width{};
    int height{};
    int nrChannels{};
    unsigned char *bytes{};
    char* path{};
};

constexpr TextureData nil_texture{-1,-1,-1,nullptr, nullptr};

struct uv {
    float x;
    float y;
};

struct Vertice {
    Vector3D Position;
    Vector3D Normal;
    uv UVs;
};
// A FAT STRUCT
// USE WHATEVER YOU NEED


enum Mesh_Type {
    Regular,
    Skinned
};

struct BoneInfo {
    Matrix4D OffsetMatrix{};
    Matrix4D FInalTransformMatrix{};
};

struct ModelMesh {
    uint32_t diffuse_index;
    uint32_t specular_index;
    uint32_t emission_index;
    
    Vertice *vertices{};
    size_t vertex_count{};

    uint32_t* indices{};
    size_t index_count{};
    
    size_t bone_count{};
    
    Mesh_Type Type{};
    
    
};


struct ModelNode {
    char* Name{};
    Matrix4D NodeTransformationMatrix{};
    int parentID{-1};
};



VECTOR_DECLARATION_LIBRARY(ModelMesh)
VECTOR_DECLARATION_LIBRARY(TextureData)
VECTOR_DECLARATION_LIBRARY(VertexBoneData)
VECTOR_DECLARATION_LIBRARY(BoneInfo)
VECTOR_DECLARATION_LIBRARY(ModelNode)
VECTOR_DECLARATION_LIBRARY(int)


struct ModelData {
    Vector_TextureData *diffuse_textures;
    Vector_TextureData *specular_textures;
    Vector_ModelMesh* mesh_data{};
    
    // this contains all the offsets for each of those mesh starts
    // the idea of mesh_base_vertex is that we use it to tightly pack each mesh and its data into
    Vector_int* mesh_base_vertex{};
    // while initially this was about storing bones
    // the hierarchy exists anyways
    // it hasn't bitten us in the us because we rendered static models but this is 
    // a much needed step to prevent that
    Vector_ModelNode* NodeHierarchy{};
    
    // a flat storage of BoneData
    // if there's no data 
    // this contains all the bones for every mesh
    // for each vertice
    Vector_VertexBoneData *BoneDataPerMeshPerVertice{};
    Vector_BoneInfo *BoneMatrices{};
    
    
    // for each mesh we have a hierarhcy
    // we have 10 meshes(example)
    
    
};

// assuming the engine supports 3 textures types diffuse, 
// specular
// emission


EXPORTED TextureData LoadTexture(const char *path);
EXPORTED ModelData LoadModel(const char *path);
EXPORTED void parse_scene(const char * path);

#endif //VOXEL_MANIA_ASSETLOADER_H
