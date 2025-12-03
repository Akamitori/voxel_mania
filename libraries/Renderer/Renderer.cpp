#include "Renderer.h"
#include "ShaderLoader.h"
#include "Vector4D.h"
#include "SDL3/SDL_init.h"
#include "GL/glew.h"
#include "Camera.h"
#include "quad.h"
#include "Matrix4D.h"
#include "vector"
#include <cstddef>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION

#include <queue>

#include "stb_image.h"
#include "Transformations.h"
#include "Trigonometry.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

unsigned int world_geometry_program;
unsigned int world_unshaded_geometry_program;
unsigned int world_geometry_program_cross_textures;

unsigned int ViewMatricesBlock;
unsigned int Point_Lights_Block;
unsigned int Directional_Lights_Block;
unsigned int Spot_Lights_Block;

unsigned int ViewMatrices_binding_point = 0;
unsigned int Point_Lights_binding_point = 1;
unsigned int Directional_Lights_binding_point = 2;
unsigned int Spot_Lights_binding_point = 3;


unsigned int defaultTexture;
unsigned int defaultEmissionTexture;

enum class TextureType {
    RGB,
    RGBA
};

struct Texture {
    int id{};
    int width{};
    int height{};
    TextureType texture_type{};
    unsigned char *data{};
    char *path{};
    GLuint texture_id{};
};

struct Mesh {
    int id{};
    float *vertices{nullptr};
    size_t vertice_count{0};

    uint32_t *indices{nullptr};
    size_t index_count{0};

    GLuint VAO{0};
    GLuint VBO{0};
    GLuint VBE{0};
    int diffuse_texture_id{-1};
    int specular_texture_id{-1};
    int emission_texture_id{-1};
    unsigned int program_id{0};
};

struct Model {
    int *mesh_ids;
    size_t mesh_count;
};

struct {
    float FOV;
    float Z_near;
    float Z_far;
} ProjectionParams;

constexpr int MAX_POINT_LIGHTS = 4;

constexpr int MAX_DIRECTIONAL_LIGHTS = 4;

constexpr int MAX_SPOT_LIGHTS = 4;

struct Point_Lights {
    alignas(16) int num_of_light{0};
    PointLight Lights[MAX_POINT_LIGHTS]{};
};

struct Directional_Lights {
    alignas(16) int num_of_light{0};
    DirectionalLight Lights[MAX_DIRECTIONAL_LIGHTS]{};
};

struct Spot_Lights {
    alignas(16) int num_of_light{0};
    SpotLight Lights[MAX_SPOT_LIGHTS]{};
};

Point_Lights Point_Lights{};
Directional_Lights Directional_Lights{};
Spot_Lights Spot_Lights{};

Camera *SceneCamera = nullptr;
SDL_Window *window{};
SDL_GLContext open_gl_context{};

std::vector<Model> Models{};
std::vector<Mesh> Meshes{};
std::vector<Mesh> Lights{};


void Renderer_UploadLights();

int LoadTexture(aiTextureType type, const char *directory, const aiMaterial *material);

