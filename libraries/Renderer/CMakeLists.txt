include(../../cmake_scripts/link_and_export_utilities.cmake)

add_and_export_library(Renderer TYPE SHARED FILES
        Renderer.cpp
        Camera.cpp
)

target_link_libraries(Renderer
        PUBLIC
        LinearAlgebra        # Camera uses Vector3D in public header
        PRIVATE
        SDL3::SDL3          # Internal only
        OpenGlHelpers       # Internal only
        libglew_shared      # Internal only
        primitive_mesh_data # Internal only
        stb_image          # Internal only
        Trigonometry       # Internal only
        math_ops           # Internal only
)