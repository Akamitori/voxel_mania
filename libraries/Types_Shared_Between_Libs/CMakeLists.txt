include(../../cmake_scripts/link_and_export_utilities.cmake)
add_and_export_library(Common_Lib_Types TYPE INTERFACE)

target_link_libraries(Common_Lib_Types INTERFACE libglew_shared)


target_link_libraries(Common_Lib_Types INTERFACE  Common_Hashmaps)