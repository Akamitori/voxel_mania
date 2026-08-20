#include "AssetLoader.h"

#include <cassert>
#include "queue_container.h"


#define STB_IMAGE_IMPLEMENTATION


#include <unordered_map>

#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include <map>
#include <string>


size_t get_bone_id(const aiBone *pBone);

uint64_t fnv1a(const char *s) {
    const size_t len = strlen(s);
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t) s[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}


typedef aiNode *aiNodePtr;
QUEUE_IMPLEMENTATION_STATIC(aiNodePtr)
VECTOR_IMPLEMENTATION(ModelMesh)
VECTOR_IMPLEMENTATION(TextureData)

static TextureData LoadTextureFromMaterial(const aiScene *scene, const aiTextureType type, const char *directory, const aiMaterial *material) {
    aiString str;
    const unsigned int texture_count_for_type = material->GetTextureCount(type);
    assert(("Currently we only support one texture per type", texture_count_for_type<=1));

    if (texture_count_for_type == 0) {
        return nil_texture;
    }

    material->GetTexture(type, 0, &str);
    const char *path = str.C_Str();

    // deal with embedded textures
    if (path[0] == '*') {
        // embedded texture
        int index = atoi(path + 1);
        const aiTexture *tex = scene->mTextures[index];

        assert(("Unsupported: raw ARGB8888 embedded texture", tex->mHeight == 0));

        int width, height, nrChannels;
        unsigned char *data = stbi_load_from_memory(
            (unsigned char *) tex->pcData,
            tex->mWidth,
            &width, &height, &nrChannels, 0
        );

        assert(("Failed to load embedded texture", data));
        assert(("Support only for RGB/RGBA", nrChannels == 3 || nrChannels == 4));

        char *path_copy = (char *) malloc((strlen(path) + 1) * sizeof(char));
        strcpy(path_copy, path);

        return TextureData{width, height, nrChannels, data, path_copy};
    }

    const char *file_name = str.C_Str();
    const size_t dir_name_len = strlen(directory);
    const size_t file_name_len = strlen(file_name);

    char *relative_path = (char *) malloc((dir_name_len + file_name_len + 2) * sizeof(char));
    if (dir_name_len == 0) {
        strcpy(relative_path, file_name);
    } else {
        strcpy(relative_path, directory);
        strcat(relative_path, "/");
        strcat(relative_path, file_name);
    }

    const TextureData t = LoadTexture(relative_path);
    free(relative_path);

    return t;
}

TextureData LoadTexture(const char *path) {
    int width;
    int height;
    int nrChannels;

    // let's assume for now that we are using rgb
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    //assert(("Texture path", data));

    if (!data) {
        fprintf(stdout, "Warning : didn't load any data for %s\n", path);
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
        path_to_load,
        fnv1a(path_to_load)
    };
}

void AddBoneData(VertexBoneData *bone_data, uint32_t BoneId, float Weight) {
    for (uint32_t i = 0; i < MAX_NUM_BONES_PER_VERTEX; ++i) {
        // fill in the first available weight
        if (bone_data->Weights[i] == 0) {
            bone_data->BoneIds[i] = BoneId;
            bone_data->Weights[i] = Weight;
            printf("bone %d weight %f index %d\n", BoneId, Weight, i);
            return;
        }
    }

    assert(0 && "should never get here for our max limits");
}