void OpenGLGlobalSetup() {
    world_geometry_program = InitializeProgram("program_for_regular_textures");
    world_geometry_program_cross_textures = InitializeProgram("program_for_transparent_cross_textures");
    world_unshaded_geometry_program = InitializeProgram("program_for_unshaded_textures");


    const unsigned char whitePixel[4] = {255, 255, 255, 255};

    glGenTextures(1, &defaultTexture);
    glBindTexture(GL_TEXTURE_2D, defaultTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    const unsigned char blackPixels[4] = {0, 0, 0, 0};
    glGenTextures(1, &defaultEmissionTexture);
    glBindTexture(GL_TEXTURE_2D, defaultEmissionTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackPixels);
}


void Renderer_Init(const int screen_width, const int screen_height, const float fov, const float z_near,
                   const float z_far) {
    // Initialize GLFW
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "%s\n", SDL_GetError());
    }

    ProjectionParams.FOV = fov;
    ProjectionParams.Z_near = z_near;
    ProjectionParams.Z_far = z_far;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


    SceneCamera = new Camera{};


    // Create window
    window = SDL_CreateWindow("Hello World - VAO and VBO", screen_width, screen_height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "Failed to create SDL window. Error: %s\n", SDL_GetError());
        SDL_Quit();
    }

    open_gl_context = SDL_GL_CreateContext(window);

    if (!open_gl_context) {
        fprintf(stderr, "Failed to create OpenGL context. Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    if (const GLenum err = glewInit(); err != GLEW_OK) {
        fprintf(stderr, "glewInitFailed: %s\n", reinterpret_cast<const char *>(glewGetErrorString(err)));

        SDL_GL_DestroyContext(open_gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    glViewport(0, 0, screen_width, screen_height);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // we should map values that are further away to the lower part of [0,1] for better precision
    // this is because [0,1/2] has better precision from [1/2,1] due to the nature of floating point numbers
    // in practice -> higher values => closer to the camera
    glDepthFunc(GL_GEQUAL);
    glDepthRange(0.0f, 1.0f);

    // open gl should expect depth values from [0,1]
    // for the above depth mapping to work properly on the projection level
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);


    OpenGLGlobalSetup();

    // bind this buffer and fill it with nothing
    // this can be run once
    glGenBuffers(1, &ViewMatricesBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, ViewMatricesBlock);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(Matrix4D), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &Point_Lights_Block);
    glBindBuffer(GL_UNIFORM_BUFFER, Point_Lights_Block);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Point_Lights), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &Directional_Lights_Block);
    glBindBuffer(GL_UNIFORM_BUFFER, Directional_Lights_Block);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Directional_Lights), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &Spot_Lights_Block);
    glBindBuffer(GL_UNIFORM_BUFFER, Spot_Lights_Block);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Spot_Lights), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    Renderer_ResolutionChanged(screen_width, screen_height);
    Renderer_CameraUpdate();


    unsigned int programs_to_initialize[]{
        world_geometry_program,
        world_geometry_program_cross_textures,
        world_unshaded_geometry_program
    };
    // this is run per shader
    // for now we have only one shader to worry about
    // get the index of that block out of our shader
    for (const auto &program_to_initialize: programs_to_initialize) {
        const unsigned int ViewMatrices_Index = glGetUniformBlockIndex(program_to_initialize, "ViewMatrices");
        const unsigned int Point_Lights_Index = glGetUniformBlockIndex(program_to_initialize, "Point_Lights");
        const unsigned int Directional_Lights_Index = glGetUniformBlockIndex(
            program_to_initialize, "Directional_Lights");
        const unsigned int Spot_Lights_Index = glGetUniformBlockIndex(program_to_initialize, "Spot_Lights");

        //bind it to the buffer we made at a specific binding point
        // in the geometry program, our uniform that map to a UBO  can be found at global binding pointViewMatrices_binding_point
        // in practice this mean the first call of this actually creates that association at the index
        glUniformBlockBinding(program_to_initialize, ViewMatrices_Index, ViewMatrices_binding_point);
        glUniformBlockBinding(program_to_initialize, Point_Lights_Index, Point_Lights_binding_point);
        glUniformBlockBinding(program_to_initialize, Directional_Lights_Index, Directional_Lights_binding_point);
        glUniformBlockBinding(program_to_initialize, Spot_Lights_Index, Spot_Lights_binding_point);

        // now, bind the buffer to that UBO please
        glBindBufferBase(GL_UNIFORM_BUFFER, ViewMatrices_binding_point, ViewMatricesBlock);
        glBindBufferBase(GL_UNIFORM_BUFFER, Point_Lights_binding_point, Point_Lights_Block);
        glBindBufferBase(GL_UNIFORM_BUFFER, Directional_Lights_binding_point, Directional_Lights_Block);
        glBindBufferBase(GL_UNIFORM_BUFFER, Spot_Lights_binding_point, Spot_Lights_Block);
    }
}

