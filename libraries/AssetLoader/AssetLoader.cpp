#include "AssetLoader.h"

#include <cassert>
#include "queue_container.h"


#define STB_IMAGE_IMPLEMENTATION


#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include <string>

#include "hashmap_container.h"
#include "Matrix4D.h"

struct bone_id_result {
    size_t bone_id{};
    bool new_bone{};
};

VECTOR_IMPLEMENTATION(int)


static bone_id_result get_bone_id(const aiBone *pBone, cstr_uint32_t_hashmap *bone_name_to_index_map);

static void AddBoneData(VertexBoneData *bone_data, uint32_t BoneId, float Weight);

static Matrix4D ConvertAssimptMatrix4dToMatrix4D(const aiMatrix4x4 &m);

static void GenerateTextureStringName(
    const aiScene *scene,
    aiTextureType type,
    const aiMaterial *material,
    const char *model_name,
    const char *model_dir,
    char *result
);

static TextureData LoadTextureFromMaterial(const aiScene *scene, aiTextureType type, const aiMaterial *material, const char *texture_name);

static const char *copy_string(const char *s);

VECTOR_IMPLEMENTATION(ModelMesh)
VECTOR_IMPLEMENTATION(TextureData)
VECTOR_IMPLEMENTATION(VertexBoneData)
VECTOR_IMPLEMENTATION(BoneInfo)
VECTOR_IMPLEMENTATION(ModelNode)

TextureData LoadTexture(const char *path) {
    int width;
    int height;
    int nrChannels;

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    //assert(("Texture path", data));

    if (!data) {
        fprintf(stdout, "Warning : didn't load any data for %s\n", path);
        return nil_texture;
    }

    if (nrChannels != 3 && nrChannels != 4) {
        fprintf(stderr, "Warning : channel error for  %s\n", path);
        exit(1);
    }

    assert(("Support only for RGBA", nrChannels == 3 || nrChannels == 4));

    char *path_to_load = (char *) malloc((strlen(path) + 1) * sizeof(char));
    strcpy(path_to_load, path);

    return TextureData{
        width,
        height,
        nrChannels,
        data,
        path_to_load
    };
}


void print_matrix(aiMatrix4x4 &matrix);

void print_matrix(aiMatrix4x4 &matrix) {
    printf("-------Matrixxxxxx start-----\n");
    for (int i = 0; i < 4; i++) {
        float *number_pt4 = matrix[i];
        printf("[%f,%f,%f,%f]\n",
               number_pt4[0],
               number_pt4[1],
               number_pt4[2],
               number_pt4[3]
        );
    }
    printf("---------Matrixxxxxx end--------\n");
}

#include "stack_container.h"
typedef const aiNode *const_aiNodePtr;


typedef struct {
    const_aiNodePtr node;
    size_t parent_id;
} node_stack_entry;

STACK_IMPLEMENTATION_STATIC(node_stack_entry)

