#ifndef RENDERER_H
#define RENDERER_H

#include "Vector3D.h"
#include "export.h"
#include "SDL3/SDL_video.h"

struct Matrix4D;
struct Vector4D;
struct Camera;



extern EXPORTED Camera *SceneCamera;
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


EXPORTED void Renderer_Init(int screen_width, int screen_height, float fov, float z_near, float z_far);

EXPORTED int Renderer_RegisterPrimitiveMeshData(
    const float *vertices,
    size_t vertice_count,
    const int *indices,
    size_t index_count
);

EXPORTED int Renderer_RegisterUnshadedTexture(
    const float *vertices,
    size_t vertice_count,
    const int *indices,
    size_t index_count
);

EXPORTED int Renderer_RegisterTexture(const char* path);

EXPORTED int Renderer_RegisterTexturedMesh(
    int diffuse_texture_id,
    int specular_texture_id,
    int emission_texture_id,
    const float *vertices,
    size_t vertice_count,
    const int *indices,
    size_t index_count
);

EXPORTED int Renderer_RegisterTextured_Cross_Mesh(int texture_id, float scale = 1);

EXPORTED int Renderer_Register_Directional_Light(const DirectionalLight &light);

EXPORTED int Renderer_Register_Point_Light(const PointLight &light);

EXPORTED int Renderer_Register_Spot_Light(const SpotLight &light);

EXPORTED void Renderer_FinalizeMeshLoading();

EXPORTED void Renderer_Destroy();

EXPORTED void Renderer_FrameStart();

EXPORTED void Renderer_FrameEnd();

EXPORTED void Renderer_Draw(int mesh_id, Vector3D pos, Vector3D color, Material material);

EXPORTED void Renderer_DrawUnshadedTexture(int light_id, Vector3D pos, Vector3D color);

EXPORTED void Renderer_ResolutionChanged(int new_screen_width, int new_screen_height);

EXPORTED void Renderer_CameraUpdate();

EXPORTED void Renderer_Change_Emission(int mesh_id, int emission_texture_id);


#endif //RENDERER_H
