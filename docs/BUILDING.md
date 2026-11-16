# Building Voxel Mania

All third-party code (SDL3, assimp, GLEW, Dear ImGui) comes in as git submodules and is
built as part of the project. You only need a compiler toolchain, CMake and Ninja.

## Requirements

| | Linux | Windows |
|---|---|---|
| Compiler | clang or gcc with C++23 (tested: clang 22, gcc 16) | MSVC from Visual Studio 2022 (tested: 19.44) |
| CMake | 3.30+ | 3.30+ (the one bundled with CLion works) |
| Generator | Ninja | Ninja (bundled with CLion and Visual Studio) |
| Other | git; OpenGL, GLX and X11 development packages (GLEW needs them) | git |
| Optional | `mold` (faster linking, used automatically) | RAD linker (`radlink.exe`, used automatically in Debug/RelWithDebInfo) |
| Optional | Doxygen (for the `assimp_docs` target) | Doxygen |

Only **single-config** generators are supported (Ninja, Makefiles). Visual Studio solution
generators and Ninja Multi-Config stop configure with an error that explains what to do
instead. Each build type gets its own build directory.

## Building

### Command line

```bash
git clone <repo url> voxel_mania && cd voxel_mania      # submodules are initialized on first configure
cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
cd build-debug && ./voxel_mania                          # Windows: voxel_mania.exe
```

On Windows run these from an **x64 Native Tools / Developer Command Prompt** so MSVC is on the
path. For a release build use another directory: `-B build-release -DCMAKE_BUILD_TYPE=Release`.
If you leave out `CMAKE_BUILD_TYPE`, it defaults to Debug.

The game loads `data/` relative to the **working directory**, so start it from the directory
that contains it (the build directory, or the install directory).

### CLion

Open the repository folder. The default CMake profiles (Ninja, Debug/Release) work as they are.
On Linux, point the toolchain at `/usr/bin/clang` / `/usr/bin/clang++` (not `/bin/...`) to
avoid needless rebuilds; see *Troubleshooting*.

## Options

Pass with `-D<NAME>=<value>` at configure time.

