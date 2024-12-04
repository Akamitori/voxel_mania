# Ensure all submodules are initialized
message("Initializing git submodules")
execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE result
)

add_subdirectory(SDL3)
add_subdirectory(glm)

set(glew-cmake_BUILD_SHARED ON CACHE BOOL "Build the shared glew library")
set(glew-cmake_BUILD_STATIC OFF CACHE BOOL "Build the static glew library")
set(ONLY_LIBS ON CACHE BOOL "Do not build executables")
add_subdirectory(glew)



# Check if the command was successful
if (NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to initialize submodules. Please run:
        git submodule update --init --recursive")
endif()