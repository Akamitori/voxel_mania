# Initialize git submodules
find_package(Git REQUIRED)
execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE git_result
)
if (NOT git_result EQUAL 0)
    message(FATAL_ERROR "Failed to initialize git submodules")
endif ()

# Absolute path to the assimp submodule. Uses CMAKE_CURRENT_LIST_DIR because
# CMAKE_CURRENT_SOURCE_DIR is overwritten below by the GLEW workaround.
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
)


# GLEW - fast build, use add_subdirectory
set(glew-cmake_BUILD_SHARED ON CACHE BOOL "Build shared GLEW")
set(glew-cmake_BUILD_STATIC OFF CACHE BOOL "Don't build static GLEW")
set(ONLY_LIBS ON CACHE BOOL "GLEW libs only")

set(CMAKE_CURRENT_SOURCE_DIR "not_main" CACHE INTERNAL "" FORCE)
add_subdirectory(glew)

# detect the proper name for assimp lib file
if (MSVC)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ASSIMP_LIB_NAME "assimp-vc143-mtd")
    else ()
        set(ASSIMP_LIB_NAME "assimp-vc143-mt")
    endif ()
else ()
    set(ASSIMP_LIB_NAME "assimp")
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