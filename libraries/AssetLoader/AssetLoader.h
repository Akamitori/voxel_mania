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
    const char* texture_name{};
};

constexpr TextureData nil_texture{-1, -1, -1, nullptr, "nil_texture"};

struct uv {
    float x;
    float y;
};

struct Vertice {
    Vector3D Position;
    Vector3D Normal;
    uv UVs;
};

enum Mesh_Type {
    Regular,
    Skinned
};

struct BoneInfo {
    Matrix4D OffsetMatrix{};
};

struct ModelMesh {
    uint32_t diffuse_index;
    uint32_t specular_index;
    uint32_t emission_index;
    
    Vertice *vertices{};
    size_t vertex_count{};

    uint32_t* indices{};
    size_t index_count{};
    
    Mesh_Type Type{};
};




constexpr size_t NO_BONE_NODE = UINT64_MAX;


struct ModelNode {
    const char* Name{};
    Matrix4D NodeTransformationMatrix{};
    size_t parentID{0};
    size_t bone_index{NO_BONE_NODE};
    Quaternion Rotation{};
    Vector3D Scaling{};
    Vector3D Position{};
};

const ModelNode NilNode{"nil_node", IdentityMatrix, 0};



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
    // the data structure this is packed into is the BoneDataPerMeshPerVertice
    Vector_int* mesh_base_vertex{};
    
    // while initially this was about storing bones
    // the hierarchy exists anyways
    // it hasn't bitten us in the ass because we rendered static models but this is 
    // a much needed step to prevent that
    // TODO this is to be filled after i rewatch the videos
    Vector_ModelNode* NodeHierarchy{};
    
    // a flat storage of BoneData
    // if there's no data 
    // this contains all the bones for every mesh
    // for each vertice
    Vector_VertexBoneData *BoneDataPerMeshPerVertice{};
    
    // this is the same size as the bones yeah
    Vector_BoneInfo *BoneMatrices{};
    // this map provides indices for the bone matrices   . we can use this during animations to see if the nodes make sense(probably???)
    cstr_uint32_t_hashmap* Bone_name_to_index_map{};
    
    Matrix4D GlobalInverseTransform{};
    
    Animation* Animations{};
    cstr_uint32_t_hashmap* animation_name_to_index_map{};
    size_t count_animations{};
};

// assuming the engine supports 3 textures types diffuse, 
// specular
// emission


EXPORTED TextureData LoadTexture(const char *path);
EXPORTED ModelData LoadModel(const char *path);

#endif //VOXEL_MANIA_ASSETLOADER_H

