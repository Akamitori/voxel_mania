#include "AssetLoader.h"

#include <cassert>
#include "queue_container.h"


#define STB_IMAGE_IMPLEMENTATION


#include "stb_image.h"


typedef aiNode *aiNodePtr;
QUEUE_IMPLEMENTATION_STATIC(aiNodePtr)
VECTOR_IMPLEMENTATION(ModelMesh)

static TextureData LoadTextureFromMaterial(const aiTextureType type, const char *directory, const aiMaterial *material) {
    aiString str;
    const unsigned int texture_count_for_type = material->GetTextureCount(type);
    assert(("Currently we only support one texture per type", texture_count_for_type<=1));

    if (texture_count_for_type == 0) {
        return {};
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

    const TextureData t = LoadTextureNew(relative_path);
    free(relative_path);

    return t;
}

TextureData LoadTextureNew(const char *path) {
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
        path_to_load
    };
}

ModelData LoadModel(const char *path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_MakeLeftHanded
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


    model.mesh_data = Vector_ModelMesh_Create(1024);
    
    
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
            mesh_data.diffuse = LoadTextureFromMaterial(aiTextureType_DIFFUSE, directory, material);
            mesh_data.specular = LoadTextureFromMaterial(aiTextureType_SPECULAR, directory, material);
            
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
