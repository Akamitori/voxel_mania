include(../../cmake_scripts/link_and_export_utilities.cmake)

add_and_export_library(LinearAlgebra TYPE SHARED FILES
        Vector2D.cpp
        Vector3D.cpp
        Vector4D.cpp
        Matrix3D.cpp
        Matrix4D.cpp
        Transformations.cpp
        Plane.cpp
        Clipping.cpp
        Bounding.cpp
)
add_and_export_library(Trigonometry TYPE SHARED FILES Trigonometry.cpp)