| Option | Default | What it does |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Debug` | `Debug`, `Release`, `RelWithDebInfo` or `MinSizeRel` |
| `USE_MOLD` | `ON` | Linux: link everything with mold when it is installed |
| `USE_RADLINK` | `ON` | MSVC, Debug/RelWithDebInfo only: link with the RAD linker when found (on `PATH` or in `C:/rad_linker`; `-DRADLINK_EXECUTABLE=<path>` points elsewhere) |
| `VOXEL_WARNINGS` | `ON` | `-Wall -Wextra` / `/W4` for the project's own targets (not third-party code). Warnings only, not errors |
| `ENABLE_ASAN` | `OFF` | AddressSanitizer build, for finding memory errors. Use a separate build directory. On Windows run it from a developer prompt (the ASan runtime DLL is not copied next to the exe) |
| `VOXEL_SYNC_SUBMODULES` | `OFF` | Force `git submodule update --init --recursive` on every configure |

The chosen linker applies to the whole build, SDL3 and assimp included. Release builds on
Windows always use the standard MSVC linker.

## Installing

```bash
cmake --install build-release --prefix <dir>
```

Gives a self-contained, relocatable folder:

| Linux | Windows |
|---|---|
| `<dir>/voxel_mania` | `<dir>\voxel_mania.exe` |
| `<dir>/lib/*.so` (our libraries, GLEW, SDL3, assimp) | `<dir>\*.dll` next to the exe |
| `<dir>/data/` | `<dir>\data\` |

On Linux the executable and libraries find each other through `$ORIGIN`-relative RPATHs, so
the folder can be moved or copied to another machine (system libraries like OpenGL and X11 are
not bundled).

## How the third-party libraries are built

### SDL3 and assimp: the `.prebuild_libs` cache

SDL3 and assimp are large and rarely change, so they are built once per *setup* and cached in
`.prebuild_libs/<lib>/<BuildType>-<Compiler>-<hash>/` in the source tree (git-ignored). The hash
covers the submodule commit, OS, compiler path and version, build type, CMake arguments and
linker. Different compilers, build types and build directories therefore share the cache
without overwriting each other.

- A build directory that finds a complete cache for its setup just reuses it.
- Each build directory gets its own copy of the libraries in `<build>/prebuilt/`. That copy is
  what gets linked and deployed, so *Clean* / *Rebuild* never touches the shared cache.
- Deleting `.prebuild_libs` (all of it or one folder) is always safe: the next build
  re-runs CMake and rebuilds what is missing.
- `cmake --build <dir> --target prune_prebuilt_libs` deletes cache folders built from submodule
  commits that are no longer in use. Folders for other compilers or build types at the current
  commits are kept (other build directories may use them); delete those by hand if unused.

Why a cache instead of building SDL3/assimp in-tree with ccache (measured on Linux, 32 cores,
clang, mold, Debug):

| | Cache | In-tree + ccache |
|---|---|---|
| Cold first build | 28 s | 34 s |
| New build directory, everything warm | **3 s** | 19 s (17 s of it is SDL3/assimp's configure) |
| Clean + rebuild | 2 s | 1.5 s |
| Reconfigure (e.g. CLion reload) | **0.15 s** | 0.9 s |

GLEW and Dear ImGui are small and built in-tree normally.

### Submodules

Configure keeps the submodules in step with the repository automatically:

- **Fresh clone:** uninitialized submodules are initialized.
- **Pull / branch switch moved a submodule's recorded commit:** the submodule follows it.
- **You checked a submodule out at another commit by hand:** it is left alone. If the recorded
  commit *also* changed, configure prints a warning once and leaves it alone.

The sync runs at configure time. To have git update submodules immediately on pull/checkout as
well, run once in your clone: `git config submodule.recurse true`.

**Updating a submodule** (e.g. SDL3 to a new release):

```bash
cd libraries/3rd_party/SDL3 && git fetch && git checkout <tag or commit> && cd -
# build and test: SDL3 is rebuilt once into a new cache folder
git add libraries/3rd_party/SDL3 && git commit -m "Update SDL3 to <version>"
```

Other machines pick it up on their next configure after pulling. Afterwards,
`prune_prebuilt_libs` removes the old cache folder. When updating imgui, check that its core
`.cpp` files still match the list in `libraries/3rd_party/imgui_build.cmake`.

## Adding a library

Use `add_and_export_library` (`cmake_scripts/link_and_export_utilities.cmake`):

```cmake
add_and_export_library(MyLib TYPE SHARED FILES a.cpp b.cpp)
target_link_libraries(MyLib PUBLIC LinearAlgebra PRIVATE math_ops)
```

It generates `mylib_export.h` with a `MYLIB_EXPORT` macro: mark the public functions/classes in
your headers with it. Everything else is hidden on every platform (`dllexport`/`dllimport` on
Windows, symbol visibility on Linux). Shared libraries made this way are installed
automatically; pass `NO_INSTALL` for test/benchmark-only ones.

## Troubleshooting

- **"The '...' generator is not supported"**: use Ninja in a new, empty build directory (see
  the message for CLion / Visual Studio instructions).
- **Linux: every build recompiles and relinks, even with nothing changed**: the compiler is
  being invoked as `/bin/clang`. Use `/usr/bin/clang` (`/bin` is a symlink, which confuses
  Ninja's dependency paths).
- **Something odd with SDL3/assimp** (link errors, a broken DLL): delete
  `.prebuild_libs/<lib>/<setup>/` and build again.
- **Don't build the same setup from two build directories at the same time while its cache is
  being created** (e.g. a terminal and CLion on a fresh clone); they would install into the
  same cache folder concurrently.
- **Local edits inside SDL3/assimp sources are not detected** (the cache is keyed on the
  submodule commit). Delete their cache folder after patching them.
