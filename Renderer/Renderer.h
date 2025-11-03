#ifndef RENDERER_H
#define RENDERER_H
#include <string>

#include "Vector3D.h"
#include "SDL3/SDL_video.h"


struct Matrix4D;
struct Vector4D;
struct Camera;


inline SDL_Window *window;
inline SDL_GLContext open_gl_context;
inline Camera *SceneCamera;


struct Material {
    Vector3D ambient;
    Vector3D diffuse;
    Vector3D specular;
    float shininess;
};

struct Light {
    Vector3D position;
    
    Vector3D ambient;
    Vector3D diffuse;
    Vector3D specular;
};


void Renderer_Init(int screen_width, int screen_height, float fov, float z_near, float z_far);

int Renderer_RegisterPrimitiveMeshData(
    const float *vertices,
    size_t vertice_count,
    const int *indices,
    size_t index_count
);

int Renderer_RegisterLight(
    const float *vertices,
    size_t vertice_count,
    const int *indices,
    size_t index_count
);

int Renderer_RegisterTexture(const std::string &path);

int Renderer_RegisterTexturedMesh(
    int texture_id,
    const float *vertices,
    size_t vertice_count,
    const int *indices,
    size_t index_count
);

int Renderer_RegisterTextured_Cross_Mesh(int texture_id, float scale = 1);

void Renderer_FinalizeMeshLoading();

void Renderer_Destroy();

void Renderer_FrameStart();

void Renderer_FrameEnd();

void Renderer_Draw(int mesh_id, Vector3D pos, Vector3D color, Light light, Material material);

void Renderer_DrawLight(int light_id, Vector3D pos, Vector3D color);

void Renderer_ResolutionChanged(int new_screen_width, int new_screen_height);

void Renderer_CameraUpdate();


#endif //RENDERER_H
