
include(../../cmake_scripts/link_and_export_utilities.cmake)

add_and_export_library(OpenGlHelpers TYPE SHARED FILES
        ShaderLoader.cpp
)
target_link_libraries(OpenGlHelpers PRIVATE libglew_shared SDL3::SDL3)