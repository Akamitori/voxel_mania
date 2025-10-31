# Initialize git submodules
find_package(Git REQUIRED)
execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE git_result
)
if(NOT git_result EQUAL 0)
    message(FATAL_ERROR "Failed to initialize git submodules")
endif()

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


# Other libraries
include(imgui_build.cmake)
add_subdirectory(stb_image)