ModelData LoadModel(const char *path) {
    Assimp::Importer importer;
    //TODO we should probably unify the way we load our models for obvious reasons
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        //aiProcess_MakeLeftHanded |
        aiProcess_JoinIdenticalVertices
        | aiProcess_GenNormals

    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        fprintf(stderr, "ERROR::ASSIMP:: %s", importer.GetErrorString());
        return {};
    }


    const char *last_slash = strrchr(path, '/');
    const size_t chars_to_copy = last_slash ? last_slash - path : strlen(path);
    char *directory = (char *) malloc((chars_to_copy + 1) * sizeof(char));
    memcpy(directory, path, chars_to_copy * sizeof(char));
    directory[chars_to_copy] = '\0';

    ModelData model{};

    model.mesh_data = Vector_ModelMesh_Create(100);
    model.diffuse_textures = Vector_TextureData_Create(scene->mNumMaterials);
    model.specular_textures = Vector_TextureData_Create(scene->mNumMaterials);
    model.mesh_base_vertex = Vector_int_Create(scene->mNumMeshes);


    Vector_TextureData_Add(model.diffuse_textures, nil_texture);
    Vector_TextureData_Add(model.specular_textures, nil_texture);
    // start from 1 since the nill texture takes slot 0
    uint32_t current_diffuse_index = 1;
    uint32_t current_specular_index = 1;

    cstr_uint32_t_hashmap *specular_keys = cstr_uint32_t_Hashmap_Create(1024, 0.9);
    cstr_uint32_t_hashmap *diffuse_keys = cstr_uint32_t_Hashmap_Create(1024, 0.9);
    model.Bone_name_to_index_map = cstr_uint32_t_Hashmap_Create(1024, 0.9);


    int vertice_count = 0;
    int bone_number = 0;
    Mesh_Type type_to_parse = Regular;


    // TODO we can def count both the vertices and the indices here per mesh to make allocations below a little less mesy
    for (int mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index) {
        const aiMesh *mesh = scene->mMeshes[mesh_index];
        Vector_int_Add(model.mesh_base_vertex, vertice_count);
        vertice_count += mesh->mNumVertices;

        if (mesh->HasBones()) {
            type_to_parse = Skinned;
            bone_number += mesh->mNumBones;
        }
    }

    if (type_to_parse == Skinned) {
        model.BoneDataPerMeshPerVertice = Vector_VertexBoneData_Create(vertice_count);
        model.BoneMatrices = Vector_BoneInfo_Create(bone_number);
    } else {
        model.BoneDataPerMeshPerVertice = nullptr;
        model.BoneMatrices = nullptr;
    }

    char result_texture_name[1024] = {};
    // parse all the meses
    for (int mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index) {
        const aiMesh *mesh = scene->mMeshes[mesh_index];
        ModelMesh mesh_data{};
        // parse positions and normals
        {
            const unsigned int total_vertices = mesh->mNumVertices;

            assert(total_vertices>0 && "a node with no vertices makes no sense");

            mesh_data.vertex_count = total_vertices;
            mesh_data.vertices = (Vertice *) calloc(mesh->mNumVertices, sizeof(Vertice));

            assert(mesh_data.vertices && "Should get memory for vertices");
            for (unsigned int j = 0; j < total_vertices; ++j) {
                Vertice vertex{};

                vertex.Position = {
                    mesh->mVertices[j].x,
                    mesh->mVertices[j].y,
                    mesh->mVertices[j].z,
                };

                vertex.Normal = {
                    mesh->mNormals[j].x,
                    mesh->mNormals[j].y,
                    mesh->mNormals[j].z,
                };

                vertex.UVs = {0, 0};
                if (mesh->mTextureCoords[0]) {
                    vertex.UVs.x = mesh->mTextureCoords[0][j].x;
                    vertex.UVs.y = mesh->mTextureCoords[0][j].y;
                }

                mesh_data.vertices[j] = vertex;
            }
        }

        // parse the indices
        // TODO we can probably do better counting instead of relying on the total of vertices
        {
            mesh_data.index_count = 1024;
            mesh_data.indices = (uint32_t *) calloc(1024, sizeof(uint32_t));

            assert(mesh_data.indices && "Should get memory for indices");
            size_t indice_total = 0;

            // this will probably overallocate in some cases
            // the alternative is to do a double pass
            // one for getting the actual count
            // and one for allocating
            // i will argue that memory is kinda free
            for (size_t face_index = 0; face_index < mesh->mNumFaces; ++face_index) {
                const aiFace face = mesh->mFaces[face_index];
                for (unsigned int face_indices_index = 0; face_indices_index < face.mNumIndices; ++face_indices_index) {
                    if (indice_total >= mesh_data.index_count) {
                        const size_t new_count = mesh_data.index_count * 2;
                        uint32_t *new_memory = (uint32_t *) realloc(mesh_data.indices, sizeof(uint32_t) * new_count);

                        assert(new_memory && "Should get memory for more indices");
                        mesh_data.indices = new_memory;
                        mesh_data.index_count = new_count;
                    }

                    mesh_data.indices[indice_total++] = face.mIndices[face_indices_index];
                }
            }

            mesh_data.index_count = indice_total;
        }

        // parse the materials
        {
            const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];


            GenerateTextureStringName(scene, aiTextureType_DIFFUSE, material, path, directory, result_texture_name);
            uint32_t diffuse_index = 0;
            if (strcmp(result_texture_name, nil_texture.texture_name) != 0) {
                if (uint32_t *find_result = cstr_uint32_t_Hashmap_Find(diffuse_keys, result_texture_name)) {
                    diffuse_index = *find_result;
                } else {
                    TextureData diffuse = LoadTextureFromMaterial(scene, aiTextureType_DIFFUSE, material, result_texture_name);
                    if (diffuse.nrChannels != nil_texture.nrChannels) {
                        Vector_TextureData_Add(model.diffuse_textures, diffuse);
                        // we can fix this api a bit by changing it
                        cstr_uint32_t_Hashmap_Insert(diffuse_keys, {diffuse.texture_name, current_diffuse_index});
                        diffuse_index = current_diffuse_index;
                        ++current_diffuse_index;
                    }
                }
            }

            uint32_t specular_index = 0;
            GenerateTextureStringName(scene, aiTextureType_SPECULAR, material, path, directory, result_texture_name);
            if (strcmp(result_texture_name, nil_texture.texture_name) != 0) {
                if (uint32_t *find_result = cstr_uint32_t_Hashmap_Find(specular_keys, result_texture_name)) {
                    specular_index = *find_result;
                } else {
                    TextureData specular = LoadTextureFromMaterial(scene, aiTextureType_SPECULAR, material, result_texture_name);
                    if (specular.nrChannels != nil_texture.nrChannels) {
                        Vector_TextureData_Add(model.specular_textures, specular);
                        // we can fix this api a bit by changing it
                        cstr_uint32_t_Hashmap_Insert(specular_keys, {specular.texture_name, current_specular_index});
                        specular_index = current_specular_index;
                        ++current_specular_index;
                    }
                }
            }

            mesh_data.diffuse_index = diffuse_index;
            mesh_data.specular_index = specular_index;
        }

        // parse the bones if applicable
        {
            mesh_data.Type = Regular;
            if (mesh->HasBones()) {
                mesh_data.Type = Skinned;

                for (int bone_index = 0; bone_index < mesh->mNumBones; ++bone_index) {
                    aiBone *bone = mesh->mBones[bone_index];
                    bone_id_result id_result = get_bone_id(bone, model.Bone_name_to_index_map);

                    if (id_result.new_bone) {
                        const aiMatrix4x4 &offset_matrix_assimp = bone->mOffsetMatrix;
                        aiVector3D scaling_assimp;
                        aiVector3D position_assimp;
                        aiQuaternion rotation_assimp;
                        offset_matrix_assimp.Decompose(scaling_assimp, rotation_assimp, position_assimp);

                        Matrix4D offsetMatrix = ConvertAssimptMatrix4dToMatrix4D(offset_matrix_assimp);

                        Vector_BoneInfo_Add(model.BoneMatrices, {offsetMatrix});
                    }

                    //print_matrix(bone->mOffsetMatrix);
                    printf("\n");
                    for (int bone_weight_index = 0; bone_weight_index < bone->mNumWeights; ++bone_weight_index) {
                        // since we store the data per mesh we dont need a global vertex for obvious reasons
                        const aiVertexWeight &vw = bone->mWeights[bone_weight_index];

                        uint32_t global_vertex_id = model.mesh_base_vertex->data[mesh_index] + vw.mVertexId;
                        //printf("    %d : vertex id %d weight %.2f\n", bone_weight_index, vw.mVertexId, vw.mWeight);

                        assert(global_vertex_id< Vector_VertexBoneData_Capacity(model.BoneDataPerMeshPerVertice) && "We went over the limit bro");
                        AddBoneData(&model.BoneDataPerMeshPerVertice->data[global_vertex_id], id_result.bone_id, vw.mWeight);
                    }
                    printf("\n");
                }
            }
        }
        Vector_ModelMesh_Add(model.mesh_data, mesh_data);
    }

    model.BoneDataPerMeshPerVertice->end = model.BoneDataPerMeshPerVertice->data + vertice_count;


    // we can use all those 3 to get what we need
    // we can probably go with a stack ;)
    const aiNode *assimpt_root_node = scene->mRootNode;

    model.NodeHierarchy = Vector_ModelNode_Create(1024);
    Stack_node_stack_entry *nodePtr_stack = Stack_node_stack_entry_Create(1024);


    Vector_ModelNode_Add(model.NodeHierarchy, NilNode);

    node_stack_entry start = {assimpt_root_node, 0};

    Stack_node_stack_entry_Push(nodePtr_stack, start);

    cstr_size_t_hashmap *name_to_index_for_nodes = cstr_size_t_Hashmap_Create(1024, 0.9);

    node_stack_entry value;
    while (Stack_node_stack_entry_Pop(nodePtr_stack, &value)) {
        ModelNode node_to_add;
        const_aiNodePtr assimp_node = value.node;
        node_to_add.Name = copy_string(assimp_node->mName.C_Str());
        node_to_add.NodeTransformationMatrix = ConvertAssimptMatrix4dToMatrix4D(assimp_node->mTransformation);
        node_to_add.parentID = value.parent_id;

        const aiMatrix4x4 &offset_matrix_assimp = assimp_node->mTransformation;
        aiVector3D scaling_assimp;
        aiVector3D position_assimp;
        aiQuaternion rotation_assimp;
        offset_matrix_assimp.Decompose(scaling_assimp, rotation_assimp, position_assimp);

        node_to_add.Position = {position_assimp.x, position_assimp.y, position_assimp.z};
        node_to_add.Scaling = {scaling_assimp.x, scaling_assimp.y, scaling_assimp.z};
        node_to_add.Rotation = {rotation_assimp.x, rotation_assimp.y, rotation_assimp.z, rotation_assimp.w};

        uint32_t *bone_index = cstr_uint32_t_Hashmap_Find(model.Bone_name_to_index_map, node_to_add.Name);

        node_to_add.bone_index = bone_index ? *bone_index : NO_BONE_NODE;

        size_t parent_index = Vector_ModelNode_Length(model.NodeHierarchy);
        Vector_ModelNode_Add(model.NodeHierarchy, node_to_add);
        cstr_size_t_Hashmap_Insert(name_to_index_for_nodes, {node_to_add.Name, parent_index});

        for (size_t i = assimp_node->mNumChildren; i > 0; --i) {
            const node_stack_entry entry_to_push = {assimp_node->mChildren[i - 1], parent_index};
            Stack_node_stack_entry_Push(nodePtr_stack, entry_to_push);
        }
    }


    model.count_animations = 0;
    if (scene->HasAnimations()) {
        model.count_animations = scene->mNumAnimations;
        model.Animations = (Animation *) calloc(model.count_animations, sizeof(Animation));
        model.animation_name_to_index_map = cstr_uint32_t_Hashmap_Create(scene->mNumAnimations * 5, 0.9);

        for (uint32_t anim_i = 0; anim_i < scene->mNumAnimations; ++anim_i) {
            const aiAnimation *ai_anim_data = scene->mAnimations[anim_i];
            Animation *const animation = &model.Animations[anim_i];

            animation->Name = copy_string(ai_anim_data->mName.C_Str());

            animation->Duration = ai_anim_data->mDuration;
            animation->Ticks_Per_Second = ai_anim_data->mTicksPerSecond != 0 ? ai_anim_data->mTicksPerSecond : 25.0;
            animation->Channel_count = ai_anim_data->mNumChannels;
            animation->Channels = (Animation_Channel *) calloc(animation->Channel_count, sizeof(Animation_Channel));

            // no need to own the animation name data
            // this lives together and dies together
            cstr_uint32_t_Hashmap_Insert(model.animation_name_to_index_map, {animation->Name, anim_i});

            for (uint32_t chan_i = 0; chan_i < ai_anim_data->mNumChannels; ++chan_i) {
                const aiNodeAnim *const ai_anim_channel = ai_anim_data->mChannels[chan_i];

                Animation_Channel *const anim_channel = &animation->Channels[chan_i];
                anim_channel->node_name = copy_string(ai_anim_channel->mNodeName.C_Str());

                size_t *node_index = cstr_size_t_Hashmap_Find(name_to_index_for_nodes, anim_channel->node_name);

                if (!node_index) {
                    fprintf(stderr, "channel %s matches no node, skipping\n", anim_channel->node_name);
                    anim_channel->node_index = 0; // NilNode, harmless
                    continue;
                }

                anim_channel->node_index = *node_index;

                const aiVectorKey *const ai_pos_keys = ai_anim_channel->mPositionKeys;
                anim_channel->count_position_keys = ai_anim_channel->mNumPositionKeys;
                if (anim_channel->count_position_keys > 0) {
                    anim_channel->position_keys = (Vector_Key *) calloc(anim_channel->count_position_keys, sizeof(Vector_Key));
                    for (uint32_t p_key_i = 0; p_key_i < anim_channel->count_position_keys; ++p_key_i) {
                        const aiVectorKey &ai_pos_key = ai_pos_keys[p_key_i];
                        anim_channel->position_keys[p_key_i] = {
                            {ai_pos_key.mValue.x, ai_pos_key.mValue.y, ai_pos_key.mValue.z},
                            ai_pos_key.mTime,
                        };
                    }
                }

                const aiQuatKey *const ai_quat_keys = ai_anim_channel->mRotationKeys;
                anim_channel->count_rotation_keys = ai_anim_channel->mNumRotationKeys;
                if (anim_channel->count_rotation_keys > 0) {
                    anim_channel->rotation_keys = (Quaternion_Key *) calloc(anim_channel->count_rotation_keys, sizeof(Quaternion_Key));
                    for (uint32_t p_key_i = 0; p_key_i < anim_channel->count_rotation_keys; ++p_key_i) {
                        const aiQuatKey &ai_quat_key = ai_quat_keys[p_key_i];
                        anim_channel->rotation_keys[p_key_i] = {
                            {ai_quat_key.mValue.x, ai_quat_key.mValue.y, ai_quat_key.mValue.z, ai_quat_key.mValue.w},
                            ai_quat_key.mTime,
                        };
                    }
                }

                const aiVectorKey *const ai_scale_keys = ai_anim_channel->mScalingKeys;
                anim_channel->count_scaling_keys = ai_anim_channel->mNumScalingKeys;
                if (anim_channel->count_scaling_keys > 0) {
                    anim_channel->scaling_keys = (Vector_Key *) calloc(anim_channel->count_scaling_keys, sizeof(Vector_Key));
                    for (uint32_t p_key_i = 0; p_key_i < anim_channel->count_scaling_keys; ++p_key_i) {
                        const aiVectorKey &ai_scale_key = ai_scale_keys[p_key_i];
                        anim_channel->scaling_keys[p_key_i] = {
                            {ai_scale_key.mValue.x, ai_scale_key.mValue.y, ai_scale_key.mValue.z},
                            ai_scale_key.mTime,
                        };
                    }
                }
            }
        }
    }
        
    cstr_uint32_t_Hashmap_Free(specular_keys);
    cstr_uint32_t_Hashmap_Free(diffuse_keys);
    cstr_size_t_Hashmap_Free(name_to_index_for_nodes);
    Stack_node_stack_entry_Free(nodePtr_stack);
    free(directory);

    model.GlobalInverseTransform = inverse(ConvertAssimptMatrix4dToMatrix4D(assimpt_root_node->mTransformation));

    return model;
}


