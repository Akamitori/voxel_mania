#ifndef VOXEL_MANIA_ASSETLOADER_H
#define VOXEL_MANIA_ASSETLOADER_H


#include <cstdint>

#include "Vector3D.h"
#include "export.h"
#include "Matrix4D.h"
#include "RendererTypes.h"



// the caller will own both the bytes and the texture name
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



struct ModelMesh {
    uint32_t diffuse_index{ static_cast<uint32_t>(-1)};
    uint32_t specular_index{static_cast<uint32_t>(-1)};
    uint32_t emission_index{static_cast<uint32_t>(-1)};
    
    Vertice *vertices{};
    size_t vertex_count{};

    uint32_t* indices{};
    size_t index_count{};
    
    Mesh_Type Type{};
};

constexpr size_t NO_BONE_NODE = UINT64_MAX;


// the node that's part of a node hierarchy
struct hierarchy_model_node {
    const char* Name{};                      
    // matrix at bind poise
    Matrix4D NodeTransformationMatrix{};
    size_t parentID{0};
    size_t bone_index{NO_BONE_NODE};
    
    // rotation bind pose
    Quaternion Rotation{};
    // scaling bind pose
    Vector3D Scaling{};
    // position bind pose
    Vector3D Position{};
};

const hierarchy_model_node NilNode{"nil_node", IdentityMatrix, 0, NO_BONE_NODE, {}, {}, {}};


struct ModelData {
    // this one doesn't need any count since we don't iterate them directly
    TextureData *diffuse_textures;
    TextureData *specular_textures;
    
    ModelMesh* mesh_data{};
    size_t mesh_data_count{};
    
    // this contains all the offsets for each of those mesh starts
    // the idea of mesh_base_vertex is that we use it to tightly pack each mesh and its data into
    // the data structure this is packed into is the BoneDataPerMeshPerVertice
    int* mesh_base_vertex{};
    size_t mesh_base_vertex_count{};
    
    // while initially this was about storing bones
    // the hierarchy exists anyways
    // it hasn't bitten us in the ass because we rendered static models but this is 
    // a much needed step to prevent that
    hierarchy_model_node* node_hierarchy{};
    size_t node_hierarchy_count{};
    
    // a flat storage of BoneData
    // if there's no data 
    // this contains all the bones for every mesh
    // for each vertice
    VertexBoneData *BoneDataPerMeshPerVertice{};
    size_t BoneDataPerMeshPerVertice_count{};
    
    // this is the same size as the bones yeah
    BoneInfo *BoneMatrices{};
    size_t bone_matrices_count{};
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
EXPORTED void asset_loader_free(void* ptr);

#endif //VOXEL_MANIA_ASSETLOADER_H