ModelData LoadModel(const char *path) {
    Assimp::Importer importer;
    //TODO we should probably unify the way we load our models for obvious reasons
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        //aiProcess_FlipUVs |
        //aiProcess_MakeLeftHanded |
        aiProcess_JoinIdenticalVertices
        |
        aiProcess_GenNormals

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

    Queue_aiNodePtr *nodes_to_process = Queue_aiNodePtr_Create(100);
    Queue_aiNodePtr_Enqueue(nodes_to_process, scene->mRootNode);

    aiNodePtr node;
    ModelData model{};


    model.mesh_data = Vector_ModelMesh_Create(100);
    model.diffuse_textures = Vector_TextureData_Create(scene->mNumMaterials);
    model.specular_textures = Vector_TextureData_Create(scene->mNumMaterials);

    Vector_TextureData_Add(model.diffuse_textures, nil_texture);
    Vector_TextureData_Add(model.specular_textures, nil_texture);

    int current_diffuse_index = 1;
    int current_specular_index = 1;

    std::unordered_map<size_t, int> specular_keys{};
    std::unordered_map<size_t, int> diffuse_keys{};

    while (Queue_aiNodePtr_Deque(nodes_to_process, &node)) {
        for (unsigned int mesh_index = 0; mesh_index < node->mNumMeshes; ++mesh_index) {
            const aiMesh *mesh = scene->mMeshes[node->mMeshes[mesh_index]];
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

                TextureData diffuse = //nil_texture; 
                        LoadTextureFromMaterial(scene, aiTextureType_DIFFUSE, directory, material);
                TextureData specular = //nil_texture; 
                        LoadTextureFromMaterial(scene, aiTextureType_SPECULAR, directory, material);


                int diffuse_index = 0;
                if (diffuse.nrChannels != nil_texture.nrChannels) {
                    if (auto search = diffuse_keys.find(diffuse.path_hash); search != diffuse_keys.end()) {
                        // just get the id
                        // and call it a day
                        diffuse_index = search->second;
                    } else {
                        Vector_TextureData_Add(model.diffuse_textures, diffuse);
                        diffuse_keys[diffuse.path_hash] = current_diffuse_index;
                        diffuse_index = current_diffuse_index;
                        ++current_diffuse_index;
                    }
                }
                mesh_data.diffuse_index = diffuse_index;

                int specular_index = 0;
                if (specular.nrChannels != nil_texture.nrChannels) {
                    if (auto search = specular_keys.find(specular.path_hash); search != specular_keys.end()) {
                        // just get the id
                        // and call it a day
                        specular_index = search->second;
                    } else {
                        Vector_TextureData_Add(model.specular_textures, specular);
                        specular_keys[specular.path_hash] = current_specular_index;
                        specular_index = current_specular_index;
                        ++current_specular_index;
                    }
                }
                mesh_data.specular_index = specular_index;
            }

            // parse the bones if applicable
            {
                mesh_data.Type = Regular;
                mesh_data.bone_data_per_vertice_count = 0;
                if (mesh->HasBones()) {
                    mesh_data.Type = Skinned;
                    mesh_data.BoneDataPerVertice = (VertexBoneData *) calloc(mesh->mNumVertices, sizeof(VertexBoneData));
                    mesh_data.bone_data_per_vertice_count = mesh->mNumVertices;
                    assert(mesh_data.BoneDataPerVertice && "we either alloc or we dont DUH");
                    for (int bone_index = 0; bone_index < mesh->mNumBones; ++bone_index) {
                        aiBone *bone = mesh->mBones[bone_index];
                        size_t bone_id = get_bone_id(bone);

                        printf("\n");
                        for (int bone_weight_index = 0; bone_weight_index < bone->mNumWeights; ++bone_weight_index) {
                            const aiVertexWeight &vw = bone->mWeights[bone_weight_index];
                            printf("    %d : vertex id %d weight %.2f\n", bone_weight_index, vw.mVertexId, vw.mWeight);

                            AddBoneData(&mesh_data.BoneDataPerVertice[vw.mVertexId],bone_id, vw.mWeight);
                        }
                        printf("\n");
                    }
                }
            }
            Vector_ModelMesh_Add(model.mesh_data, mesh_data);
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            Queue_aiNodePtr_Enqueue(nodes_to_process, node->mChildren[i]);
        }
    }

    Queue_aiNodePtr_Free(nodes_to_process);
    free(directory);
    return model;
}


// this contains all the bones for every mesh
// for each vertice 
std::vector<VertexBoneData> vertex_to_bones;

