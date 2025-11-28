include(../../cmake_scripts/link_and_export_utilities.cmake)

add_and_export_library(math_ops TYPE SHARED FILES
        math_ops.cpp
        math_ops.h
)