std::vector<VertexBoneData> vertex_to_bones;

// this contains all the offsets for each of those mesh starts
// the idea of mesh_base_vertex is that we use it to tightly pack each mesh and its data into
// i would argue we dont need it since we can calculate the number in a loop 
// but it may be needed in future tutorial
//std::vector<int> mesh_base_vertex;

// finally, for id generation we use an std::map so we can either append a bone and create an id for it OR to get an existing id
// so far this is so we can keep tags on individual bones internally
// no usage(SO FAR)
// TODO assuming this loads a bunch of models, then we need to clean it up... DUH
//std::map<std::string, uint32_t>
//cstr_uint32_t_hashmap bone_name_to_index_map=cstr_uint32_t_hashmap;


bone_id_result get_bone_id(const aiBone *pBone, cstr_uint32_t_hashmap *bone_name_to_index_map) {
    bone_id_result result{};

    if (uint32_t *found = cstr_uint32_t_Hashmap_Find(bone_name_to_index_map, pBone->mName.C_Str())) {
        result.bone_id = *found;
        return result;
    }

    result.bone_id = bone_name_to_index_map->num_elements;
    result.new_bone = true;
    cstr_uint32_t_Hashmap_Insert(bone_name_to_index_map,
                                 {copy_string(pBone->mName.C_Str()), static_cast<uint32_t>(result.bone_id)}
    );
    return result;
}

