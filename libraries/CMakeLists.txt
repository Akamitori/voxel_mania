#add the common export header
add_library(ExportHeader INTERFACE)
target_include_directories(ExportHeader INTERFACE .)

add_subdirectory(math)
add_subdirectory(3rd_party)
add_subdirectory(OpenGlHelpers)