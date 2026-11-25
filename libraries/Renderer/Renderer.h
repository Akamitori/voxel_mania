#ifndef RENDERER_H
#define RENDERER_H

#include "Vector3D.h"
#include "renderer_export.h"
#include "SDL3/SDL_video.h"
#include <cstdint>
#include <cstdio>
#include "RendererTypes.h"

struct Matrix4D;
struct Vector4D;
struct Camera;

extern RENDERER_EXPORT Camera *SceneCamera;
extern RENDERER_EXPORT SDL_Window *window;
extern RENDERER_EXPORT SDL_GLContext open_gl_context;

RENDERER_EXPORT void Renderer_Init(int screen_width, int screen_height, float fov, float z_near, float z_far, int anti_aliasing_samples = 0);

// this API uses just positions
// we can update it to reflect that!
RENDERER_EXPORT int Renderer_RegisterPrimitiveMeshData(
    const float *vertices,
    size_t vertice_count,
    const uint32_t *indices,
    size_t index_count
);

// this API uses just positiosn and normals
// we can update it to reflect that!
RENDERER_EXPORT int Renderer_RegisterUnshadedTexture(
    const float *vertices,
    size_t vertice_count,
    const uint32_t *indices,
    size_t index_count
);

RENDERER_EXPORT int Renderer_RegisterTextureFromPath(
    const char *path,
    Texture_Parameters texture_parameters = {}
);

// this is the full deal
// so we can update the input to reflect this
// work for later but it would be nice i guess
RENDERER_EXPORT int Renderer_RegisterTexturedMesh(
    int diffuse_texture_id,
    int specular_texture_id,
    int emission_texture_id,
    const float *vertices,
    size_t vertice_count,
    const uint32_t *indices,
    size_t index_count
);

RENDERER_EXPORT int Renderer_RegisterTextured_Cross_Mesh(int texture_id, float scale = 1);

RENDERER_EXPORT int Renderer_Register_Model(const char *path);

RENDERER_EXPORT int Renderer_Register_Directional_Light(const DirectionalLight &light);

RENDERER_EXPORT int Renderer_Register_Point_Light(const PointLight &light);

RENDERER_EXPORT int Renderer_Register_Spot_Light(const SpotLight &light);

RENDERER_EXPORT void Renderer_FinalizeMeshLoading();

RENDERER_EXPORT void Renderer_Destroy();

RENDERER_EXPORT void Renderer_FrameStart();

RENDERER_EXPORT void Renderer_ResolveDrawCalls(double delta_in_ms);

RENDERER_EXPORT void Renderer_FrameEnd();

RENDERER_EXPORT void Renderer_Draw_Mesh(int mesh_id, const Transform &transform, Vector3D color, Material material);

RENDERER_EXPORT void Renderer_Draw_Model(int model_id, const Transform &transform, Vector3D color, Material material);

RENDERER_EXPORT void Renderer_Draw_Skeletal_Model(int model_id, const Transform &transform, Vector3D color, Material material);

RENDERER_EXPORT void Renderer_Draw_Mesh_Unshaded(int mesh_id, const Transform &transform, Vector3D color);

RENDERER_EXPORT void Renderer_ResolutionChanged(int new_screen_width, int new_screen_height);

RENDERER_EXPORT void Renderer_CameraUpdate();

RENDERER_EXPORT void Renderer_Change_Emission(int mesh_id, int emission_texture_id);

RENDERER_EXPORT void DEBUG_SendContourDataToTheGPU(const Line2D *lines, size_t count);

RENDERER_EXPORT void DEBUG_SendPointDataToTheGPU(const Vector3D *points, const float * densities, size_t count);

RENDERER_EXPORT void DEBUG_SendGridDataToTheGPU(const Vector3D *grid_vertices, size_t count);

RENDERER_EXPORT void DEBUG_SendCircleCursorDataToTheGPU(const Vector3D *circle_points, size_t point_count, Vector3D color);

RENDERER_EXPORT void DEBUG_Update_Brush_Trans(Vector3D trans);


RENDERER_EXPORT void DEBUG_Draw_Points();

RENDERER_EXPORT void DEBUG_Modify_Slice_Layer(int value);

#endif //RENDERER_H
