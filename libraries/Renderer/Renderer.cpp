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


#include "stb_image.h"
#include "Transformations.h"
#include "Trigonometry.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "queue_container.h"
#include "vector_container.h"


typedef aiNode *aiNodePtr;
QUEUE_DECLARATION_STATIC(aiNodePtr)

QUEUE_IMPLEMENTATION(aiNodePtr)


Camera *SceneCamera = nullptr;
SDL_Window *window{};
SDL_GLContext open_gl_context{};

static unsigned int world_geometry_program;
static unsigned int world_unshaded_geometry_program;
static unsigned int world_geometry_program_cross_textures;
static unsigned int world_geometry_program_outlines;
static unsigned int world_geometry_program_outlines_cross_textures;


struct screen {
    unsigned int screen_texture_program{0};
    int Width{};
    int Height{};

    int Internal_Width{};
    int Internal_Height{};

    GLuint VAO{0};
    GLuint VBO{0};
    GLuint VBE{0};
    int IndexCount{0};

    int Samples{0};

    GLuint frame_buffer{0};
    GLuint frame_buffer_multisample_result{0};
    GLuint texture_multisample{0};
    GLuint screen_texture{0};
    GLuint texture_render_buffer_object{0};
};

static screen Screen_Texture{};

static unsigned int ViewMatricesBlock;
static unsigned int Point_Lights_Block;
static unsigned int Directional_Lights_Block;
static unsigned int Spot_Lights_Block;

static unsigned int ViewMatrices_binding_point = 0;
static unsigned int Point_Lights_binding_point = 1;
static unsigned int Directional_Lights_binding_point = 2;
static unsigned int Spot_Lights_binding_point = 3;


static unsigned int defaultTexture;
static unsigned int defaultEmissionTexture;

enum class TextureType {
    RGB,
    RGB_ALPHA,
    SRGB,
    SRGB_ALPHA
};

enum class TextureInternalStorageConversion {
    None,
    Linear,
};

struct Texture {
    int id{};
    int width{};
    int height{};
    TextureType loaded_texture_type{};
    TextureType stored_texture_storage{};
    unsigned char *data{};
    char *path{};
    TextureWrapMode WrapMode_S{};
    TextureWrapMode WrapMode_T{};

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
    unsigned int outline_program_id{0};
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

static Point_Lights Point_Lights{};
static Directional_Lights Directional_Lights{};
static Spot_Lights Spot_Lights{};

VECTOR_IMPLEMENTATION_STATIC(Mesh);
VECTOR_IMPLEMENTATION_STATIC(Model);
VECTOR_IMPLEMENTATION_STATIC(Texture);
VECTOR_IMPLEMENTATION_STATIC(uint32_t);
VECTOR_IMPLEMENTATION_STATIC(float);

static Vector_Model *Models = Vector_Model_Create(100);
static Vector_Mesh *Meshes = Vector_Mesh_Create(100);
static Vector_Mesh *UnshadedMeshes = Vector_Mesh_Create(100);
static Vector_Texture *Textures = Vector_Texture_Create(100);

static void SendLightUBOsToTheGPU();

static void SendGeometryDataToTheGPU();

static void SendLightGeometryDataToTheGPU();

static void SendTextureDataToTheGPU();

static void SendScreenTextureDataToTheGPU();

static void Draw(
    unsigned int program_to_use,
    const Mesh &mesh,
    Vector3D pos,
    Vector3D color,
    Material material
);

static int LoadTexture(aiTextureType type, const char *directory, const aiMaterial *material);

static void OpenGLGlobalSetup();

void OpenGLGlobalSetup() {
    world_geometry_program = InitializeProgram("program_for_regular_textures");
    world_geometry_program_cross_textures = InitializeProgram("program_for_transparent_cross_textures");
    world_unshaded_geometry_program = InitializeProgram("program_for_unshaded_textures");
    world_geometry_program_outlines = InitializeProgram("program_for_regular_texture_outlines");

    // no special impl for this. it was added just for completion's sake
    world_geometry_program_outlines_cross_textures = world_geometry_program_outlines;

    Screen_Texture.screen_texture_program = InitializeProgram("program_for_screen_texture");


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


void Renderer_Init(const int screen_width,
                   const int screen_height,
                   const float fov,
                   const float z_near,
                   const float z_far,
                   const int anti_aliasing_samples
) {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "%s\n", SDL_GetError());
    }

