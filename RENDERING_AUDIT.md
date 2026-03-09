# Rendering Pipeline Audit

Snapshot taken from branch `pr/depth_test_revisited`.

---

## Voxel Mesh Generation & Draw Calls

**Approach: fully naive, one draw call per object.**

- Hardcoded unit-cube geometry in `primitive_mesh_data/cube.h` (24 verts, 36 indices, 12 tris).
  Two UV layouts: 3-part atlas (grass top/side/bottom) and 1-part (uniform face).
- Vertex format: `[pos.xyz, normal.xyz, uv.xy]` = 8 floats, 32-byte stride (`Renderer.cpp:1141`).
- Vegetation uses an X-shaped cross-mesh (two rotated quads) registered via `Renderer_RegisterTextured_Cross_Mesh` (`Renderer.cpp:691-787`).
- Each mesh drawn individually with `glDrawElements` (`Renderer.cpp:1304`). Main loop in `main.cpp:370-386` iterates per-object for opaques, outlines, then transparents.
- No greedy meshing, no chunk batching, no instanced rendering, no indirect draw.
- Assimp model loader exists (`Renderer.cpp:790-902`) but is commented out.

## Shader Pipeline

OpenGL 4.5 core, GLSL 3.30. Six programs, no geometry/compute shaders.

| Program | Folder | Purpose |
|---------|--------|---------|
| `program_for_regular_textures` | `data/program_for_regular_textures/` | Main geometry + Phong lighting |
| `program_for_regular_texture_outlines` | `data/program_for_regular_texture_outlines/` | Stencil outline pass (solid teal, 1.1x scale) |
| `program_for_transparent_cross_textures` | `data/program_for_transparent_cross_textures/` | Vegetation with alpha-test discard (`a < 1`) |
| `program_for_unshaded_textures` | `data/program_for_unshaded_textures/` | Unlit light-source cubes |
| `program_for_screen_texture` | `data/program_for_screen_texture/` | Post-process blit (currently passthrough) |
| `cursor_program` | `data/cursor_program/` | Crosshair overlay (flat red) |

Post-processing shader has commented-out kernels (sharpen, inversion, grayscale) at `program_for_screen_texture/frag.frag:16-52`.

## Lighting

**Phong reflection model** (not Blinn-Phong). Three light types via UBOs (std140):

| Type | UBO Binding | Max Count | Key Properties |
|------|-------------|-----------|----------------|
| Point | 1 | 4 | position, ambient/diffuse/specular, linear+quadratic attenuation |
| Directional | 2 | 4 | direction, ambient/diffuse/specular, no attenuation |
| Spot | 3 | 4 | position+direction, attenuation, inner/outer cutoff with smooth falloff |

Material system: diffuse (unit 0), specular (unit 1), emission (unit 2) samplers + shininess float.
Default textures: 1x1 white for missing diffuse/specular, 1x1 black for missing emission (`Renderer.cpp:198-215`).

**No shadows of any kind.**

Normal is passed from vertex shader without inverse-transpose correction (`vert.vert:31`) -- will break with non-uniform scaling.

## Anti-Aliasing

MSAA infrastructure is built out (`Renderer.cpp:241-244, 384, 392`) with multisample FBO + resolve blit (`Renderer.cpp:1024-1027`), but **currently disabled** -- `Renderer_Init` is called with `anti_aliasing_samples` defaulting to 0 (`main.cpp:158`).

No FXAA, TAA, or SMAA.

## Texture Handling

- Loaded via stb_image, flipped on load (`Renderer.cpp:627`).
- Filtering: `GL_NEAREST` mag + `GL_NEAREST_MIPMAP_NEAREST` min (`Renderer.cpp:1211-1212`) -- intentionally pixelated.
- Mipmaps generated via `glGenerateMipmap` (`Renderer.cpp:1216`).
- Wrap: `GL_REPEAT` default, `GL_CLAMP_TO_EDGE` for transparent windows.
- No texture atlas system -- each block type is a separate texture. The 3-part UV layout in `cube.h` is a manual per-texture atlas for grass faces.
- No texture arrays or 3D textures.

## Reverse Z-Mapping

**Fully implemented** (recently fixed in `b27df82`):

1. `glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE)` -- `Renderer.cpp:295`
2. `glDepthFunc(GL_GEQUAL)` -- `Renderer.cpp:290`
3. Custom perspective matrix mapping near->1.0, far->0.0 -- `Camera.cpp:92-109`
4. `glClearDepth(0.0f)` -- `Renderer.cpp:467`

Near=0.1, far=100. Depth buffer is `GL_DEPTH24_STENCIL8` (24-bit depth + 8-bit stencil).

## Gamma Correction

**Completely absent.**

- Textures uploaded as `GL_RGB`/`GL_RGBA`, not `GL_SRGB`/`GL_SRGB_ALPHA` (`Renderer.cpp:1215`).
- Framebuffers are `GL_RGB` (`Renderer.cpp:384, 412, 430`), not `GL_SRGB8`.
- `GL_FRAMEBUFFER_SRGB` is never enabled.
- No manual gamma operations (`pow 2.2` / `pow 1/2.2`) in any shader.
- All lighting math runs in sRGB space -- physically incorrect. Expect overly dark midtones and wrong falloff curves.

## Other Notable State

| Feature | Status |
|---------|--------|
| Off-screen FBO | Scene rendered at fixed 800x600, blitted to display resolution |
| Transparency | Back-to-front sorted, standard alpha blend (`GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`) |
| Stencil outlines | Two-pass stencil technique, depth-test disabled for outline pass |
| Back-face culling | Enabled, CCW winding |
| VSync | On (`SDL_GL_SetSwapInterval(1)`) |
| Frustum culling | Code exists (`main.cpp:61-87`) but **commented out** |
| Dear ImGui | Integrated but rendering call commented out |
| Perlin noise | FBM-capable implementation exists, not wired to terrain gen |

---

## Prioritized Next Steps

### Quick Wins (low effort, high impact)

1. **Gamma correction** -- Easiest path: upload textures as `GL_SRGB8_ALPHA8`, enable `GL_FRAMEBUFFER_SRGB`, done. Lighting will immediately look more correct.
2. **Enable MSAA** -- Infrastructure already built. Pass a non-zero sample count (4 or 8) to `Renderer_Init`.
3. **Uncomment frustum culling** -- Code is already written in `main.cpp:61-87`. Free perf.
4. **Fix normal transform** -- Use `transpose(inverse(mat3(model)))` in the vertex shader. Currently broken for non-uniform scales.

### Medium Effort

5. **Blinn-Phong** -- Replace `reflect()` + `pow(dot(viewDir, reflectDir))` with `pow(dot(normal, halfDir))`. Better specular highlights, ~5 lines changed per shader.
6. **Texture atlas / texture array** -- Merge block textures into a `GL_TEXTURE_2D_ARRAY`. Eliminates texture bind overhead and is a prerequisite for batching.
7. **Post-processing pass** -- The blit shader is ready for gamma, tone mapping, or FXAA. Add at least a simple Reinhard or ACES tonemap if you ever move to HDR.

### Larger Efforts (architecture)

8. **Chunk-based batching + greedy meshing** -- The single biggest perf win. Build chunks, merge coplanar faces, one draw call per chunk.
9. **Instanced rendering** -- Intermediate step if greedy meshing is too much. Use `glDrawElementsInstanced` with per-instance position/texture-id.
10. **Shadow mapping** -- Directional light shadow map is the natural next lighting feature.
11. **HDR framebuffers** -- Switch to `GL_RGBA16F` for the scene FBO, add tonemapping in the post-process pass.
