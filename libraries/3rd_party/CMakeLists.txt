# Ensure all submodules are initialized
message("Initializing git submodules")
execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE result
)
# Check if the command was successful
if (NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to initialize submodules. Please run:
        git submodule update --init --recursive")
endif ()

add_subdirectory(SDL3)

set(glew-cmake_BUILD_SHARED ON CACHE BOOL "Build the shared glew library")
set(glew-cmake_BUILD_STATIC OFF CACHE BOOL "Build the static glew library")
set(ONLY_LIBS ON CACHE BOOL "Do not build executables")
add_subdirectory(glew)

install(TARGETS SDL3-shared libglew_shared
        RUNTIME DESTINATION ${BINARY_DIRECTORY}  # DLLs
        LIBRARY DESTINATION ${BINARY_DIRECTORY_LIBS}  # .so/.dylib
)