void AddBoneData(VertexBoneData *bone_data, uint32_t BoneId, float Weight) {
    for (uint32_t i = 0; i < MAX_NUM_BONES_PER_VERTEX; ++i) {
        // fill in the first available weight
        if (bone_data->Weights[i] == 0) {
            bone_data->BoneIds[i] = BoneId;
            bone_data->Weights[i] = Weight;
            //printf("bone %d weight %f index %d\n", BoneId, Weight, i);
            return;
        }
    }

    assert(0 && "should never get here for our max limits");
}


Matrix4D ConvertAssimptMatrix4dToMatrix4D(const aiMatrix4x4 &m) {
    return Matrix4D{
        m.a1, m.a2, m.a3, m.a4,
        m.b1, m.b2, m.b3, m.b4,
        m.c1, m.c2, m.c3, m.c4,
        m.d1, m.d2, m.d3, m.d4,
    };
}

static void GenerateTextureStringName(
    const aiScene *scene,
    const aiTextureType type,
    const aiMaterial *material,
    const char *model_name,
    const char *model_dir,
    char *result
) {
    const unsigned int texture_count_for_type = material->GetTextureCount(type);

    if (texture_count_for_type == 0) {
        strcpy(result, nil_texture.texture_name);
        return;
    }

    aiString str;
    material->GetTexture(type, 0, &str);
    const char *path = str.C_Str();

    const aiTexture *tex = scene->GetEmbeddedTexture(path);

    result[0] = '\0';
    if (tex) {
        strcat(result, model_name);
        strcat(result, path);
    } else {
        const char *file_name = str.C_Str();
        const size_t dir_name_len = strlen(model_dir);

        if (dir_name_len == 0) {
            strcat(result, file_name);
        } else {
            strcpy(result, model_dir);
            strcat(result, "/");
            strcat(result, file_name);
        }
    }
}