void Renderer_FrameStart() {
    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // set the clear value to the value associated with the "furthest" object
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


int Renderer_RegisterPrimitiveMeshData(
    const float *vertices,
    const size_t vertice_count,
    const uint32_t *indices,
    const size_t index_count
) {
    const int currentId = Meshes.size();

    Meshes.emplace_back();
    Mesh &m = Meshes.back();

    m.indices = new uint32_t[index_count];
    memset(m.indices, -1, sizeof(uint32_t) * index_count);
    memcpy(m.indices, indices, sizeof(uint32_t) * index_count);

    m.index_count = index_count;

    // the idea is to add a bunch of UVs at the end
    const size_t actual_vertex_count = vertice_count / 3;
    constexpr size_t float_per_vertex = 8;
    const size_t totalFloats = actual_vertex_count * float_per_vertex;
    m.vertices = new float[totalFloats]{};
    m.vertice_count = totalFloats;
    m.diffuse_texture_id = m.specular_texture_id = m.emission_texture_id = -1;


    // sets uvs and vertices
    for (size_t v = 0; v < actual_vertex_count; ++v) {
        m.vertices[v * float_per_vertex + 0] = vertices[v * 3 + 0];
        m.vertices[v * float_per_vertex + 1] = vertices[v * 3 + 1];
        m.vertices[v * float_per_vertex + 2] = vertices[v * 3 + 2];

        m.vertices[v * float_per_vertex + 6] = 1;
        m.vertices[v * float_per_vertex + 7] = 1;
    }

    // accumulate normals
    constexpr int vertices_per_triangle = 3;
    const size_t total_triangles = index_count / vertices_per_triangle;
    for (size_t v = 0; v < total_triangles; ++v) {
        // we got the 3 indices
        const int index0 = m.indices[v * vertices_per_triangle + 0];
        const int index1 = m.indices[v * vertices_per_triangle + 1];
        const int index2 = m.indices[v * vertices_per_triangle + 2];

        Vector3D v0 = {
            m.vertices[index0 * float_per_vertex + 0],
            m.vertices[index0 * float_per_vertex + 1],
            m.vertices[index0 * float_per_vertex + 2]
        };

        Vector3D v1 = {
            m.vertices[index1 * float_per_vertex + 0],
            m.vertices[index1 * float_per_vertex + 1],
            m.vertices[index1 * float_per_vertex + 2]
        };

        Vector3D v2 = {
            m.vertices[index2 * float_per_vertex + 0],
            m.vertices[index2 * float_per_vertex + 1],
            m.vertices[index2 * float_per_vertex + 2]
        };

        Vector3D vector1 = v1 - v0;
        Vector3D vector2 = v2 - v0;
        const Vector3D normal = cross(vector1, vector2);

        m.vertices[index0 * float_per_vertex + 3] += normal.x;
        m.vertices[index0 * float_per_vertex + 4] += normal.y;
        m.vertices[index0 * float_per_vertex + 5] += normal.z;

        m.vertices[index1 * float_per_vertex + 3] += normal.x;
        m.vertices[index1 * float_per_vertex + 4] += normal.y;
        m.vertices[index1 * float_per_vertex + 5] += normal.z;

        m.vertices[index2 * float_per_vertex + 3] += normal.x;
        m.vertices[index2 * float_per_vertex + 4] += normal.y;
        m.vertices[index2 * float_per_vertex + 5] += normal.z;
    }

    for (size_t v = 0; v < actual_vertex_count; ++v) {
        Vector3D normal{
            m.vertices[v * float_per_vertex + 3],
            m.vertices[v * float_per_vertex + 4],
            m.vertices[v * float_per_vertex + 5]
        };

        normal = normalize(normal);

        m.vertices[v * float_per_vertex + 3] = normal.x;
        m.vertices[v * float_per_vertex + 4] = normal.y;
        m.vertices[v * float_per_vertex + 5] = normal.z;
    }

    m.program_id = world_geometry_program;

    return currentId;
}

int Renderer_RegisterUnshadedTexture(
    const float *vertices,
    const size_t vertice_count,
    const uint32_t *indices,
    const size_t index_count
) {
    const int currentId = Lights.size();

    Lights.emplace_back();
    Mesh &m = Lights.back();

    m.indices = new uint32_t[index_count];
    memcpy(m.indices, indices, index_count * sizeof(uint32_t));

    m.index_count = index_count;

    // the idea is to add a bunch of UVs at the end
    const size_t actual_vertex_count = vertice_count / 3;
    constexpr size_t float_per_vertex = 8;
    const size_t totalFloats = actual_vertex_count * float_per_vertex;
    m.vertices = new float[totalFloats]{};
    m.vertice_count = totalFloats;
    m.diffuse_texture_id = m.specular_texture_id = m.emission_texture_id = -1;


    // sets uvs and vertices
    for (size_t v = 0; v < actual_vertex_count; ++v) {
        m.vertices[v * float_per_vertex + 0] = vertices[v * 3 + 0];
        m.vertices[v * float_per_vertex + 1] = vertices[v * 3 + 1];
        m.vertices[v * float_per_vertex + 2] = vertices[v * 3 + 2];

        m.vertices[v * float_per_vertex + 4] = 1;
        m.vertices[v * float_per_vertex + 5] = 1;
    }

    m.program_id = world_unshaded_geometry_program;

    return currentId;
}


std::vector<Texture> textures{};


int Renderer_RegisterTexture(const char *path) {
    int width;
    int height;
    int nrChannels;

    // let's assume for now that we are using rgb
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    assert(("Texture path", data));
    assert(("Support only for RGBA", nrChannels == 3 || nrChannels == 4));

    const int id = textures.size();

    char *texture_path = new char[strlen(path) + 1];
    texture_path[0] = '\0';
    strcat(texture_path, path);
    const Texture t{
        id,
        width,
        height,
        nrChannels == 3 ? TextureType::RGB : TextureType::RGBA,
        data,
        texture_path,
    };
    textures.push_back(t);
    return id;
}


int Renderer_RegisterTexturedMesh(
    const int diffuse_texture_id,
    const int specular_texture_id,
    const int emission_texture_id,
    const float *vertices,
    const size_t vertice_count,
    const uint32_t *indices,
    const size_t index_count
) {
    assert(("Texture should be indexable", diffuse_texture_id < textures.size()));
    assert(("Texture should be indexable", specular_texture_id < textures.size()));
    assert(("Texture should be indexable", emission_texture_id < 0 || specular_texture_id < textures.size()));

    const int mesh_id = Meshes.size();
    Meshes.emplace_back();
    Mesh &m = Meshes.back();

    m.vertices = new float[vertice_count];
    memcpy(m.vertices, vertices, vertice_count * sizeof(float));
    m.vertice_count = vertice_count;

    m.indices = new uint32_t[index_count];
    memcpy(m.indices, indices, index_count * sizeof(uint32_t));

    m.index_count = index_count;

    m.id = mesh_id;
    m.diffuse_texture_id = diffuse_texture_id;
    m.specular_texture_id = specular_texture_id;
    m.emission_texture_id = emission_texture_id;
    m.program_id = world_geometry_program;

    return mesh_id;
}

int Renderer_RegisterTextured_Cross_Mesh(const int texture_id, const float scale) {
    assert(("Support only downsizing", scale<=1));

    constexpr int total_quads = 2;

    Mesh m{};
    m.vertice_count = quad::vertices_count_uv * total_quads;
    m.vertices = new float[m.vertice_count];

    for (int i = 0; i < total_quads; ++i) {
        memcpy(
            m.vertices + i * quad::vertices_count_uv,
            quad::vertex_data_uv_1_part_texture,
            quad::vertices_count_uv * sizeof(float)
        );
    }
    Matrix4D scale_matrix{scale};
    scale_matrix[3].w = 1;

    Matrix4D translation_matrix{1};
    const float amount_lost_per_side = (1 - scale) / 2;
    translation_matrix[3] = {0, 0, -amount_lost_per_side, 1};


    const auto rotate_45 = rotation_z_matrix4D(DegreeToRadians(45));
    const auto rotate_45_minus = rotation_z_matrix4D(DegreeToRadians(-45));


    const Matrix4D rotations_per_quad[total_quads] = {rotate_45, rotate_45_minus};


    Matrix4D rotations_and_scales_per_quad[total_quads];

    for (int i = 0; i < total_quads; ++i) {
        rotations_and_scales_per_quad[i] = translation_matrix * rotations_per_quad[i] * scale_matrix;
    }


    constexpr int stride = 8;
    // iterate over the vertices which have 8 elements
    for (int i = 0; i < total_quads; i++) {
        const int index_offset = i * quad::vertices_count_uv;
        const Matrix4D rotate_and_scale = rotations_and_scales_per_quad[i];
        const Matrix4D rotate = rotations_per_quad[i];

        for (int j = 0; j < quad::vertices_count_uv; j += stride) {
            const Vector3D transformed_point = transform_point(
                rotate_and_scale,
                Vector3D{
                    m.vertices[index_offset + j + 0],
                    m.vertices[index_offset + j + 1],
                    m.vertices[index_offset + j + 2],
                }
            );

            const Vector3D transformed_normal = transform_vector(
                rotate,
                {
                    m.vertices[index_offset + j + 3],
                    m.vertices[index_offset + j + 4],
                    m.vertices[index_offset + j + 5],
                }
            );

            m.vertices[index_offset + j + 0] = transformed_point.x;
            m.vertices[index_offset + j + 1] = transformed_point.y;
            m.vertices[index_offset + j + 2] = transformed_point.z;
            m.vertices[index_offset + j + 3] = transformed_normal.x;
            m.vertices[index_offset + j + 4] = transformed_normal.y;
            m.vertices[index_offset + j + 5] = transformed_normal.z;
        }
    }


    m.index_count = quad::vertex_indices_count_uv * total_quads;;
    m.indices = new uint32_t[m.index_count];

    constexpr int total_elements_for_1_quad = (quad::vertices_count_uv / stride);
    for (int quad_idx = 0; quad_idx < total_quads; ++quad_idx) {
        const int starting_point = quad_idx * quad::vertex_indices_count_uv;
        for (int i = 0; i < quad::vertex_indices_count_uv; ++i) {
            m.indices[starting_point + i] =
                    quad::vertex_indices_uvs[i] + quad_idx * total_elements_for_1_quad;
        }
    }

    const int currentId = Meshes.size();
    m.id = currentId;

    m.diffuse_texture_id = texture_id;
    m.program_id = world_geometry_program_cross_textures;

    Meshes.push_back(m);

    return currentId;
}


int Renderer_Register_Model(const char *path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_MakeLeftHanded
    );
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        fprintf(stderr, "ERROR::ASSIMP:: %s", importer.GetErrorString());
        return -1;
    }


    // if we are here path is probably valid;
    const char *last_slash = strrchr(path, '/');
    const size_t chars_to_copy = last_slash ? (last_slash - path) : strlen(path);
    char *directory = new char[chars_to_copy];
    memcpy(directory, path, chars_to_copy * sizeof(char));
    directory[chars_to_copy] = '\0';

    // keep this bastard queue for now
    // remove later because I hate STL
    std::queue<aiNode *> nodes_to_process{};
    nodes_to_process.push(scene->mRootNode);


    const int model_id = Models.size();
    Models.emplace_back(new int[scene->mNumMeshes], scene->mNumMeshes);
    const Model &model_to_register = Models.back();

    int count = 0;

    while (!nodes_to_process.empty()) {
        const aiNode *node = nodes_to_process.front();
        nodes_to_process.pop();
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

            const size_t total_floats = mesh->mNumVertices * 8;
            float *vertex_data = new float[total_floats];

            // get the vertex data
            for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
                const Vector3D position{
                    mesh->mVertices[j].x,
                    mesh->mVertices[j].y,
                    mesh->mVertices[j].z,
                };

                const Vector3D normal{
                    mesh->mNormals[j].x,
                    mesh->mNormals[j].y,
                    mesh->mNormals[j].z,
                };

                float uv_x = 0;
                float uv_y = 0;

                if (mesh->mTextureCoords[0]) {
                    uv_x = mesh->mTextureCoords[0][j].x;
                    uv_y = mesh->mTextureCoords[0][j].y;
                }

                const unsigned int offset = j * 8;
                // flip the axes so y swaps with z and open gl forward becomes backward
                vertex_data[offset] = position.x;
                vertex_data[offset + 1] = position.z;
                vertex_data[offset + 2] = position.y;

                vertex_data[offset + 3] = normal.x;
                vertex_data[offset + 4] = normal.z;
                vertex_data[offset + 5] = normal.y;

                vertex_data[offset + 6] = uv_x;
                vertex_data[offset + 7] = uv_y;
            }

            // get the indices data
            size_t total_indices = 0;
            for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
                total_indices += mesh->mFaces[j].mNumIndices;
            }
            uint32_t *indice_data = new uint32_t[total_indices];
            for (size_t j = 0, indice_index = 0; j < mesh->mNumFaces; ++j) {
                const aiFace face = mesh->mFaces[j];
                for (unsigned int k = 0; k < face.mNumIndices; ++k, ++indice_index)
                    indice_data[indice_index] = face.mIndices[k];
            }
            
            const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            const int diffuse_texture_id = LoadTexture(aiTextureType_DIFFUSE, directory, material);
            const int specular_texture_id = LoadTexture(aiTextureType_SPECULAR, directory, material);

            model_to_register.mesh_ids[count++] = Renderer_RegisterTexturedMesh(
                diffuse_texture_id,
                specular_texture_id,
                -1,
                vertex_data,
                total_floats,
                indice_data,
                total_indices
            );

            delete[] indice_data;
            delete[] vertex_data;
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            nodes_to_process.push(node->mChildren[i]);
        }
    }


    return model_id;
}


