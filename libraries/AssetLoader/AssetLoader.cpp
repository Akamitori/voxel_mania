#include "AssetLoader.h"

#include <cassert>
#include "queue_container.h"


#define STB_IMAGE_IMPLEMENTATION


#include <unordered_map>

#include "stb_image.h"



uint64_t fnv1a(const char* s) {
    const size_t len= strlen(s);
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)s[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}


typedef aiNode *aiNodePtr;
QUEUE_IMPLEMENTATION_STATIC(aiNodePtr)
VECTOR_IMPLEMENTATION(ModelMesh)
VECTOR_IMPLEMENTATION(TextureData)

static TextureData LoadTextureFromMaterial(const aiTextureType type, const char *directory, const aiMaterial *material, const aiScene *scene) {
    aiString str;
    const unsigned int texture_count_for_type = material->GetTextureCount(type);
    assert(("Currently we only support one texture per type", texture_count_for_type <= 1));

    if (texture_count_for_type == 0) {
        return nil_texture;
    }

    material->GetTexture(type, 0, &str);
    const char *path = str.C_Str();

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

    // file path texture, existing behavior
    const size_t dir_name_len = strlen(directory);
    const size_t file_name_len = strlen(path);

    char *relative_path = (char *) malloc((dir_name_len + file_name_len + 2) * sizeof(char));
    if (dir_name_len == 0) {
        strcpy(relative_path, path);
    } else {
        strcpy(relative_path, directory);
        strcat(relative_path, "/");
        strcat(relative_path, path);
    }

    const TextureData t = LoadTexture(relative_path);
    free(relative_path);
    return t;
}

static TextureData LoadTextureFromMaterial(const aiTextureType type, const char *directory, const aiMaterial *material) {
    aiString str;
    const unsigned int texture_count_for_type = material->GetTextureCount(type);
    assert(("Currently we only support one texture per type", texture_count_for_type<=1));

    if (texture_count_for_type == 0) {
        return nil_texture;
    }


    material->GetTexture(type, 0, &str);

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

    assert(("Texture path", data));
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

ModelData LoadModel(const char *path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_MakeLeftHanded     |
        aiProcess_JoinIdenticalVertices
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
    model.diffuse_textures= Vector_TextureData_Create(scene->mNumMaterials);
    model.specular_textures= Vector_TextureData_Create(scene->mNumMaterials);
    
    Vector_TextureData_Add(model.diffuse_textures, nil_texture);
    Vector_TextureData_Add(model.specular_textures,nil_texture);

    int current_diffuse_index = 1;
    int current_specular_index = 1;

    std::unordered_map<size_t, int> specular_keys{};
    std::unordered_map<size_t, int> diffuse_keys{};
    
    while (Queue_aiNodePtr_Deque(nodes_to_process, &node)) {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

            ModelMesh mesh_data{};

            const unsigned int total_vertices = mesh->mNumVertices;

            assert(total_vertices>0 && "a node with no vertices makes no sense");

            mesh_data.vertex_count = total_vertices;
            mesh_data.vertices = (Vertice *) calloc(mesh->mNumVertices, sizeof(Vertice));
            
            assert(mesh_data.vertices && "Should get memory for vertices");
            
            // load vertices
            for (unsigned int j = 0; j < total_vertices; ++j) {
                Vertice vertex{};

                vertex.Position = {
                    mesh->mVertices[j].x,
                    mesh->mVertices[j].z,
                    mesh->mVertices[j].y,

                };

                vertex.Normal = {
                    mesh->mNormals[j].x,
                    mesh->mNormals[j].z,
                    mesh->mNormals[j].y,
                };

                vertex.UVs = {0, 0};
                if (mesh->mTextureCoords[0]) {
                    vertex.UVs.x = mesh->mTextureCoords[0][j].x;
                    vertex.UVs.y = mesh->mTextureCoords[0][j].y;
                }
                
                mesh_data.vertices[j] = vertex;
            }


            mesh_data.index_count = 1024;
            mesh_data.indices = (uint32_t *) calloc(1024, sizeof(uint32_t));
            
            assert(mesh_data.indices && "Should get memory for indices");


            size_t actual_count = 0;

            // this will probably overallocate in some cases
            // the alternative is to do a double pass
            // one for getting the actual count
            // and one for allocating
            // i will argue that memory is kinda free
            for (size_t j = 0; j < mesh->mNumFaces; ++j) {
                const aiFace face = mesh->mFaces[j];
                for (unsigned int k = 0; k < face.mNumIndices; ++k) {
                    if (actual_count >= mesh_data.index_count) {
                        const size_t new_count = mesh_data.index_count * 2;
                        uint32_t *new_memory = (uint32_t *) realloc(mesh_data.indices, sizeof(uint32_t) * new_count);
                        
                        assert(new_memory && "Should get memory for more indices");
                        mesh_data.indices = new_memory;
                        mesh_data.index_count = new_count; 
                    }

                    mesh_data.indices[actual_count++] = face.mIndices[k];
                    
                }
            }

            mesh_data.index_count = actual_count;
            
            const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            
            TextureData diffuse=LoadTextureFromMaterial(aiTextureType_DIFFUSE, directory, material);
            TextureData specular=LoadTextureFromMaterial(aiTextureType_SPECULAR, directory, material);
            
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
            
            Vector_ModelMesh_Add(model.mesh_data,mesh_data);
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            Queue_aiNodePtr_Enqueue(nodes_to_process, node->mChildren[i]);
        }
    }
    
    Queue_aiNodePtr_Free(nodes_to_process);
    free(directory);
    return model;
}