    ProjectionParams.FOV = fov;
    ProjectionParams.Z_near = z_near;
    ProjectionParams.Z_far = z_far;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


    Screen_Texture.Samples = anti_aliasing_samples;

    if (Screen_Texture.Samples > 0) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, Screen_Texture.Samples);
    }

    // add a stencil buffer
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SceneCamera = new Camera{};


    auto display_properties = SDL_GetCurrentDisplayMode(1);

    // Create window
    window = SDL_CreateWindow("Hello World - VAO and VBO", display_properties->w, display_properties->h,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
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

    // enable the stencil test. we don't need to disable it for now
    // everything not equal to 1 passes. this is just for complete init since each call will override it
    // keep  , keep and replace with 1
    // keep in mind that stencil ops can be blocked by the write mask
    glEnable(GL_STENCIL_TEST);

    // we don't need to override the glStencilFunc because we do that for each draw call
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    // keep if the stencil test fails
    // else keep if the depth pass fails
    // else replace if both tests pass
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // some plain old blending
    // the caller should take care of the ordering
    // we will definitely fix this later when we add batching
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    OpenGLGlobalSetup();

    // UBO setup part 1 : bind the buffers. bind them to the proper binding point. fill them with nothing
    glGenBuffers(1, &ViewMatricesBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, ViewMatricesBlock);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(Matrix4D), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, ViewMatrices_binding_point, ViewMatricesBlock);

    glGenBuffers(1, &Point_Lights_Block);
    glBindBuffer(GL_UNIFORM_BUFFER, Point_Lights_Block);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Point_Lights), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, Point_Lights_binding_point, Point_Lights_Block);

    glGenBuffers(1, &Directional_Lights_Block);
    glBindBuffer(GL_UNIFORM_BUFFER, Directional_Lights_Block);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Directional_Lights), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, Directional_Lights_binding_point, Directional_Lights_Block);

    glGenBuffers(1, &Spot_Lights_Block);
    glBindBuffer(GL_UNIFORM_BUFFER, Spot_Lights_Block);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Spot_Lights), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, Spot_Lights_binding_point, Spot_Lights_Block);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    Renderer_ResolutionChanged(display_properties->w, display_properties->h);
    Renderer_CameraUpdate();


    unsigned int programs_to_initialize[]{
        world_geometry_program,
        world_geometry_program_cross_textures,
        world_unshaded_geometry_program,
        world_geometry_program_outlines
    };
    // UBO setup part 2 : bind the buffers we made at to their specific binding point. do this PER SHADER.
    for (const auto &program_to_initialize: programs_to_initialize) {
        const unsigned int ViewMatrices_Index = glGetUniformBlockIndex(program_to_initialize, "ViewMatrices");
        const unsigned int Point_Lights_Index = glGetUniformBlockIndex(program_to_initialize, "Point_Lights");
        const unsigned int Directional_Lights_Index = glGetUniformBlockIndex(program_to_initialize, "Directional_Lights");
        const unsigned int Spot_Lights_Index = glGetUniformBlockIndex(program_to_initialize, "Spot_Lights");

        //bind it to the buffer we made at a specific binding point
        // in the geometry program, our uniform that map to a UBO  can be found at global binding pointViewMatrices_binding_point
        // in practice this mean the first call of this actually creates that association at the index
        glUniformBlockBinding(program_to_initialize, ViewMatrices_Index, ViewMatrices_binding_point);
        glUniformBlockBinding(program_to_initialize, Point_Lights_Index, Point_Lights_binding_point);
        glUniformBlockBinding(program_to_initialize, Directional_Lights_Index, Directional_Lights_binding_point);
        glUniformBlockBinding(program_to_initialize, Spot_Lights_Index, Spot_Lights_binding_point);
    }

    // set the internal buffer width
    Screen_Texture.Internal_Width = screen_width;
    Screen_Texture.Internal_Height = screen_height;

    // create and bind the frame buffer
    glGenFramebuffers(1, &Screen_Texture.frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, Screen_Texture.frame_buffer);

    // generate texture for which we sample the colors

    if (Screen_Texture.Samples > 0) {
        // setup for the anti alias buffer
        // it should be using both a 
        glGenTextures(1, &Screen_Texture.texture_multisample);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Screen_Texture.texture_multisample);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Screen_Texture.Samples, GL_RGB16F,
                                Screen_Texture.Internal_Width, Screen_Texture.Internal_Height, GL_TRUE
        );
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, Screen_Texture.texture_multisample, 0);

        glGenRenderbuffers(1, &Screen_Texture.texture_render_buffer_object);
        glBindRenderbuffer(GL_RENDERBUFFER, Screen_Texture.texture_render_buffer_object);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, Screen_Texture.Samples, GL_DEPTH24_STENCIL8, Screen_Texture.Internal_Width,
                                         Screen_Texture.Internal_Height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Screen_Texture.texture_render_buffer_object);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            exit(1);
        }

        // now we need to create the actual screen buffer
        // since the anti-alias buffer already does the depth and stencil test we are free to just use it as a color attachment and call it a day

        glGenFramebuffers(1, &Screen_Texture.frame_buffer_multisample_result);
        glBindFramebuffer(GL_FRAMEBUFFER, Screen_Texture.frame_buffer_multisample_result);

        // simple screen texture with no samples
        glGenTextures(1, &Screen_Texture.screen_texture);
        glBindTexture(GL_TEXTURE_2D, Screen_Texture.screen_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Screen_Texture.Internal_Width, Screen_Texture.Internal_Height, 0,GL_RGB,GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Screen_Texture.screen_texture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            exit(1);
        }
    } else {
        // simple screen texture with no samples
        glGenTextures(1, &Screen_Texture.screen_texture);
        glBindTexture(GL_TEXTURE_2D, Screen_Texture.screen_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Screen_Texture.Internal_Width, Screen_Texture.Internal_Height, 0,GL_RGB,GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        //attach it to the currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Screen_Texture.screen_texture, 0);

        // generate a renderbuffer object object for which we only do write ops
        // this is done for depth and stencil testing
        glGenRenderbuffers(1, &Screen_Texture.texture_render_buffer_object);
        glBindRenderbuffer(GL_RENDERBUFFER, Screen_Texture.texture_render_buffer_object);
        glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8, Screen_Texture.Internal_Width, Screen_Texture.Internal_Height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Screen_Texture.texture_render_buffer_object);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            exit(1);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer_FrameStart() {
    // bind to the buffer we are drawing
    glBindFramebuffer(GL_FRAMEBUFFER, Screen_Texture.frame_buffer);
    // set the viewport to be the size of the buffer we are drawing to
    glViewport(0, 0, Screen_Texture.Internal_Width, Screen_Texture.Internal_Height);

    // set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // set the clear value for the depth buffer to the value associated with the "furthest" object(it's 0 due to reverse z mapping)
    glClearDepth(0.0f);
    // set the clear value for the stencil buffer
    glClearStencil(0);
    // clear the buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


int Renderer_RegisterPrimitiveMeshData(
    const float *vertices,
    const size_t vertice_count,
    const uint32_t *indices,
    const size_t index_count
) {
    const int currentId = Vector_Mesh_Length(Meshes);

    Mesh m{};

    m.indices = (uint32_t *) malloc(index_count * sizeof(uint32_t));
    memcpy(m.indices, indices, sizeof(uint32_t) * index_count);

    m.index_count = index_count;

    // the idea is to add a bunch of UVs at the end
    const size_t actual_vertex_count = vertice_count / 3;
    constexpr size_t float_per_vertex = 8;
    const size_t totalFloats = actual_vertex_count * float_per_vertex;

    m.vertices = (float *) malloc(totalFloats * sizeof(float));
    m.vertice_count = totalFloats;
    m.diffuse_texture_id = m.specular_texture_id = m.emission_texture_id = -1;


    // sets uvs and vertices
    for (size_t v = 0; v < actual_vertex_count; ++v) {
        m.vertices[v * float_per_vertex + 0] = vertices[v * 3 + 0];
        m.vertices[v * float_per_vertex + 1] = vertices[v * 3 + 1];
        m.vertices[v * float_per_vertex + 2] = vertices[v * 3 + 2];

        m.vertices[v * float_per_vertex + 3] = 0;
        m.vertices[v * float_per_vertex + 4] = 0;
        m.vertices[v * float_per_vertex + 5] = 0;

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

    Vector_Mesh_Add(Meshes, m);

    return currentId;
}

int Renderer_RegisterUnshadedTexture(
    const float *vertices,
    const size_t vertice_count,
    const uint32_t *indices,
    const size_t index_count
) {
    const int currentId = Vector_Mesh_Length(UnshadedMeshes);

    Mesh m{};

    m.indices = (uint32_t *) malloc(index_count * sizeof(uint32_t));
    memcpy(m.indices, indices, index_count * sizeof(uint32_t));

    m.index_count = index_count;

    // the idea is to add a bunch of UVs at the end
    const size_t actual_vertex_count = vertice_count / 3;
    constexpr size_t float_per_vertex = 8;
    const size_t totalFloats = actual_vertex_count * float_per_vertex;
    m.vertices = (float *) malloc(totalFloats * sizeof(float));
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

    Vector_Mesh_Add(UnshadedMeshes, m);

    return currentId;
}


int Renderer_RegisterTexture(const char *path, Texture_Parameters parameters) {
    int width;
    int height;
    int nrChannels;

    // let's assume for now that we are using rgb
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    assert(("Texture path", data));
    assert(("Support only for RGBA", nrChannels == 3 || nrChannels == 4));

    const int id = Vector_Texture_Length(Textures);

    char *texture_path = (char *) malloc(strlen(path) + 1 * sizeof(char));
    strcpy(texture_path, path);

    TextureType loaded_format = nrChannels == 3 ? TextureType::RGB : TextureType::RGB_ALPHA;
    TextureType internal_format = loaded_format;
    if (parameters.convert_from_srgb_to_linear_space) {
        internal_format = loaded_format == TextureType::RGB ? TextureType::SRGB : TextureType::SRGB_ALPHA;
    }

    const Texture t{
        id,
        width,
        height,
        loaded_format,
        internal_format,
        data,
        texture_path,
        parameters.wrap_mode_s,
        parameters.wrap_mode_t
    };

    Vector_Texture_Add(Textures, t);

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
    assert(("Texture should be indexable", diffuse_texture_id < Vector_Texture_Length(Textures)));
    assert(("Texture should be indexable", specular_texture_id < Vector_Texture_Length(Textures)));
    assert(("Texture should be indexable", emission_texture_id < 0 || specular_texture_id <Vector_Texture_Length(Textures)));

    const int mesh_id = Vector_Mesh_Length(Meshes);
    Mesh m{};

    m.vertice_count = vertice_count;
    m.index_count = index_count;

    m.vertices = (float *) malloc(m.vertice_count * sizeof(float));
    m.indices = (uint32_t *) malloc(m.index_count * sizeof(uint32_t));

    memcpy(m.vertices, vertices, m.vertice_count * sizeof(float));
    memcpy(m.indices, indices, m.index_count * sizeof(uint32_t));

    m.id = mesh_id;
    m.diffuse_texture_id = diffuse_texture_id;
    m.specular_texture_id = specular_texture_id;
    m.emission_texture_id = emission_texture_id;
    m.program_id = world_geometry_program;
    m.outline_program_id = world_geometry_program_outlines;

    Vector_Mesh_Add(Meshes, m);

    return mesh_id;
}

int Renderer_RegisterTextured_Cross_Mesh(const int texture_id, const float scale) {
    assert(("Support only downsizing", scale<=1));

    constexpr int total_quads = 2;

    Mesh m{};
    m.vertice_count = quad::vertices_count_uv * total_quads;
    m.vertices = (float *) malloc(m.vertice_count * sizeof(float));

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
    m.indices = (uint32_t *) malloc(m.index_count * sizeof(uint32_t));

    constexpr int total_elements_for_1_quad = (quad::vertices_count_uv / stride);
    for (int quad_idx = 0; quad_idx < total_quads; ++quad_idx) {
        const int starting_point = quad_idx * quad::vertex_indices_count_uv;
        for (int i = 0; i < quad::vertex_indices_count_uv; ++i) {
            m.indices[starting_point + i] =
                    quad::vertex_indices_uvs[i] + quad_idx * total_elements_for_1_quad;
        }
    }

    const int currentId = Vector_Mesh_Length(Meshes);
    m.id = currentId;

    m.diffuse_texture_id = texture_id;
    m.program_id = world_geometry_program_cross_textures;
    m.outline_program_id = world_geometry_program_outlines;

    Vector_Mesh_Add(Meshes, m);

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
    char *directory = (char *) malloc((chars_to_copy + 1) * sizeof(char));
    memcpy(directory, path, chars_to_copy * sizeof(char));
    directory[chars_to_copy] = '\0';

    // keep this bastard queue for now
    // remove later because I hate STL
    Queue_aiNodePtr *nodes_to_process = Queue_aiNodePtr_Create(100);
    Queue_aiNodePtr_Enqueue(nodes_to_process, scene->mRootNode);


    const Model &model_to_register = {(int *) malloc(scene->mNumMeshes * sizeof(int)), scene->mNumMeshes};
    int count = 0;
    aiNodePtr node;

    Vector_uint32_t *index_data = Vector_uint32_t_Create(1024);
    Vector_float *vertex_data = Vector_float_Create(1024);

    while (Queue_aiNodePtr_Deque(nodes_to_process, &node)) {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

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

                Vector_float_Add(vertex_data, position.x);
                Vector_float_Add(vertex_data, position.z);
                Vector_float_Add(vertex_data, position.y);
                Vector_float_Add(vertex_data, normal.x);
                Vector_float_Add(vertex_data, normal.z);
                Vector_float_Add(vertex_data, normal.y);
                Vector_float_Add(vertex_data, uv_x);
                Vector_float_Add(vertex_data, uv_y);
            }


            for (size_t j = 0, indice_index = 0; j < mesh->mNumFaces; ++j) {
                const aiFace face = mesh->mFaces[j];
                for (unsigned int k = 0; k < face.mNumIndices; ++k, ++indice_index) {
                    Vector_uint32_t_Add(index_data, face.mIndices[k]);
                }
            }

            const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            const int diffuse_texture_id = LoadTexture(aiTextureType_DIFFUSE, directory, material);
            const int specular_texture_id = LoadTexture(aiTextureType_SPECULAR, directory, material);

            model_to_register.mesh_ids[count++] = Renderer_RegisterTexturedMesh(
                diffuse_texture_id,
                specular_texture_id,
                -1,
                vertex_data->data,
                Vector_float_Length(vertex_data),
                index_data->data,
                Vector_uint32_t_Length(index_data)
            );

            Vector_uint32_t_Clear(index_data);
            Vector_float_Clear(vertex_data);
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            Queue_aiNodePtr_Enqueue(nodes_to_process, node->mChildren[i]);
        }
    }

    Vector_uint32_t_Free(index_data);
    Vector_float_Free(vertex_data);

    Queue_aiNodePtr_Free(nodes_to_process);

    Vector_Model_Add(Models, model_to_register);

    const int model_id = Vector_Model_Length(Models);

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

// we can probably do some post-processing here if we want for the index sorting
void Renderer_FinalizeMeshLoading() {
    SendGeometryDataToTheGPU();
    SendLightGeometryDataToTheGPU();
    SendTextureDataToTheGPU();
    SendLightUBOsToTheGPU();
    SendScreenTextureDataToTheGPU();
}


// TODO we can probably work with a Matrix4D eventually
void Renderer_Draw(const int mesh_id, const Vector3D pos, const Vector3D color, const Material material) {
    // 1st stencil pass
    // write 1 to stencil buffer where fragments are drawn
    // for now assume everything has an outline
    // all fragments pass stencil test (still need to pass depth test)

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    const Mesh mesh = Meshes->data[mesh_id];
    Draw(mesh.program_id, mesh, pos, color, material);
}

void Renderer_Draw_Outline(int mesh_id, Vector3D pos, Vector3D color, Material material) {
    // now that we have written to the stencil buffer we need to draw an outline
    // therefore everywhere where stencil passed shouldn't be drawn
    // we also disable writing to the stencil buffer because we don't want outlines to write there
    // our shader will scale the model internally and discard the fragments we had at scale 1
    // we also disable depth test so the outline will be drawn on top
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);


    const Mesh mesh = Meshes->data[mesh_id];
    Draw(mesh.outline_program_id, mesh, pos, color, material);

    // reset to the previous state
    glStencilMask(0xFF);
    // no need to bother with stencil func itself since all draw calls set it up proplery
    //glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glEnable(GL_DEPTH_TEST);
}


void Renderer_Draw_Model(int model_id, Vector3D pos, Vector3D color, Material material) {
    const Model model = Models->data[model_id];
    for (size_t i = 0; i < model.mesh_count; ++i) {
        Renderer_Draw(model.mesh_ids[i], pos, color, material);
    }
}

void Renderer_Draw_Model_Outline(int model_id, Vector3D pos, Vector3D color, Material material) {
    const Model model = Models->data[model_id];
    for (size_t i = 0; i < model.mesh_count; ++i) {
        Renderer_Draw_Outline(model.mesh_ids[i], pos, color, material);
    }
}


void Renderer_DrawUnshadedTexture(const int light_id, const Vector3D pos, const Vector3D color) {
    // always pass the test so we can draw
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    // don't write anything to the stenci buffer though ;)
    glStencilMask(0x00);

    const Mesh light = UnshadedMeshes->data[light_id];
    const unsigned int program_to_use = light.program_id;
    glUseProgram(program_to_use);

    // this means all programs need this uniform
    const GLint voxel_color = glGetUniformLocation(program_to_use, "voxel_color");
    const GLint position_id = glGetUniformLocation(program_to_use, "position");
    const GLuint diffuse_texture_id = light.diffuse_texture_id == -1
                                          ? defaultTexture
                                          : Textures->data[light.diffuse_texture_id].texture_id;


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
    // point back to the default buffer
    if (Screen_Texture.Samples > 0) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, Screen_Texture.frame_buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Screen_Texture.frame_buffer_multisample_result);
        glBlitFramebuffer(0, 0, Screen_Texture.Internal_Width, Screen_Texture.Internal_Height, 0, 0, Screen_Texture.Internal_Width,
                          Screen_Texture.Internal_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Screen_Texture.Width, Screen_Texture.Height);

    // clear the color of the screen buffer
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    // since we are only drawin on the screen we don't need any of those things so we disable them
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);


    //draw the quad here
    glUseProgram(Screen_Texture.screen_texture_program);
    glBindVertexArray(Screen_Texture.VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Screen_Texture.screen_texture);
    glDrawElements(GL_TRIANGLES, static_cast<int>(quad::vertex_indices_count_uv_single_faced),GL_UNSIGNED_INT, nullptr);

    // restore the flags now that we are done drawing on the screen
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);

    SDL_GL_SwapWindow(window);
}