int Renderer_Register_Directional_Light(const DirectionalLight &light) {
    assert(("Registered more lights than possible", Directional_Lights.num_of_light<MAX_DIRECTIONAL_LIGHTS));
    const int currentId = Directional_Lights.num_of_light;
    Directional_Lights.Lights[currentId] = light;
    ++Directional_Lights.num_of_light;
    return currentId;
}

int Renderer_Register_Point_Light(const PointLight &light) {
    assert(("Registered more lights than possible", Point_Lights.num_of_light<MAX_POINT_LIGHTS));
    const int currentId = Point_Lights.num_of_light;
    Point_Lights.Lights[currentId] = light;
    ++Point_Lights.num_of_light;
    return currentId;
}

int Renderer_Register_Spot_Light(const SpotLight &light) {
    assert(("Registered more lights than possible", Spot_Lights.num_of_light<MAX_SPOT_LIGHTS));
    const int currentId = Spot_Lights.num_of_light;
    Spot_Lights.Lights[currentId] = light;
    ++Spot_Lights.num_of_light;
    return currentId;
}

// this is for untextured meshes ;)
void SendGeometryDataToTheGPU() {
    for (int i = 0; i < Meshes.size(); ++i) {
        auto &m = Meshes[i];
        glUseProgram(m.program_id);

        glGenVertexArrays(1, &m.VAO);
        glBindVertexArray(m.VAO);

        glGenBuffers(1, &m.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m.vertice_count, m.vertices, GL_STATIC_DRAW);

        constexpr int float_per_vertex = 8;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, float_per_vertex * sizeof(float), nullptr);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, float_per_vertex * sizeof(float),
                              reinterpret_cast<void *>(3 * sizeof(float)));


        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, float_per_vertex * sizeof(float),
                              reinterpret_cast<void *>(6 * sizeof(float)));

        glGenBuffers(1, &m.VBE);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.VBE);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m.index_count, m.indices, GL_STATIC_DRAW);

        delete[] m.vertices;
        m.vertices = nullptr;

        delete[] m.indices;
        m.indices = nullptr;
    }
}

