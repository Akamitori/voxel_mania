#ifndef RENDERER_H
#define RENDERER_H

#include <cassert>

#include "Vector3D.h"
#include "export.h"
#include "SDL3/SDL_video.h"
#include <cstdint>
#include <cstdio>
#include "RendererTypes.h"

struct Matrix4D;
struct Vector4D;
struct Camera;

extern EXPORTED Camera *SceneCamera;
extern EXPORTED SDL_Window *window;
extern EXPORTED SDL_GLContext open_gl_context;

EXPORTED void Renderer_Init(int screen_width, int screen_height, float fov, float z_near, float z_far, int anti_aliasing_samples = 0);

// this API uses just positions
// we can update it to reflect that!
EXPORTED int Renderer_RegisterPrimitiveMeshData(
    const float *vertices,
    size_t vertice_count,
    const uint32_t *indices,
    size_t index_count
);

// this API uses just positiosn and normals
// we can update it to reflect that!
EXPORTED int Renderer_RegisterUnshadedTexture(
    const float *vertices,
    size_t vertice_count,
    const uint32_t *indices,
    size_t index_count
);

EXPORTED int Renderer_RegisterTextureFromPath(
    const char *path,
    Texture_Parameters texture_parameters = {}
);

// this is the full deal
// so we can update the input to reflect this
// work for later but it would be nice i guess
EXPORTED int Renderer_RegisterTexturedMesh(
    int diffuse_texture_id,
    int specular_texture_id,
    int emission_texture_id,
    const float *vertices,
    size_t vertice_count,
    const uint32_t *indices,
    size_t index_count,
    const VertexBoneData* vertex_bone_data,
    size_t vertex_bone_data_count
);

EXPORTED int Renderer_RegisterTextured_Cross_Mesh(int texture_id, float scale = 1);

EXPORTED int Renderer_Register_Model(const char *path);

EXPORTED int Renderer_Register_Directional_Light(const DirectionalLight &light);

EXPORTED int Renderer_Register_Point_Light(const PointLight &light);

EXPORTED int Renderer_Register_Spot_Light(const SpotLight &light);

EXPORTED void Renderer_FinalizeMeshLoading();

EXPORTED void Renderer_Destroy();

EXPORTED void Renderer_FrameStart();

EXPORTED void Renderer_ResolveDrawCalls(double delta_in_ms);

EXPORTED void Renderer_FrameEnd();

EXPORTED void Renderer_Draw_Mesh(int mesh_id, const Transform &transform, Vector3D color, Material material);

EXPORTED void Renderer_Draw_Model(int model_id, const Transform &transform, Vector3D color, Material material);

EXPORTED void Renderer_Draw_Skeletal_Model(int model_id, const Transform &transform, Vector3D color, Material material);

EXPORTED void Renderer_Draw_Mesh_Unshaded(int mesh_id, const Transform &transform, Vector3D color);

EXPORTED void Renderer_ResolutionChanged(int new_screen_width, int new_screen_height);

EXPORTED void Renderer_CameraUpdate();

EXPORTED void Renderer_Change_Emission(int mesh_id, int emission_texture_id);

EXPORTED void Mess_With_The_Bones();

EXPORTED void Renderer_Draw_Gizmo(const Vector3D &world_position);

EXPORTED void Renderer_SetGizmoPosition(Vector3D);

#endif //RENDERER_H
