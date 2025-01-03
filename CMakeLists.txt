cmake_minimum_required(VERSION 3.30)
project(voxel_mania)

set(CMAKE_CXX_STANDARD 23)

set(BINARY_DIRECTORY ${CMAKE_BINARY_DIR} CACHE PATH "The target destination of our build")
set(BINARY_DIRECTORY_LIBS ${CMAKE_BINARY_DIR} CACHE PATH "The target destination of our libs")

include(cmake_scripts/compiler_flags.cmake)

add_subdirectory(libraries)

find_package(OpenGL REQUIRED)

add_executable(voxel_mania main.cpp
        Camera.cpp
        Camera.h
        data/cube.h
        InputHandling.cpp
        InputHandling.h
        AppData.h
        math_ops.h
        math_ops.cpp
        RandomNumberGeneration.cpp
        RandomNumberGeneration.h
        Perlin.h
        Perlin.cpp
)

target_link_libraries(voxel_mania
        PUBLIC
        libglew_shared
        SDL3::SDL3
        LinearAlgebra
        Trigonometry
)


##### -------------COPY SHADERS           --------------
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/data")
set(SHADER_DEST_DIR "${BINARY_DIRECTORY}/data")

# Copy the shaders to the build directory
add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHADER_SOURCE_DIR}" "${SHADER_DEST_DIR}"
        COMMENT "Copying shader programs to build directory"
)

add_dependencies(voxel_mania copy_shaders)

add_custom_command(
        TARGET voxel_mania POST_BUILD
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}
        COMMENT "Running the install step after all builds are complete"
)
