#include <fstream>
#include <array>
#include <format>

#include <GL/glew.h>
#include <SDL3/SDL_init.h>

#include "InputHandling.h"
#include "cube.h"
#include "Matrix4D.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "OpenGlHelpers/ShaderLoader.h"
#include "Renderer/Renderer.h"


// Vertex Shader source code
float normalize_coord(const float value, const float max) {
    return 2 * value / max - 1;
}

void Draw_Cursor(const unsigned int cursorProgram, const unsigned int cursor_vao) {
    glUseProgram(cursorProgram);
    glBindVertexArray(cursor_vao);
    glDrawArrays(GL_LINES, 0, 8);
    glBindVertexArray(0);
    glUseProgram(0);
}

void InitializeCursorVBO(const std::array<float, 12> &cursor, unsigned int &cursor_vao, unsigned int &cursor_vbo) {
    glGenVertexArrays(1, &cursor_vao);
    glGenBuffers(1, &cursor_vbo);

    glBindVertexArray(cursor_vao);

    // Bind and set VBO
    glBindBuffer(GL_ARRAY_BUFFER, cursor_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cursor.size(), cursor.data(), GL_STATIC_DRAW);

    // Define the vertex attributes (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Unbind the VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// void calculate_frustum_planes_and_check_for_visibility(const Matrix4D &world_space_matrix, AppData &appData,
//                                                        const Matrix4D &triangle_model_view_space,
//                                                        const cube &my_cube) {
//     Vector3D vertex_data[8];
//
//     build_frustum_polyhedron(
//         inverse(appData.camera_matrix),
//         1 / tan(DegreeToRadians(appData.FOV) * 0.5f),
//         static_cast<float>(initial_screen_width) / static_cast<float>(initial_screen_height),
//         appData.z_near, appData.z_far, &appData.frustum_polyhedron);
//
//     for (int i = 0, j = 0; i < 8 && j < 24; i++, j += 3) {
//         auto point = Vector3D{
//             my_cube.vertex_data[j],
//             my_cube.vertex_data[j + 1],
//             my_cube.vertex_data[j + 2]
//         };
//
//         // get the cube to global space
//         vertex_data[i] = transform_point(world_space_matrix, transform_point(triangle_model_view_space, point));
//     }
//
//     // check if the cube is contained in the frustum
//     auto a = calculate_axis_aligned_bounding_box(8, vertex_data);
//     auto result = AxisAlignedBoxVisible(6, appData.frustum_polyhedron.plane, a);
//     std::cout << "Box contained : " << result << std::endl;
// }

// draw this model at this position
//

struct model_instance {
    int mesh_id{};
    Vector3D pos{};
    Vector3D color{};
};

int RegisterCubeMesh3Part(int mesh_id) {
    return Renderer_RegisterTexturedMesh(
        mesh_id,
        cube::vertex_data_uv_3_part_texture,
        cube::vertices_count_uv,
        cube::vertex_indices_uvs,
        cube::vertex_indices_count_uv
    );
}

int RegisterCubeMesh1Part(int mesh_id) {
    return Renderer_RegisterTexturedMesh(
        mesh_id,
        cube::vertex_data_uv_1_part_texture,
        cube::vertices_count_uv,
        cube::vertex_indices_uvs,
        cube::vertex_indices_count_uv
    );
}


int main() {
    constexpr int initial_screen_width = 800, initial_screen_height = 600;
    Renderer_Init(initial_screen_width, initial_screen_height, 45, 0.1, 100);

    const int woodTextureId = Renderer_RegisterTexture("data/textures/wood.png");
    const int grassTextureId = Renderer_RegisterTexture("data/textures/grass_block.png");
    const int mushroomTextureId = Renderer_RegisterTexture("data/textures/mushroom_red.png");

    // if our problem is wrapping cubes and data like that
    // we can probably have the API acknowledge that
    const int cubeId_1 = RegisterCubeMesh3Part(
        grassTextureId
    );

    const int cubeId_2 = Renderer_RegisterTextured_Cross_Mesh(
        mushroomTextureId ,
        0.5f
    );

    const int cubeId_3 = Renderer_RegisterTextured_Cross_Mesh(
        mushroomTextureId ,
        1
    );


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // so we can dock things to windows
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // so our ui can exist outside the window
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplSDL3_InitForOpenGL(window, open_gl_context);
    ImGui_ImplOpenGL3_Init();

    const unsigned int cursorProgram = InitializeProgram("cursor_program");
    const GLint cursor_color_uniform = glGetUniformLocation(cursorProgram, "cursor_color");


    constexpr Vector4D cursor_color(1, 0, 0, 1);
    glUseProgram(cursorProgram);
    glUniform4fv(cursor_color_uniform, 1, &cursor_color.x);

    constexpr float centerX = static_cast<float>(initial_screen_width) / 2.0f;
    constexpr float centerY = static_cast<float>(initial_screen_height) / 2.0f;
    const std::array cursor{
        normalize_coord(centerX - 10, static_cast<float>(initial_screen_width)),
        normalize_coord(centerY, static_cast<float>(initial_screen_height)),
        1.0f,

        normalize_coord(centerX + 10, static_cast<float>(initial_screen_width)),
        normalize_coord(centerY, static_cast<float>(initial_screen_height)),
        1.0f,

        normalize_coord(centerX, static_cast<float>(initial_screen_width)),
        normalize_coord(centerY - 10, static_cast<float>(initial_screen_height)),
        1.0f,

        normalize_coord(centerX, static_cast<float>(initial_screen_width)),
        normalize_coord(centerY + 10, static_cast<float>(initial_screen_height)),
        1.0f,
    };


    unsigned int cursor_vao;
    unsigned int cursor_vbo;
    InitializeCursorVBO(cursor, cursor_vao, cursor_vbo);

    Renderer_FinalizeMeshLoading();

    model_instance models[]{
        cubeId_2, Vector3D{0, 0, 0}, Vector3D{1, 0, 0},
        cubeId_3, Vector3D{1, 0, 0}, Vector3D{1, 0, 0},
        cubeId_1, Vector3D{0, 0, -1}, Vector3D{1, 0, 0},
        
    };

    bool keepRunning = true;
    // Rendering loop
    while (keepRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_WINDOW_RESIZED: {
                    Renderer_ResolutionChanged(event.window.data1, event.window.data2);
                    break;
                }
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                    keepRunning = false;
                    break;
                }
                case SDL_EVENT_KEY_DOWN: {
                    KeyDown(event.key.scancode, *SceneCamera);
                    break;
                }
                default: {
                    break;
                }
            }

            ImGui_ImplSDL3_ProcessEvent(&event); // Forward your event to backend
        }

        Renderer_FrameStart();

        for (const auto &[mesh_id, pos, color]: models) {
            Renderer_Draw(mesh_id, pos, color);
        }


        Draw_Cursor(cursorProgram, cursor_vao);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "Missing Dear ImGui context. Refer to examples app!");

        // Verify ABI compatibility between caller code and compiled version of Dear ImGui. This helps detects some build issues.
        IMGUI_CHECKVERSION();

        // float *p_as_float3 = reinterpret_cast<float *>(&t);
        //
        //
        // if (ImGui::InputFloat3("test", p_as_float3, "%.3f")
        // ) {
        //     appData.view_dirty = true;
        // }
        // Show demo window! :)

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
            const SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext(); // NOLINT(*-misplaced-const) , we want this as is
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }


        Renderer_FrameEnd();
    }

    glDeleteVertexArrays(1, &cursor_vao);
    glDeleteBuffers(1, &cursor_vbo);

    Renderer_Destroy();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
