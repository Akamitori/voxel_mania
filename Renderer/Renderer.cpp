#include "Renderer.h"

#include <iostream>

#include "ShaderLoader.h"
#include "Vector4D.h"
#include "SDL3/SDL_init.h"
#include "GL/glew.h"
#include "../Camera.h"
#include "quad.h"
#include "Matrix4D.h"
#include "vector"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "Transformations.h"
#include "Trigonometry.h"

unsigned int world_geometry_program;
unsigned int world_geometry_program_cross_textures;
unsigned int ViewMatricesBlock;
unsigned int ViewMatrices_binding_point = 0;


unsigned int defaultTexture;

void OpenGLGlobalSetup() {
    world_geometry_program = InitializeProgram("program1");
    world_geometry_program_cross_textures = InitializeProgram("program_for_transparent_cross_textures");


    unsigned char whitePixel[4] = {255, 255, 255, 255};
    glGenTextures(1, &defaultTexture);
    glBindTexture(GL_TEXTURE_2D, defaultTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}


struct {
    float FOV;
    float Z_near;
    float Z_far;
} ProjectionParams;

void Renderer_Init(int screen_width, int screen_height, float fov, float z_near, float z_far) {
    // Initialize GLFW
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << SDL_GetError() << std::endl;
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
        std::cerr << "Failed to create SDL window. Error :" << SDL_GetError() << std::endl;
        SDL_Quit();
    }

    open_gl_context = SDL_GL_CreateContext(window);

    if (!open_gl_context) {
        std::cerr << "Failed to create OpenGL context. Error : " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    if (const GLenum err = glewInit(); err != GLEW_OK) {
        std::cerr << "glewInitFailed: " << glewGetErrorString(err) << std::endl;

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

    Renderer_ResolutionChanged(screen_width, screen_height);
    Renderer_CameraUpdate();


    unsigned int programs_to_initialize[]{
        world_geometry_program,
        world_geometry_program_cross_textures
    };
    // this is run per shader
    // for now we have only one shader to worry about
    // get the index of that block out of our shader
    for (const auto &program_to_initialize: programs_to_initialize) {
        unsigned int ViewMatrices_Index = glGetUniformBlockIndex(program_to_initialize, "ViewMatrices");

        //bind it to the buffer we made at a specific binding point
        // in the geometry program, our uniform that map to a UBO  can be found at global binding pointViewMatrices_binding_point
        // in practice this mean the first call of this actually creates that association at the index
        glUniformBlockBinding(world_geometry_program, ViewMatrices_Index, ViewMatrices_binding_point);

        // now, bind the buffer to that UBO please
        glBindBufferBase(GL_UNIFORM_BUFFER, ViewMatrices_binding_point, ViewMatricesBlock);
    }
}

void Renderer_FrameStart() {
    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // set the clear value to the value associated with the "furthest" object
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

struct Mesh {
    int id{};
    float *vertices{nullptr};
    size_t vertice_count{0};

    int *indices{nullptr};
    size_t index_count{0};

    GLuint VAO{0};
    GLuint VBO{0};
    GLuint VBE{0};
    int texture_id{-1};
    unsigned int program_id{0};
};

std::vector<Mesh> Meshes{};

// load the models into our cache
int Renderer_RegisterPrimitiveMeshData(const float *vertices, const size_t vertice_count, const int *indices,
                                       const size_t index_count) {
    const int currentId = Meshes.size();

    Meshes.emplace_back();
    Mesh &m = Meshes.back();

    m.indices = new int[index_count];
    memcpy(m.indices, indices, index_count * sizeof(int));

    m.index_count = index_count;

    // the idea is to add a bunch of UVs at the end
    const size_t actual_vertex_count = vertice_count / 3;
    constexpr size_t float_per_vertex = 5;
    const size_t totalFloats = actual_vertex_count * float_per_vertex;
    m.vertices = new float[totalFloats];
    m.vertice_count = totalFloats;
    m.texture_id = -1;

    for (size_t v = 0; v < actual_vertex_count; ++v) {
        m.vertices[v * float_per_vertex + 0] = vertices[v * 3 + 0];
        m.vertices[v * float_per_vertex + 1] = vertices[v * 3 + 1];
        m.vertices[v * float_per_vertex + 2] = vertices[v * 3 + 2];

        m.vertices[v * float_per_vertex + 3] = 1;
        m.vertices[v * float_per_vertex + 4] = 1;
    }
    m.program_id = world_geometry_program;

    return currentId;
}

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
    GLuint texture_id{};
};


std::vector<Texture> textures{};


int Renderer_RegisterTexture(const std::string &path) {
    int width;
    int height;
    int nrChannels;

    // let's assume for now that we are using rgb
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    assert(("Texture path", data));
    assert(("Support only for RGBA", nrChannels == 3 || nrChannels == 4));

    const int id = textures.size();
    const Texture t{
        id,
        width,
        height,
        nrChannels == 3 ? TextureType::RGB : TextureType::RGBA,
        data
    };
    textures.push_back(t);
    return id;
}


int Renderer_RegisterTexturedMesh(int texture_id, const float *vertices, const size_t vertice_count, const int *indices,
                                  const size_t index_count) {
    assert(("Texture should be indexable", texture_id < textures.size()));

    const int mesh_id = Meshes.size();
    Meshes.emplace_back();
    Mesh &m = Meshes.back();

    m.vertices = new float[vertice_count];
    memcpy(m.vertices, vertices, vertice_count * sizeof(float));
    m.vertice_count = vertice_count;

    m.indices = new int[index_count];
    memcpy(m.indices, indices, index_count * sizeof(int));

    m.index_count = index_count;

    m.id = mesh_id;
    m.texture_id = texture_id;
    m.program_id = world_geometry_program;

    return mesh_id;
}

int Renderer_RegisterTextured_Cross_Mesh(const int texture_id, float scale) {
    assert(("Support only downsizing", scale<=1));

    Mesh m{};
    m.vertice_count = quad::vertices_count_uv * 2;
    m.vertices = new float[m.vertice_count];
    memcpy(m.vertices, quad::vertex_data_uv_1_part_texture, quad::vertices_count_uv * sizeof(float));
    memcpy(m.vertices + quad::vertices_count_uv, quad::vertex_data_uv_1_part_texture,
           quad::vertices_count_uv * sizeof(float));
    Matrix4D scale_matrix{scale};
    scale_matrix[3].w = 1;


    Matrix4D translation_matrix{1};
    float amount_lost_per_side = (1 - scale) / 2;
    translation_matrix[3] = {0, 0, -amount_lost_per_side, 1};

    auto rotation_and_scale_45 = translation_matrix * rotation_z_matrix4D(DegreeToRadians(45)) * scale_matrix;
    auto rotation_and_scale_minus_45 = translation_matrix * rotation_z_matrix4D(DegreeToRadians(-45)) * scale_matrix;
    for (int i = 0; i < quad::vertices_count_uv; i += 5) {
        Vector3D v_45_init = Vector3D{
            m.vertices[i + 0],
            m.vertices[i + 1],
            m.vertices[i + 2],
        };

        Vector3D v_45 = transform_point(rotation_and_scale_45, v_45_init);
        m.vertices[i + 0] = v_45.x;
        m.vertices[i + 1] = v_45.y;
        m.vertices[i + 2] = v_45.z;

        Vector3D v_45_second_set_init = Vector3D{
            m.vertices[quad::vertices_count_uv + i + 0],
            m.vertices[quad::vertices_count_uv + i + 1],
            m.vertices[quad::vertices_count_uv + i + 2],
        };

        Vector3D v_45_minus = transform_point(rotation_and_scale_minus_45, v_45_second_set_init);
        m.vertices[quad::vertices_count_uv + i + 0] = v_45_minus.x;
        m.vertices[quad::vertices_count_uv + i + 1] = v_45_minus.y;
        m.vertices[quad::vertices_count_uv + i + 2] = v_45_minus.z;
    }

    m.index_count = quad::vertex_indices_count_uv * 2;;
    m.indices = new int[m.index_count];
    memcpy(m.indices, quad::vertex_indices_uvs, quad::vertex_indices_count_uv * sizeof(int));

    for (int i = quad::vertex_indices_count_uv; i < 2 * quad::vertex_indices_count_uv; ++i) {
        m.indices[i] = quad::vertex_indices_uvs[i - quad::vertex_indices_count_uv]
                       + (quad::vertices_count_uv / 5);
    }

    const int currentId = Meshes.size();
    m.id = currentId;
    m.texture_id = texture_id;
    m.program_id = world_geometry_program_cross_textures;

    Meshes.push_back(m);

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

        constexpr int float_per_vertex = 5;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, float_per_vertex * sizeof(float), nullptr);


        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, float_per_vertex * sizeof(float),
                              reinterpret_cast<void *>(3 * sizeof(float)));

        glGenBuffers(1, &m.VBE);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.VBE);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * m.index_count, m.indices, GL_STATIC_DRAW);

        delete[] m.vertices;
        m.vertices = nullptr;

        delete[] m.indices;
        m.indices = nullptr;
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
    SendTextureDataToTheGPU();
}

// TODO we can probably work with a Matrix4D eventually
void Renderer_Draw(int mesh_id, Vector3D pos, Vector3D color) {
    Mesh mesh = Meshes[mesh_id];
    unsigned int program_to_use = mesh.program_id;
    glUseProgram(program_to_use);

    // this means all programs need this uniform
    GLint voxel_color = glGetUniformLocation(program_to_use, "voxel_color");
    GLint position_id = glGetUniformLocation(program_to_use, "position");

    GLuint texture_to_bind = mesh.texture_id == -1 ? defaultTexture : textures[mesh.texture_id].texture_id;


    glBindTexture(GL_TEXTURE_2D, texture_to_bind);
    glBindVertexArray(mesh.VAO);

    Vector4D color_4{color.x, color.y, color.z, 1};

    glUniform3fv(position_id, 1, &pos.x);
    glUniform4fv(voxel_color, 1, &color_4.x);
    glDrawElements(GL_TRIANGLES, mesh.index_count,GL_UNSIGNED_INT, nullptr);
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


void Renderer_ResolutionChanged(int new_screen_width, int new_screen_height) {
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