void Renderer_Destroy() {
    for (size_t i = 0; i < Vector_Mesh_Length(Meshes); ++i) {
        const Mesh m = Meshes->data[i];
        glDeleteVertexArrays(1, &m.VAO);
        glDeleteBuffers(1, &m.VBO);
        glDeleteBuffers(1, &m.VBE);
    }

    for (size_t i = 0; i < Vector_Mesh_Length(UnshadedMeshes); ++i) {
        const Mesh m = UnshadedMeshes->data[i];
        glDeleteVertexArrays(1, &m.VAO);
        glDeleteBuffers(1, &m.VBO);
        glDeleteBuffers(1, &m.VBE);
    }

    for (size_t i = 0; i < Vector_Model_Length(Models); ++i) {
        const Model m = Models->data[i];
        free(m.mesh_ids);
    }

    glDeleteBuffers(1, &ViewMatricesBlock);
    glDeleteFramebuffers(1, &Screen_Texture.frame_buffer);
    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(open_gl_context);
    SDL_Quit();

    Vector_Mesh_Free(Meshes);
    Vector_Mesh_Free(UnshadedMeshes);
    Vector_Model_Free(Models);
    Vector_Texture_Free(Textures);
}


void Renderer_ResolutionChanged(const int new_screen_width, const int new_screen_height) {
    auto m = PerspectiveMatrix(ProjectionParams.FOV, ProjectionParams.Z_near, ProjectionParams.Z_far,
                               static_cast<float>(new_screen_width) / static_cast<float>(new_screen_height));

    glBindBuffer(GL_UNIFORM_BUFFER, ViewMatricesBlock);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4D), &m[0].x);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    Screen_Texture.Height = new_screen_height;
    Screen_Texture.Width = new_screen_width;
}