// this is for untextured meshes ;)
void SendLightGeometryDataToTheGPU() {
    for (int i = 0; i < Lights.size(); ++i) {
        auto &light = Lights[i];
        glUseProgram(light.program_id);

        glGenVertexArrays(1, &light.VAO);
        glBindVertexArray(light.VAO);

        glGenBuffers(1, &light.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, light.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * light.vertice_count, light.vertices, GL_STATIC_DRAW);

        constexpr int float_per_vertex = 8;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, float_per_vertex * sizeof(float), nullptr);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, float_per_vertex * sizeof(float),
                              reinterpret_cast<void *>(4 * sizeof(float)));

        glGenBuffers(1, &light.VBE);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light.VBE);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * light.index_count, light.indices, GL_STATIC_DRAW);

        delete[] light.vertices;
        light.vertices = nullptr;

        delete[] light.indices;
        light.indices = nullptr;
    }
}

void SendTextureDataToTheGPU() {
    for (auto &t: textures) {
        glGenTextures(1, &t.texture_id);
        glBindTexture(GL_TEXTURE_2D, t.texture_id);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        const int mode = t.texture_type == TextureType::RGB ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, mode, t.width, t.height, 0, mode, GL_UNSIGNED_BYTE, t.data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(t.data);
        t.data = nullptr;
    }
}


