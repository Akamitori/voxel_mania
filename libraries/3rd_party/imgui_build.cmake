# Glob source files from the actual imgui directory
file(GLOB im_gui_cpp_files_full_path
        "imgui/*.cpp"
        "imgui/*.h"
)

add_library(
        DearImGui
        STATIC
        ${im_gui_cpp_files_full_path}
        imgui/backends/imgui_impl_sdl3.cpp
        imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(DearImGui PUBLIC
        ${CMAKE_SOURCE_DIR}/libraries/3rd_party/imgui
        ${CMAKE_SOURCE_DIR}/libraries/3rd_party/imgui/backends
)

target_link_libraries(DearImGui PUBLIC SDL3::SDL3 libglew_shared)