void Renderer_CameraUpdate() {
    auto m = CameraLookAtMatrix(*SceneCamera);
    glBindBuffer(GL_UNIFORM_BUFFER, ViewMatricesBlock);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Matrix4D), sizeof(Matrix4D), &m[0].x);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer_Change_Emission(const int mesh_id, const int emission_texture_id) {
    Meshes->data[mesh_id].emission_texture_id = emission_texture_id;
}

void SendLightUBOsToTheGPU() {
    glBindBuffer(GL_UNIFORM_BUFFER, Point_Lights_Block);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Point_Lights), &Point_Lights);

    glBindBuffer(GL_UNIFORM_BUFFER, Directional_Lights_Block);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Directional_Lights), &Directional_Lights);


    glBindBuffer(GL_UNIFORM_BUFFER, Spot_Lights_Block);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Spot_Lights), &Spot_Lights);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// this is for untextured meshes ;)
void SendGeometryDataToTheGPU() {
    for (int i = 0; i < Vector_Mesh_Length(Meshes); ++i) {
        auto &m = Meshes->data[i];

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

        free(m.vertices);
        m.vertices = nullptr;

        free(m.indices);
        m.indices = nullptr;
    }
}

// this is for untextured meshes ;)
void SendLightGeometryDataToTheGPU() {
    for (int i = 0; i < Vector_Mesh_Length(UnshadedMeshes); ++i) {
        auto &light = UnshadedMeshes->data[i];
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
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              float_per_vertex * sizeof(float),
                              reinterpret_cast<void *>(5 * sizeof(float))
        );

        glGenBuffers(1, &light.VBE);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light.VBE);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * light.index_count, light.indices, GL_STATIC_DRAW);

        free(light.vertices);
        light.vertices = nullptr;

        free(light.indices);
        light.indices = nullptr;
    }
}

