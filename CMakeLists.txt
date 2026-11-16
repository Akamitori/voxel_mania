cmake_minimum_required(VERSION 3.30)
project(voxel_mania)

# Single-config generators only: the external library cache (cmake_scripts/external_lib_import.cmake)
# decides library paths, names and the linker per CMAKE_BUILD_TYPE at configure time.
get_property(VOXEL_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (VOXEL_IS_MULTI_CONFIG)
    message(FATAL_ERROR
            "The '${CMAKE_GENERATOR}' generator is not supported.\n"
            "This project needs a single-config generator, with the build type chosen at configure time. "
            "Multi-config generators (Visual Studio, Ninja Multi-Config, Xcode) would build the external "
            "libraries (SDL3, assimp) for every configuration into the same cache and link the wrong ones.\n"
            "How to fix it (generator can't be changed in an existing build dir, so use a new/empty one):\n"
            "  Command line:   cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug\n"
            "                  (Windows: run it from a \"x64 Native Tools / Developer\" prompt for MSVC)\n"
            "  CLion:          the default CMake profiles already use Ninja; if you changed the generator, "
            "set it back to Ninja (Settings > Build, Execution, Deployment > CMake > Generator) and Reset Cache.\n"
            "  Visual Studio:  use File > Open > Folder (CMake mode, uses Ninja) instead of generating a .sln.\n"
            "Build types: one build dir per type, e.g. build-debug (Debug) and build-release (Release).")
endif ()

# Single-config without a build type would mean no optimization and no debug info
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
    message(STATUS "CMAKE_BUILD_TYPE not set, defaulting to Debug")
endif ()

set(CMAKE_CXX_STANDARD 23)
# Fail at configure if the compiler lacks C++23, instead of silently falling back to an older standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# We don't use C++20 modules. Without this, CMake runs a dependency scan (clang-scan-deps /
# cl /scanDependencies) for every C++ source on every build just in case.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(ENABLE_ASAN "Build with AddressSanitizer" OFF)
if (ENABLE_ASAN)
    if (MSVC)
        string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        add_compile_options(/fsanitize=address /Zi)
        add_link_options(/INCREMENTAL:NO /DEBUG)
    else ()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address)
    endif ()
endif ()

# One linker for everything (mold on Linux, RAD linker on MSVC Debug/RelWithDebInfo, else default).
# Also forwarded to the SDL3/assimp sub-builds, see cmake_scripts/linker_setup.cmake.
include(cmake_scripts/linker_setup.cmake)

if (UNIX AND NOT APPLE)
    # Installed layout (relocatable, same shape as on Windows; the game loads data/ relative to the
    # working directory, so run it from the install dir):
    #   <prefix>/voxel_mania   <prefix>/data/   <prefix>/lib/*.so
    # $ORIGIN/lib is for the executable, $ORIGIN for the libraries in lib/. Must be set before
    # the targets are created.
    set(CMAKE_INSTALL_RPATH "$ORIGIN/lib;$ORIGIN")
endif ()


add_subdirectory(libraries)
add_subdirectory(primitive_mesh_data)

# Housekeeping: cmake --build <dir> --target prune_prebuilt_libs
# Deletes .prebuild_libs cache dirs built from submodule commits that are no longer used.
get_property(VOXEL_PREBUILT_COMMITS GLOBAL PROPERTY VOXEL_PREBUILT_COMMITS)
list(REMOVE_DUPLICATES VOXEL_PREBUILT_COMMITS)
string(REPLACE ";" "," VOXEL_PREBUILT_COMMITS "${VOXEL_PREBUILT_COMMITS}")
add_custom_target(prune_prebuilt_libs
        COMMAND ${CMAKE_COMMAND}
        -DPREBUILT_DIR=${CMAKE_SOURCE_DIR}/.prebuild_libs
        -DKEEP_COMMITS=${VOXEL_PREBUILT_COMMITS}
        -P ${CMAKE_SOURCE_DIR}/cmake_scripts/prune_prebuilt_libs.cmake
        COMMENT "Removing stale .prebuild_libs cache dirs"
        VERBATIM)
find_package(OpenGL REQUIRED)

add_executable(voxel_mania
        main.cpp
        InputHandling.cpp
        AppData.h
        RandomNumberGeneration.cpp
        Perlin.cpp
)


voxel_enable_warnings(voxel_mania)

target_link_libraries(voxel_mania
        PUBLIC
        primitive_mesh_data
        PRIVATE
        libglew_shared
        SDL3::SDL3
        DearImGui
        OpenGlHelpers
        math_ops
        Renderer
        Trigonometry
        Custom_Containers
        AssetLoader
)

# Copy shaders
add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:voxel_mania>/data"
        COMMENT "Copying shaders"
)
add_dependencies(voxel_mania copy_shaders)

# Automatically copy all runtime shared libraries (cross-platform)
if (WIN32)
    add_custom_command(TARGET voxel_mania POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:voxel_mania>
            $<TARGET_FILE_DIR:voxel_mania>
            COMMAND_EXPAND_LISTS
            COMMENT "Copying runtime shared libraries"
    )
endif ()

# Install: executable + its shared libraries + data/
if (WIN32)
    install(TARGETS voxel_mania RUNTIME DESTINATION . COMPONENT Application)
    # All DLLs next to the executable (same set as the POST_BUILD copy above)
    install(FILES $<TARGET_RUNTIME_DLLS:voxel_mania> DESTINATION . COMPONENT Application)
elseif (UNIX AND NOT APPLE)
    install(TARGETS voxel_mania
            RUNTIME_DEPENDENCY_SET voxel_runtime_deps
            RUNTIME DESTINATION . COMPONENT Application)
    # Our shared libraries and GLEW: install(TARGETS) rewrites their RPATH to CMAKE_INSTALL_RPATH
    get_property(VOXEL_SHARED_LIBS GLOBAL PROPERTY VOXEL_SHARED_LIBS)
    # Their own dependencies (e.g. AssetLoader -> assimp) go into the same set: the scan doesn't
    # descend into libraries it excludes, so the executable's set alone would miss them.
    install(TARGETS ${VOXEL_SHARED_LIBS} libglew_shared
            RUNTIME_DEPENDENCY_SET voxel_runtime_deps
            LIBRARY DESTINATION lib COMPONENT Application)
    # SDL3/assimp: resolved from the executable's runtime dependencies, limited to this build dir's
    # prebuilt/ copies (system libraries and the targets above are excluded). Their RPATH is
    # already $ORIGIN, and the soname symlinks are installed along with the real files.
    install(RUNTIME_DEPENDENCY_SET voxel_runtime_deps
            POST_INCLUDE_REGEXES "^${CMAKE_BINARY_DIR}/prebuilt/"
            POST_EXCLUDE_REGEXES ".*"
            LIBRARY DESTINATION lib COMPONENT Application)
endif ()

# Install data folder  
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/" DESTINATION data COMPONENT Application)

