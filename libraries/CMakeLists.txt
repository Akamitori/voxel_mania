include(../cmake_scripts/link_and_export_utilities.cmake)

add_and_export_library(ExportHeader TYPE INTERFACE)

add_subdirectory(math)
add_subdirectory(math_ops)
add_subdirectory(3rd_party)
add_subdirectory(OpenGlHelpers)
add_subdirectory(Renderer)
add_subdirectory(Containers)
add_subdirectory(AssetLoader)
add_subdirectory(Common_Types)