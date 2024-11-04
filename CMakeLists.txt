cmake_minimum_required(VERSION 3.29)
project(voxel_mania)

set(CMAKE_CXX_STANDARD 20)

find_package(glm)
find_package(glfw3)
find_package(glew)

# Then, link your executable or library with the corresponding package targets:

add_executable(voxel_mania main.cpp
        Camera.cpp
        Camera.h
        data/cube.cpp
        data/cube.h
        math_ops.cpp
        math_ops.h)

target_link_libraries(voxel_mania
        glm::glm
        glfw
        GLEW::GLEW)

## copy all data files to the build directory
# Define the source and destination for the shaders
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/data")
set(SHADER_DEST_DIR "${CMAKE_BINARY_DIR}/data")

# Copy the shaders to the build directory
add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHADER_SOURCE_DIR}" "${SHADER_DEST_DIR}"
        COMMENT "Copying shader programs to build directory"
)

add_dependencies(voxel_mania copy_shaders)