# Voxel Mania

A voxel rendering engine built from scratch in C++ and OpenGL. Learning graphics programming by implementing everything from first principles.

## Current Features

- **Custom renderer** - OpenGL 3.3 core with UBO-based shader management
- **Phong lighting** - Point lights, spotlights, and directional lights with proper attenuation
- **Material system** - Diffuse, specular, and emission texture support with shininess control
- **Camera system** - Free-look camera with proper matrix transformations
- **Voxel rendering** - Textured cubes and cross-mesh vegetation

## Controls

- **WASD** - Move camera
- **Arrow keys** - Rotate camera

## Why This Project

Building a renderer from scratch to understand the graphics pipeline without black-box abstractions. Every matrix multiplication, every shader uniform, every rendering decision is intentional and understood.

## Build

Requires: OpenGL 3.3+, SDL3, GLEW, GLM, Dear ImGui
```bash
mkdir build && cd build
cmake ..
make
./voxel_mania
```

## What's Next

- Model loading (OBJ/glTF via Assimp)
- Shadow mapping
- Chunk-based world management
- Procedural terrain generation

## Technical Notes

The codebase follows a "minimal C++" approach - treating C++ as "C with classes" to understand memory layout and avoid unnecessary complexity during the learning phase. The renderer is structured as a proper library with clean API boundaries.

---

*Personal learning project exploring graphics programming fundamentals.*