// we can probably do some post-processing here if we want for the index sorting
void Renderer_FinalizeMeshLoading() {
    SendGeometryDataToTheGPU();
    SendLightGeometryDataToTheGPU();
    SendTextureDataToTheGPU();
    Renderer_UploadLights();
}

// TODO we can probably work with a Matrix4D eventually
void Renderer_Draw(const int mesh_id, const Vector3D pos, const Vector3D color, const Material material) {
    const Mesh mesh = Meshes[mesh_id];
    const unsigned int program_to_use = mesh.program_id;
    glUseProgram(program_to_use);

    // this means all programs need this uniform
    const GLint voxel_color = glGetUniformLocation(program_to_use, "voxel_color");
    const GLint position_id = glGetUniformLocation(program_to_use, "position");
    const GLint view_pos_id = glGetUniformLocation(program_to_use, "view_position");
    const GLuint diffuse_texture_id = mesh.diffuse_texture_id == -1
                                          ? defaultTexture
                                          : textures[mesh.diffuse_texture_id].texture_id;
    const GLuint specular_texture_id = mesh.specular_texture_id == -1
                                           ? defaultEmissionTexture
                                           : textures[mesh.specular_texture_id].texture_id;

    const GLuint emission_texture_id = mesh.emission_texture_id == -1
                                           ? defaultEmissionTexture
                                           : textures[mesh.emission_texture_id].texture_id;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_texture_id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular_texture_id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, emission_texture_id);
    glBindVertexArray(mesh.VAO);

    const Vector4D color_4{color.x, color.y, color.z, 1};

    glUniform3fv(position_id, 1, &pos.x);
    glUniform4fv(voxel_color, 1, &color_4.x);

    glUniform3fv(view_pos_id, 1, &SceneCamera->position.x);

    // TODO set slot to zero


    const GLint material_diffuse_id = glGetUniformLocation(program_to_use, "material.diffuse");
    glUniform1i(material_diffuse_id, 0);

    const GLint material_specular_id = glGetUniformLocation(program_to_use, "material.specular");
    glUniform1i(material_specular_id, 1);

    const GLint material_emission_id = glGetUniformLocation(program_to_use, "material.emission");
    glUniform1i(material_emission_id, 2);

    const GLint material_shineness_id = glGetUniformLocation(program_to_use, "material.shininess");
    glUniform1fv(material_shineness_id, 1, &material.shininess);

    assert(mesh.index_count <= INT_MAX); // this should never happen 
    glDrawElements(GL_TRIANGLES, static_cast<int>(mesh.index_count),GL_UNSIGNED_INT, nullptr);
}