void SendTextureDataToTheGPU() {
    for (size_t i = 0; i < Vector_Texture_Length(Textures); ++i) {
        Texture &t = Textures->data[i];
        glGenTextures(1, &t.texture_id);
        glBindTexture(GL_TEXTURE_2D, t.texture_id);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(t.WrapMode_S));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(t.WrapMode_T));
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        const int format = t.loaded_texture_type == TextureType::RGB ? GL_RGB : GL_RGBA;

        int internal_format = 0;
        switch (t.stored_texture_storage) {
            case TextureType::SRGB_ALPHA:
                internal_format = GL_SRGB8_ALPHA8;
                break;
            case TextureType::SRGB:
                internal_format = GL_SRGB8;
                break;
            case TextureType::RGB:
                internal_format = GL_RGB;
                break;
            case TextureType::RGB_ALPHA:
                internal_format = GL_RGBA;
                break;
        }

        assert(internal_format!=0);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, t.width, t.height, 0, format, GL_UNSIGNED_BYTE, t.data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(t.data);
        t.data = nullptr;
    }
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

    char *relative_path = (char *) malloc((dir_name_len + file_name_len + 2) * sizeof(char));
    if (dir_name_len == 0) {
        strcpy(relative_path, file_name);
    } else {
        strcpy(relative_path, directory);
        strcat(relative_path, "/");
        strcat(relative_path, file_name);
    }

    const int texture_id = Renderer_RegisterTexture(relative_path);
    free(relative_path);

    return texture_id;
}

