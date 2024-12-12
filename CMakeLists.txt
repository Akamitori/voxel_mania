cmake_minimum_required(VERSION 3.29)
project(voxel_mania)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(3rd_party)
find_package(OpenGL REQUIRED)

add_executable(voxel_mania main.cpp
        Camera.cpp
        Camera.h
        data/cube.h
        data/tile.h
        InputHandling.cpp
        InputHandling.h
        AppData.h
        math_ops.h
        math_ops.cpp
        RandomNumberGeneration.cpp
        RandomNumberGeneration.h
        Perlin.h
        Perlin.cpp
        TerrainGeneration.h
        TerrainGeneration.cpp
        Probabilities.cpp
        Probabilities.h
        Vector2D.h
)

target_link_libraries(voxel_mania
        PUBLIC
        libglew_shared
        glm
        SDL3::SDL3
)

##### -------------COPY DLLS           --------------
# Define the directory to search (e.g., 3rdparty build output directory)
set(THIRD_PARTY_DIR "${CMAKE_BINARY_DIR}/3rd_party")

# Identify the file extension based on the platform
if(WIN32)
    set(SHARED_LIB_EXT "dll")
elseif(APPLE)
    set(SHARED_LIB_EXT "dylib")
else()
    set(SHARED_LIB_EXT "so")
endif()

# Recursively find all shared libraries with the specified extension
file(GLOB_RECURSE THIRD_PARTY_LIBS "${THIRD_PARTY_DIR}/*.${SHARED_LIB_EXT}")

# Copy each found library to the main output directory
foreach(LIB ${THIRD_PARTY_LIBS})
    add_custom_command(TARGET voxel_mania POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIB} $<TARGET_FILE_DIR:voxel_mania>
    )
endforeach()


##### -------------COPY SHADERS           --------------
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/data")
set(SHADER_DEST_DIR "${CMAKE_BINARY_DIR}/data")

# Copy the shaders to the build directory
add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHADER_SOURCE_DIR}" "${SHADER_DEST_DIR}"
        COMMENT "Copying shader programs to build directory"
)

add_dependencies(voxel_mania copy_shaders)