// this contains all the offsets for each of those mesh starts
// the idea of mesh_base_vertex is that we use it to tightly pack each mesh and its data into
// i would argue we dont need it since we can calculate the number in a loop 
// but it may be needed in future tutorial
std::vector<int> mesh_base_vertex;

// finally, for id generation we use an std::map so we can either append a bone and create an id for it OR to get an existing id
// so far this is so we can keep tags on individual bones internally
// no usage(SO FAR)
// TODO assuming this loads a bunch of models, then we need to clean it up... DUH
std::map<std::string, uint32_t> bone_name_to_index_map;

size_t get_bone_id(const aiBone *pBone) {
    size_t bone_id = 0;

    std::string bone_name(pBone->mName.C_Str());

    if (const auto result = bone_name_to_index_map.find(bone_name); result == bone_name_to_index_map.end()) {
        bone_id = bone_name_to_index_map.size();
        bone_name_to_index_map[bone_name] = bone_id;
    } else {
        bone_id = result->second;
    }

    return bone_id;
}


void parse_single_bone(int mesh_index, aiBone *pBone) {
    printf("Bone %d : '%s', num vertices affected by this bones %u\n", mesh_index, pBone->mName.C_Str(), pBone->mNumWeights);

    size_t bone_id = get_bone_id(pBone);

    printf("\n");
    for (int i = 0; i < pBone->mNumWeights; ++i) {
        const aiVertexWeight &vw = pBone->mWeights[i];
        printf("    %d : vertex id %d weight %.2f\n", i, vw.mVertexId, vw.mWeight);


        // the true dependency of this fucn it the array result so we can do it like this
        // again here we calculate into which slot we we will place our data for tight packing
        // this methods is bound to have some empty slots but this is not a big hit
        uint32_t global_vertex_id = mesh_base_vertex[mesh_index] + vw.mVertexId;
        printf("vertex id %d", global_vertex_id);

        assert(global_vertex_id<vertex_to_bones.size());
        AddBoneData(&vertex_to_bones[global_vertex_id], bone_id, vw.mWeight);
    }
    printf("\n");
}


void parse_mesh_bones(int mesh_index, const aiMesh *ai_mesh) {
    for (int i = 0; i < ai_mesh->mNumBones; ++i) {
        parse_single_bone(mesh_index, ai_mesh->mBones[i]);
    }
}

void parse_meshes(const aiScene *pScene) {
    printf("*****\n");
    printf("parsing %d meshes\n\n", pScene->mNumMeshes);

    int total_vertices = 0;
    int total_indices = 0;
    int total_bones = 0;

    // just alloc up front yeah
    mesh_base_vertex.resize(pScene->mNumMeshes);

    for (int mesh_index = 0; mesh_index < pScene->mNumMeshes; ++mesh_index) {
        const aiMesh *pMesh = pScene->mMeshes[mesh_index];
        int num_vertices = pMesh->mNumVertices;
        int num_indices = pMesh->mNumFaces * 3; // each face is a triangle therefore we have 3 indices per face
        int num_bones = pMesh->mNumBones;
        mesh_base_vertex[mesh_index] = total_vertices; // 0 N N+M etc etc
        printf("Mesh %d '%s' : vertices %d indices %d bones %d\n\n", mesh_index, pMesh->mName.C_Str(), num_vertices, num_indices, num_bones);


        total_vertices += num_vertices;
        total_indices += num_indices;
        total_bones += num_bones;

        // get total then new total then new total 
        // TODO TOO MANY REALLOCS FIX LATER
        vertex_to_bones.resize(total_vertices);

        if (pMesh->HasBones()) {
            parse_mesh_bones(mesh_index, pMesh);
        }
        printf("\n");
    }

    printf("\n Total Vertices  %d, total  indices ,total %d bones %d\n\n", total_vertices, total_indices, total_bones);
}

void parse_scene(const char *path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices
        | aiProcess_GenNormals
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        fprintf(stderr, "ERROR::ASSIMP:: %s", importer.GetErrorString());
        return;
    }
    parse_meshes(scene);
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