void Renderer_Draw_Model(int model_id, Vector3D pos, Vector3D color, Material material) {
    const Model model = Models[model_id];
    for (size_t i = 0; i < model.mesh_count; ++i) {
        Renderer_Draw(model.mesh_ids[i], pos, color, material);
    }
}

void Renderer_DrawUnshadedTexture(const int light_id, const Vector3D pos, const Vector3D color) {
    const Mesh light = Lights[light_id];
    const unsigned int program_to_use = light.program_id;
    glUseProgram(program_to_use);

    // this means all programs need this uniform
    const GLint voxel_color = glGetUniformLocation(program_to_use, "voxel_color");
    const GLint position_id = glGetUniformLocation(program_to_use, "position");
    const GLuint diffuse_texture_id = light.diffuse_texture_id == -1
                                          ? defaultTexture
                                          : textures[light.diffuse_texture_id].texture_id;


    glActiveTexture(GL_TEXTURE0); // Add this
    glBindTexture(GL_TEXTURE_2D, diffuse_texture_id);
    glBindVertexArray(light.VAO);

    const Vector4D color_4{color.x, color.y, color.z, 1};

    glUniform3fv(position_id, 1, &pos.x);
    glUniform4fv(voxel_color, 1, &color_4.x);

    assert(light.index_count <= INT_MAX); // this should never happen
    glDrawElements(GL_TRIANGLES, static_cast<int>(light.index_count),GL_UNSIGNED_INT, nullptr);
}

