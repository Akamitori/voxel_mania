
include(../../cmake_scripts/link_and_export_utilities.cmake)

add_and_export_library(OpenGlHelpers 
        TYPE SHARED
        C_STANDARD 11        
        FILES
            ShaderLoader.c
)

target_link_libraries(OpenGlHelpers PRIVATE libglew_shared)