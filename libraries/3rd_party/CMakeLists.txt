# Git submodules follow the superproject automatically (fresh clone, pull, branch switch), but
# a submodule you checked out at another commit by hand is left alone. See submodule_sync.cmake.
# -DVOXEL_SYNC_SUBMODULES=ON forces `git submodule update --init --recursive` on each configure.
find_package(Git REQUIRED)
option(VOXEL_SYNC_SUBMODULES "Force git submodule update --init --recursive on every configure" OFF)
include(${CMAKE_SOURCE_DIR}/cmake_scripts/submodule_sync.cmake)
voxel_sync_submodules(REPO_DIR "${CMAKE_SOURCE_DIR}" STATE_FILE "${CMAKE_SOURCE_DIR}/.prebuild_libs/.submodule_sync")

# Absolute path to the assimp submodule.
set(ASSIMP_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/assimp")
if (NOT EXISTS "${ASSIMP_SOURCE_DIR}/include/assimp/scene.h")
    message(FATAL_ERROR "assimp not found at: ${ASSIMP_SOURCE_DIR}")
endif ()
message(STATUS "ASSIMP_SOURCE_DIR = ${ASSIMP_SOURCE_DIR}")

include(${CMAKE_SOURCE_DIR}/cmake_scripts/external_lib_import.cmake)

# SDL3 with incremental building
import_external_library(SDL3
        SHARED
        CMAKE_ARGS -DSDL_SHARED=ON -DSDL_STATIC=OFF
        # SDL_LIBC is SDL's default (ON), but SDL3's CMakeLists checks it (to strip /RTC1 on MSVC)
        # before defining it. Unset on the first configure, so only the first configure differs
        # and the first re-configure recompiles all of SDL3. Setting it up front avoids that.
        -DSDL_LIBC=ON
)


# GLEW - fast build, use add_subdirectory
set(glew-cmake_BUILD_SHARED ON CACHE BOOL "Build shared GLEW")
set(glew-cmake_BUILD_STATIC OFF CACHE BOOL "Don't build static GLEW")
set(ONLY_LIBS ON CACHE BOOL "GLEW libs only")

# EXCLUDE_FROM_ALL drops all of glew-cmake's install rules (libs, headers, glewConfig.cmake).
# libglew_shared is still built because our targets link against it.
add_subdirectory(glew EXCLUDE_FROM_ALL)

# assimp's file name is pinned to assimp / assimpd on every platform: LIBRARY_SUFFIX= below
# disables its "-vc<toolset>-mt" (MSVC) and "-<soversion>" (MinGW) suffixes. Debug keeps
# assimp's "d" postfix (ASSIMP_INJECT_DEBUG_POSTFIX, on by default).
set(ASSIMP_LIB_NAME "assimp")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    string(APPEND ASSIMP_LIB_NAME "d")
endif ()

# assimp with incremental building
import_external_library(${ASSIMP_LIB_NAME}
        SHARED
        SOURCE_DIR "${ASSIMP_SOURCE_DIR}"                       # point to the proper dir since we can't rely on the lib name
        TARGET_NAME assimp::assimp                              # cross platform target
        CMAKE_ARGS
        -DASSIMP_BUILD_TESTS=OFF                                # skip building tests
        -DASSIMP_BUILD_ASSIMP_TOOLS=OFF                         # skip building tools
        -DASSIMP_INSTALL=ON                                     # retain install step because we need to actually install for incremental builds to work
        -DBUILD_SHARED_LIBS=ON                                  # build as a shared lib
        -DASSIMP_NO_EXPORT=ON                                   # disable for now the export tools
        -DLIBRARY_SUFFIX=                                       # same file name on every compiler (see above)
        -DASSIMP_WARNINGS_AS_ERRORS=OFF                         # assimp has warnings under gcc that -Werror turns into build failures
)
# Other libraries
include(imgui_build.cmake)
add_subdirectory(stb_image)

# assimp API docs (build on demand: cmake --build <build dir> --target assimp_docs)
find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME         "assimp")
    set(DOXYGEN_EXTRACT_ALL          YES)
    set(DOXYGEN_RECURSIVE            YES)
    set(DOXYGEN_GENERATE_LATEX       NO)
    set(DOXYGEN_ENABLE_PREPROCESSING YES)
    set(DOXYGEN_MACRO_EXPANSION      YES)
    set(DOXYGEN_PREDEFINED __cplusplus C_STRUCT= C_ENUM= ASSIMP_API= AI_NO_EXCEPT=)
    set(DOXYGEN_IMAGE_PATH ${ASSIMP_SOURCE_DIR}/doc/images
            ${ASSIMP_SOURCE_DIR}/doc/architecture)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assimp-docs)

    doxygen_add_docs(assimp_docs
            ${ASSIMP_SOURCE_DIR}/include/assimp
            ${ASSIMP_SOURCE_DIR}/doc/dox.h
            COMMENT "Generating assimp docs")
else ()
    message(STATUS "Doxygen not found - assimp_docs target unavailable")
endif ()