void Renderer_FrameEnd() {
    SDL_GL_SwapWindow(window);
}


void Renderer_Destroy() {
    for (auto &m: Meshes) {
        glDeleteVertexArrays(1, &m.VAO);
        glDeleteBuffers(1, &m.VBO);
        glDeleteBuffers(1, &m.VBE);
    }

    glDeleteBuffers(1, &ViewMatricesBlock);
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(open_gl_context);
    SDL_Quit();
}


void Renderer_ResolutionChanged(const int new_screen_width, const int new_screen_height) {
    auto m = PerspectiveMatrix(ProjectionParams.FOV, ProjectionParams.Z_near, ProjectionParams.Z_far,
                               static_cast<float>(new_screen_width) / static_cast<float>(new_screen_height));

    glViewport(0, 0, new_screen_width, new_screen_height);

    glBindBuffer(GL_UNIFORM_BUFFER, ViewMatricesBlock);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4D), &m[0].x);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer_CameraUpdate() {
    auto m = CameraLookAtMatrix(*SceneCamera);
    glBindBuffer(GL_UNIFORM_BUFFER, ViewMatricesBlock);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Matrix4D), sizeof(Matrix4D), &m[0].x);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer_Change_Emission(const int mesh_id, const int emission_texture_id) {
    Meshes[mesh_id].emission_texture_id = emission_texture_id;
}

void Renderer_UploadLights() {
    glBindBuffer(GL_UNIFORM_BUFFER, Point_Lights_Block);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Point_Lights), &Point_Lights);

    glBindBuffer(GL_UNIFORM_BUFFER, Directional_Lights_Block);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Directional_Lights), &Directional_Lights);


    glBindBuffer(GL_UNIFORM_BUFFER, Spot_Lights_Block);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Spot_Lights), &Spot_Lights);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

int LoadTexture(aiTextureType type, const char *directory, const aiMaterial *material) {
    aiString str;
    const unsigned int texture_count_for_type = material->GetTextureCount(type);
    assert((("Currently we only support one texture per type"), texture_count_for_type<=1));

    if (texture_count_for_type == 0) {
        return -1;
    }


    material->GetTexture(type, 0, &str);

    const char *file_name = str.C_Str();
    const size_t dir_name_len = strlen(directory);
    const size_t file_name_len = strlen(file_name);

    char *relative_path = new char[dir_name_len + file_name_len + 2];
    relative_path[0] = '\0';
    if (dir_name_len == 0) {
        strcat(relative_path, file_name);
    } else {
        strcat(relative_path, directory);
        strcat(relative_path, "/");
        strcat(relative_path, file_name);
    }

    const int texture_id = Renderer_RegisterTexture(relative_path);
    free(relative_path);

    return texture_id;
}
