# Selects ONE linker for the whole build: our targets, add_subdirectory'd 3rd party code (GLEW,
# imgui) and the external libraries built by import_external_library (SDL3, assimp).
#
# - Main project: include()d right after project(). Decides VOXEL_LINKER from the options below.
# - External libs: injected with -DCMAKE_PROJECT_INCLUDE=<this file> and the already-decided
#   -DVOXEL_LINKER=... (+ RADLINK_EXECUTABLE), so they apply the same choice without re-deciding.
#
# VOXEL_LINKER values: MOLD, RADLINK, DEFAULT.

include_guard(GLOBAL)

if (NOT DEFINED VOXEL_LINKER)
    set(VOXEL_LINKER DEFAULT)

    if (MSVC)
        # RAD linker: MSVC only, Debug/RelWithDebInfo, not with ASan. Used when found.
        # Found on PATH or in C:/rad_linker; point elsewhere with -DRADLINK_EXECUTABLE=<path>.
        # radlink can't read /GL (LTCG) objects: keep INTERPROCEDURAL_OPTIMIZATION off in these configs.
        option(USE_RADLINK "Link MSVC Debug/RelWithDebInfo builds with the RAD linker when it is found" ON)
        if (USE_RADLINK AND NOT ENABLE_ASAN AND CMAKE_BUILD_TYPE MATCHES "^(Debug|RelWithDebInfo)$")
            find_program(RADLINK_EXECUTABLE
                    NAMES radlink
                    HINTS "C:/rad_linker"
                    DOC "Path to the RAD linker (radlink.exe)")
            if (NOT RADLINK_EXECUTABLE)
                message(STATUS "RAD linker not found (set RADLINK_EXECUTABLE to use it), using the default MSVC linker")
            elseif (CMAKE_GENERATOR MATCHES "^Visual Studio")
                # The VS generator links through MSBuild and ignores CMAKE_LINKER
                message(STATUS "RAD linker found but not supported with the ${CMAKE_GENERATOR} generator, using the default MSVC linker")
            else ()
                set(VOXEL_LINKER RADLINK)
            endif ()
        endif ()
    elseif (UNIX AND NOT APPLE)
        # mold: fast, stable ELF linker; works with gcc and clang. Used when found.
        option(USE_MOLD "Link with mold when it is found" ON)
        if (USE_MOLD)
            find_program(MOLD_EXECUTABLE NAMES mold DOC "Path to the mold linker")
            if (MOLD_EXECUTABLE)
                set(VOXEL_LINKER MOLD)
            else ()
                message(STATUS "mold not found, using the default linker")
            endif ()
        endif ()
    endif ()

    # Arguments that hand the decision to the external library sub-builds
    set(VOXEL_LINKER_FORWARD_ARGS
            -DCMAKE_PROJECT_INCLUDE=${CMAKE_CURRENT_LIST_FILE}
            -DVOXEL_LINKER=${VOXEL_LINKER})
    if (VOXEL_LINKER STREQUAL "RADLINK")
        list(APPEND VOXEL_LINKER_FORWARD_ARGS -DRADLINK_EXECUTABLE=${RADLINK_EXECUTABLE})
    endif ()
endif ()

# Apply the choice (main project and every external sub-build)
if (VOXEL_LINKER STREQUAL "MOLD")
    # CMake 3.29+: adds -fuse-ld=mold for gcc/clang
    set(CMAKE_LINKER_TYPE MOLD)
    message(STATUS "Linker: mold")
elseif (VOXEL_LINKER STREQUAL "RADLINK")
    set(CMAKE_LINKER "${RADLINK_EXECUTABLE}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " /MANIFEST:NO")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " /MANIFEST:NO")
    string(APPEND CMAKE_MODULE_LINKER_FLAGS " /MANIFEST:NO")
    add_link_options(/INCREMENTAL:NO)
    message(STATUS "Linker: RAD linker ${RADLINK_EXECUTABLE}")
else ()
    message(STATUS "Linker: default")
endif ()
