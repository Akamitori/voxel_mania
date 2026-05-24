#ifndef RENDERER_H
#define RENDERER_H

#include "Vector3D.h"
#include "export.h"
#include "SDL3/SDL_video.h"
#include <cstdint>

#include "Matrix4D.h"
#include "GL/glew.h"

struct Matrix4D;
struct Vector4D;
struct Camera;

extern EXPORTED Camera *MainCamera;
extern EXPORTED Camera *ObserverCamera;
extern EXPORTED SDL_Window *window;
extern EXPORTED SDL_GLContext open_gl_context;


struct EXPORTED Material {
    float shininess;
};

struct EXPORTED PointLight {
    alignas(16)Vector3D position;
    alignas(16)Vector3D ambient;
    alignas(16)Vector3D diffuse;
    alignas(16)Vector3D specular;
    float linear_factor;
    float quadric_factor;
};

struct EXPORTED DirectionalLight {
    alignas(16)Vector3D direction;
    alignas(16)Vector3D ambient;
    alignas(16)Vector3D diffuse;
    alignas(16)Vector3D specular;
};

struct EXPORTED SpotLight {
    alignas(16)Vector3D position;
    alignas(16)Vector3D direction;
    alignas(16)Vector3D ambient;
    alignas(16)Vector3D diffuse;
    alignas(16)Vector3D specular;
    float linear_factor;
    float quadric_factor;
    float inner_cutOff;
    float outer_cutOff;
};

enum class TextureWrapMode : uint32_t {
    REPEAT = GL_REPEAT,
    CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE
};

 struct EXPORTED Texture_Parameters {
    TextureWrapMode wrap_mode_s = TextureWrapMode::REPEAT;
    TextureWrapMode wrap_mode_t = TextureWrapMode::REPEAT;
    bool convert_from_srgb_to_linear_space = false;
};

struct EXPORTED Transform {
    Vector3D Position{0, 0, 0};
    Vector3D Rotation{0, 0, 0};
    Vector3D Scale{1, 1, 1};
};

struct EXPORTED debug_data {
    Matrix4D camera_space{};
    Vector3D vertices_camera_space[8*4];
    Vector3D vertices_light_space[8*4];
    Vector3D vertices_world_space[8*4];
    Vector3D light_camera_s_k[4];
    Vector3D bb_min_light_space[4];
    Vector3D bb_max_light_space[4];
    float diameter[4];
}; 

EXPORTED debug_data Renderer_Get_Debug_Data();
EXPORTED void Renderer_Init(int screen_width, int screen_height, float fov, float z_near, float z_far, int anti_aliasing_samples = 0);

EXPORTED int Renderer_RegisterPrimitiveMeshData(
    const float *vertices,
    size_t vertice_count,
    const uint32_t *indices,
    size_t index_count
);

EXPORTED int Renderer_RegisterUnshadedTexture(
    const float *vertices,
    size_t vertice_count,
    const uint32_t *indices,
    size_t index_count
);

EXPORTED int Renderer_RegisterTexture(
    const char *path,
    Texture_Parameters texture_parameters = {}
);

EXPORTED int Renderer_RegisterTexturedMesh(
    int diffuse_texture_id,
    int specular_texture_id,
    int emission_texture_id,
    const float *vertices,
    size_t vertice_count,
    const uint32_t *indices,
    size_t index_count
);


EXPORTED int Renderer_RegisterTextured_Cross_Mesh(int texture_id, float scale = 1);

EXPORTED int Renderer_Register_Model(const char *path);

EXPORTED int Renderer_Register_Directional_Light(const DirectionalLight &light);

EXPORTED int Renderer_Register_Point_Light(const PointLight &light);

EXPORTED int Renderer_Register_Spot_Light(const SpotLight &light);

EXPORTED void Renderer_FinalizeMeshLoading();

EXPORTED void Renderer_Destroy();

EXPORTED void Renderer_FrameStart();

EXPORTED void Renderer_ResolveDrawCalls();

EXPORTED void Renderer_FrameEnd();

EXPORTED void Renderer_Draw_Mesh(int mesh_id, const Transform& transform, Vector3D color, Material material);

EXPORTED void Renderer_Draw_Model(int model_id, const Transform& transform, Vector3D color, Material material);

EXPORTED void Renderer_Draw_Mesh_Unshaded(int mesh_id, const Transform& transform, Vector3D color);

EXPORTED void Renderer_ResolutionChanged(int new_screen_width, int new_screen_height);

EXPORTED void Renderer_CameraUpdate();

EXPORTED void Renderer_Change_Emission(int mesh_id, int emission_texture_id);

EXPORTED void Renderer_Toggle_Shadow_Map_Rendering(int layer);

EXPORTED void Renderer_Align_Camera_With_Light();

EXPORTED void Renderer_Toggle_PCF();

#endif //RENDERER_H
