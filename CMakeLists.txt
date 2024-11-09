cmake_minimum_required(VERSION 3.29)
project(voxel_mania)

set(CMAKE_CXX_STANDARD 20)

#-----FUNCTIONS----
function(add_git_submodule module_name)
    # Construct the path to the submodule
    set(submodule_path "${CMAKE_SOURCE_DIR}/3rd_party/${module_name}")
    message("Adding ${submodule_path} to sub_directories")
    # Add the submodule directory
    add_subdirectory("${submodule_path}")
endfunction()

# Ensure all submodules are initialized
execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE result
)

# Check if the command was successful
if (NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to initialize submodules. Please run:
        git submodule update --init --recursive")
endif()

find_package(glm)
find_package(glew)

add_git_submodule(SDL3)

# Then, link your executable or library with the corresponding package targets:

add_executable(voxel_mania main.cpp
        Camera.cpp
        Camera.h
        data/cube.cpp
        data/cube.h
        math_ops.cpp
        math_ops.h
        InputHandling.cpp
        InputHandling.h
        AppData.h)

target_link_libraries(voxel_mania
        glm::glm
        GLEW::GLEW
        SDL3::SDL3
)

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

##### -------------COPY DLLS           --------------