static const char *copy_string(const char *s) {
    const size_t len = strlen(s) + 1;
    char *copy = (char *) malloc(len);
    memcpy(copy, s, len);
    return copy;
}

static TextureData LoadTextureFromMaterial(
    const aiScene *scene,
    const aiTextureType type,
    const aiMaterial *material,
    const char *texture_name) {
    assert(("Currently we only support one texture per type", material->GetTextureCount(type) <= 1));


    stbi_set_flip_vertically_on_load(true);
    if (strcmp(texture_name, nil_texture.texture_name) == 0) {
        return nil_texture;
    }

    aiString str;
    if (material->GetTexture(type, 0, &str) != AI_SUCCESS) {
        return nil_texture;
    }

    const aiTexture *tex = scene->GetEmbeddedTexture(str.C_Str());
    if (!tex) {
        // regular file on disk: the name already is the full relative path
        return LoadTexture(texture_name);
    }

    assert(("Unsupported: raw ARGB8888 embedded texture", tex->mHeight == 0));

    int width, height, nrChannels;
    unsigned char *data = stbi_load_from_memory(
        (const unsigned char *) tex->pcData,
        tex->mWidth,
        &width, &height, &nrChannels, 0
    );

    if (!data) {
        fprintf(stderr, "Warning : failed to decode embedded texture %s\n", texture_name);
        return nil_texture;
    }
    assert(("Support only for RGB/RGBA", nrChannels == 3 || nrChannels == 4));

    return TextureData{width, height, nrChannels, data, copy_string(texture_name)};
}

// we need to upload the bone vertex buffer
// our data for that is as follows (or can be as follows, this is our data after all
// positions
// normals
// textcoords
// indices
// ALL the vertex bone dat itself

// the new data the shader needs( assuming it's for animation
// are bone_id_location
// bone_weight_location                               we use those so each vertex can access its list of bones and their weights