void Draw(const unsigned int program_to_use, const Mesh &mesh, const Vector3D pos, const Vector3D color,
          const Material material) {
    glUseProgram(program_to_use);

    // this means all programs need this uniform
    const GLint voxel_color = glGetUniformLocation(program_to_use, "voxel_color");
    const GLint position_id = glGetUniformLocation(program_to_use, "position");
    const GLint view_pos_id = glGetUniformLocation(program_to_use, "view_position");
    const GLuint diffuse_texture_id = mesh.diffuse_texture_id == -1
                                          ? defaultTexture
                                          : Textures->data[mesh.diffuse_texture_id].texture_id;
    const GLuint specular_texture_id = mesh.specular_texture_id == -1
                                           ? defaultEmissionTexture
                                           : Textures->data[mesh.specular_texture_id].texture_id;

    const GLuint emission_texture_id = mesh.emission_texture_id == -1
                                           ? defaultEmissionTexture
                                           : Textures->data[mesh.emission_texture_id].texture_id;

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

void SendScreenTextureDataToTheGPU() {
    glGenVertexArrays(1, &Screen_Texture.VAO);
    glBindVertexArray(Screen_Texture.VAO);


    glGenBuffers(1, &Screen_Texture.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Screen_Texture.VBO);

    constexpr auto vertice_count = quad::vertices_count_uv_single_faced_ndc;
    constexpr auto vertices = quad::vertex_data_uv_1_part_texture_single_faced_ndc;
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertice_count, vertices, GL_STATIC_DRAW);

    constexpr int float_per_vertex = 5;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, float_per_vertex * sizeof(float), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, float_per_vertex * sizeof(float),
                          reinterpret_cast<void *>(3 * sizeof(float)));

    constexpr auto indices = quad::vertex_indices_uvs_single_faced_ndc;
    constexpr auto indices_count = quad::vertex_indices_count_uv_single_faced_ndc;

    Screen_Texture.IndexCount = indices_count;

    glGenBuffers(1, &Screen_Texture.VBE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Screen_Texture.VBE);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices_count, indices, GL_STATIC_DRAW);
}
