include(../../cmake_scripts/link_and_export_utilities.cmake)
add_and_export_library(Common_Types TYPE INTERFACE)

target_link_libraries(Common_Types INTERFACE libglew_shared)