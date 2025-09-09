# Fixed version - more robust file handling

file(GLOB im_gui_cpp_files_full_path
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h"
)

add_library(DearImGui STATIC
        ${im_gui_cpp_files_full_path}
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(DearImGui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

# Note: Make sure SDL3::SDL3 target is available when this is processed
target_link_libraries(DearImGui PUBLIC SDL3::SDL3 libglew_shared)