# Dear ImGui core + the SDL3 / OpenGL3 backends, listed explicitly (imgui's documented file set).
# A glob would only be re-evaluated at configure time, so after an imgui submodule update a
# removed or added file would break the build until the next reconfigure.
add_library(DearImGui STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(DearImGui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

# Note: Make sure SDL3::SDL3 target is available when this is processed
target_link_libraries(DearImGui PUBLIC SDL3::SDL3 libglew_shared)