include(../../cmake_scripts/link_and_export_utilities.cmake)

add_and_export_library(AssetLoader TYPE SHARED FILES
        AssetLoader.cpp
)

target_link_libraries(AssetLoader
        PUBLIC
        LinearAlgebra        # Camera uses Vector3D in public header
        PRIVATE
        assimp::assimp
        Custom_Containers
        stb_image          # Internal only
)