#include <array>
#include <exception>

#include <GL/glew.h>
#include <SDL3/SDL_init.h>

#include "InputHandling.h"
#include "cube.h"
#include "Matrix4D.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "Trigonometry.h"
#include "OpenGlHelpers/ShaderLoader.h"
#include "Renderer/Renderer.h"
#include "SDL3/SDL_timer.h"

#include "quad.h"
#include "libraries/Renderer/Camera.h"

#include "Main_Containers.h"
#include "Perlin.h"


// Vertex Shader source code
float normalize_coord(const float value, const float max) {
    return 2 * value / max - 1;
}

void Draw_Cursor(const unsigned int cursorProgram, const unsigned int cursor_vao) {
    glDisable(GL_STENCIL_TEST);
    glUseProgram(cursorProgram);
    glBindVertexArray(cursor_vao);
    glDrawArrays(GL_LINES, 0, 8);
    glBindVertexArray(0);
    glUseProgram(0);
    glEnable(GL_STENCIL_TEST);
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


int RegisterCubeMesh3Part(int diffuse_texture_id, int specular_texture_id, int emission_texture_id = -1) {
    return Renderer_RegisterTexturedMesh(
        diffuse_texture_id,
        specular_texture_id,
        emission_texture_id,
        cube::vertex_data_uv_3_part_texture,
        cube::vertices_count_uv,
        cube::vertex_indices_uvs,
        cube::vertex_indices_count_uv
    );
}

int RegisterCubeMesh1Part(int diffuse_texture_id, int specular_texture_id, int emission_texture_id = -1) {
    return Renderer_RegisterTexturedMesh(
        diffuse_texture_id,
        specular_texture_id,
        emission_texture_id,
        cube::vertex_data_uv_1_part_texture,
        cube::vertices_count_uv,
        cube::vertex_indices_uvs,
        cube::vertex_indices_count_uv
    );
}

int RegisterQuadMesh1Part(int diffuse_texture_id, int specular_texture_id, int emission_texture_id = -1) {
    return Renderer_RegisterTexturedMesh(
        diffuse_texture_id,
        specular_texture_id,
        emission_texture_id,
        quad::vertex_data_uv_1_part_texture_single_faced,
        quad::vertices_count_uv_single_faced,
        quad::vertex_indices_uvs_single_faced,
        quad::vertex_indices_count_uv_single_faced
    );
}

int RegisterQuadMesh2Part(int diffuse_texture_id, int specular_texture_id, int emission_texture_id = -1) {
    return Renderer_RegisterTexturedMesh(
        diffuse_texture_id,
        specular_texture_id,
        emission_texture_id,
        quad::vertex_data_uv_1_part_texture,
        quad::vertices_count_uv,
        quad::vertex_indices_uvs,
        quad::vertex_indices_count_uv
    );
}

int compare_models_descending(const void *p, const void *q) {
    const model_instance m1 = *(const model_instance *) p;
    const model_instance m2 = *(const model_instance *) q;

    const float distance1 = magnitude_squared(SceneCamera->position - m1.transform.Position);
    const float distance2 = magnitude_squared(SceneCamera->position - m2.transform.Position);

    return (distance1 < distance2) - (distance1 > distance2);
}

void sort_objects_based_on_camera_distance(model_instance *models, const size_t number) {
    qsort(models, number, sizeof(model_instance), compare_models_descending);
}

int main() {
    try {
        constexpr int game_resolution_width = 800, game_resolution_height = 600;
        Renderer_Init(game_resolution_width, game_resolution_height, 45, 0.1, 100, 8);

        const int whiteTextureId = Renderer_RegisterTexture("data/textures/white_texture.png", {.convert_from_srgb_to_linear_space = true});
        const int woodTextureId = Renderer_RegisterTexture("data/textures/wood.png", {.convert_from_srgb_to_linear_space = true});
        const int grassTextureId = Renderer_RegisterTexture("data/textures/grass_block.png", {.convert_from_srgb_to_linear_space = true});
        const int mushroomTextureId = Renderer_RegisterTexture("data/textures/mushroom_red.png", {.convert_from_srgb_to_linear_space = true});
        const int crate_Texture_diffuse_id = Renderer_RegisterTexture("data/textures/box_container.png", {.convert_from_srgb_to_linear_space = true});
        const int crate_Texture_specular_id = Renderer_RegisterTexture("data/textures/box_container_specular.png"); // specular stays as is
        const int tranrsparent_window_texture = Renderer_RegisterTexture(
            "data/textures/blending_transparent_window.png",
            {
                TextureWrapMode::CLAMP_TO_EDGE,
                TextureWrapMode::CLAMP_TO_EDGE,
                true
            }
        );

        // if our problem is wrapping cubes and data like that
        // we can probably have the API acknowledge that
        const int grass_cube_id = RegisterCubeMesh3Part(
            grassTextureId, grassTextureId
        );

        const int white_cube_id = RegisterCubeMesh3Part(
            whiteTextureId, whiteTextureId
        );

        const int wood_cube_id = RegisterCubeMesh1Part(woodTextureId, woodTextureId);

        const int crate_cube_id = RegisterCubeMesh1Part(crate_Texture_diffuse_id, crate_Texture_specular_id);

        const int glass_cube_id = RegisterQuadMesh2Part(tranrsparent_window_texture, tranrsparent_window_texture);

        const int mushroom_cube_id = Renderer_RegisterTextured_Cross_Mesh(
            mushroomTextureId,
            0.5f
        );


        const int light_source = Renderer_RegisterUnshadedTexture(
            cube::vertex_data,
            cube::vertices_count,
            cube::vertex_indices,
            cube::indices_count
        );


        //const int cube_id_5=RegisterCubeMesh3Part(cubeId_1);

        // loads a model . skip for now because this is slow as fuck
        //const int back_pack_model = Renderer_Register_Model("data/models/backpack/backpack.obj");


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

        constexpr float centerX = static_cast<float>(game_resolution_width) / 2.0f;
        constexpr float centerY = static_cast<float>(game_resolution_height) / 2.0f;
        const std::array cursor{
            normalize_coord(centerX - 10, static_cast<float>(game_resolution_width)),
            normalize_coord(centerY, static_cast<float>(game_resolution_height)),
            1.0f,

            normalize_coord(centerX + 10, static_cast<float>(game_resolution_width)),
            normalize_coord(centerY, static_cast<float>(game_resolution_height)),
            1.0f,

            normalize_coord(centerX, static_cast<float>(game_resolution_width)),
            normalize_coord(centerY - 10, static_cast<float>(game_resolution_height)),
            1.0f,

            normalize_coord(centerX, static_cast<float>(game_resolution_width)),
            normalize_coord(centerY + 10, static_cast<float>(game_resolution_height)),
            1.0f,
        };


        unsigned int cursor_vao;
        unsigned int cursor_vbo;
        InitializeCursorVBO(cursor, cursor_vao, cursor_vbo);


        Vector3D light_color = {1, 1, 1};


        Vector_model_instance *opaque_models = Vector_model_instance_Create(30);
        Vector_model_instance *transparent_models = Vector_model_instance_Create(30);

        // // Grid of cubes για να δεις το spotlight
        // for (int x = -3; x <= 3; x++) {
        //     for (int y = 0; y <= 10; y++) {
        //         models.push_back({
        //             crate_cube_id,
        //             Vector3D{x * 2.0f, y * 2.0f, 0}
        //         });
        //     }
        // }

        for (int x = 0; x < 10; ++x) {
            for (int y = 0; y < 10; ++y) {
                Vector_model_instance_Add(opaque_models, {wood_cube_id, {(float) x, (float) y + 2, -1}});
            }
        }

        // Target cube πιο μακριά
        // Vector_model_instance_Add(opaque_models, {crate_cube_id, {0, 2, 0}});
        // Vector_model_instance_Add(opaque_models, {crate_cube_id, {1, 2, 0}});
        // Vector_model_instance_Add(opaque_models, {crate_cube_id, {1, -2, 0}});

        // Vector_model_instance_Add(transparent_models, {glass_cube_id, {0, 1.5, 0}});
        // Vector_model_instance_Add(transparent_models, {glass_cube_id, {0.5, 1, 0}});

        Material our_material{
            2
        };

        //         PointLight our_light{
        //             Vector3D{-5, 5, 2}, // Αντίθετη πλευρά από την κάμερα
        // {0.03f, 0.03f, 0.03f},  // ambient - πολύ χαμηλό
        // {0.8f, 0.8f, 0.8f},      // diffuse - δυνατό, τώρα θα φαίνεται σωστά
        // {1.0f, 1.0f, 1.0f},      // specular
        // 0.045f,                   // linear - χαμηλότερο, πιο soft falloff
        // 0.0075f 
        //         };


        Vector3D spotLightpos = SceneCamera->position;
        spotLightpos.z += 0.5f;
        SpotLight our_spot_light{
            {5, 5, 3},
            {0, 0, -1},
            {0.1f, 0.1f, 0.1f}, // Καθόλου ambient (ήταν 0.01)
            {0.8f, 0.8f, 0.8f}, // Πολύ πιο δυνατό (ήταν 0.3)
            {0.1f, 0.1f, 0.1f},
            0.09f,
            0.032f,
            (float) cos(DegreeToRadians(12.5f)),
            (float) cos(DegreeToRadians(17.5f)),
        };
        // DirectionalLight our_dir_light{
        //     {0, -1, 0.2f}, // Από πάνω προς τα κάτω
        //     {0.1f, 0.1f, 0.1f}, // Minimal ambient (ήταν 0.1)
        //     {0.15f, 0.15f, 0.15f}, // Πολύ αχνό diffuse (ήταν 0.7)
        //     {0.3f, 0.3f, 0.3f}
        // };

        DirectionalLight our_dir_light{
            {0.8f, 0.2f, -0.5f},
            {0.01f, 0.01f, 0.015f}, // ambient - ελάχιστο, μόνο για να μην είναι pitch black
            {0.08f, 0.08f, 0.1f}, // diffuse - πολύ αδύναμο fill, σαν indirect light
            {0.0f, 0.0f, 0.0f}
        };

        PointLight our_light{
            Vector3D{5, 5, 1}, // κέντρο-ish του scene
            {0.0f, 0.0f, 0.0f},
            {1.0f, 0.9f, 0.7f}, // warm λάμπα
            {1.0f, 1.0f, 1.0f},
            0.22f, // aggressive linear falloff
            0.20f // aggressive quadratic
        };

        Renderer_Register_Point_Light(our_light);
        //Renderer_Register_Spot_Light(our_spot_light);
        Renderer_Register_Directional_Light(our_dir_light);


        model_instance lights[]{
            light_source, {our_light.position},
        };

        Renderer_FinalizeMeshLoading();

        bool keepRunning = true;

        bool matrix = true;
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


            for (const auto &m: lights) {
                Renderer_DrawUnshadedTexture(m.mesh_id, m.transform, light_color);
            }

            for (size_t i = 0; i < Vector_model_instance_Length(opaque_models); ++i) {
                const model_instance m = opaque_models->data[i];
                Renderer_Draw(m.mesh_id, m.transform, {1, 1, 1}, our_material);
            }

            // for (size_t i = 0; i < Vector_model_instance_Length(opaque_models); ++i) {
            //     const model_instance m = opaque_models->data[i];
            //     Renderer_Draw_Outline(m.mesh_id, m.pos, {1, 1, 1}, our_material);
            // }
            //
            //
            // sort_objects_based_on_camera_distance(transparent_models->data, Vector_model_instance_Length(transparent_models));
            //
            // for (size_t i = 0; i < Vector_model_instance_Length(transparent_models); ++i) {
            //     const model_instance m = transparent_models->data[i];
            //     Renderer_Draw(m.mesh_id, m.pos, {1, 1, 1}, our_material);
            // }

            //Renderer_Draw_Model(back_pack_model, {1, 5, 1}, {0, 0, 0}, {32});
            //Renderer_Draw_Model_Outline(back_pack_model, {1, 5, 1}, {0, 0, 0}, {32});

            Draw_Cursor(cursorProgram, cursor_vao);

            // // Start the Dear ImGui frame
            // ImGui_ImplOpenGL3_NewFrame();
            // ImGui_ImplSDL3_NewFrame();
            // ImGui::NewFrame();
            //
            // IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "Missing Dear ImGui context. Refer to examples app!");
            //
            // // Verify ABI compatibility between caller code and compiled version of Dear ImGui. This helps detects some build issues.
            // IMGUI_CHECKVERSION();

            //float *p_as_float3 = reinterpret_cast<float *>(&lights[0].pos);

            // if (ImGui::Button("Matrixxx")) {
            //     const int value = matrix ? -1 : crate_Texture_emission_id;
            //     matrix = !matrix;
            //     Renderer_Change_Emission(crate_cube_id, value);
            // }
            //
            // if (ImGui::InputFloat3("test", p_as_float3, "%.3f")
            // ) {
            //     lights[0].pos = *reinterpret_cast<Vector3D *>(p_as_float3);
            //     our_light.position = lights[0].pos;
            // }
            // Show demo window! :)

            // ImGui::Render();
            // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
            // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            //     SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
            //     const SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext(); // NOLINT(*-misplaced-const) , we want this as is
            //     ImGui::UpdatePlatformWindows();
            //     ImGui::RenderPlatformWindowsDefault();
            //     SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
            // }


            Renderer_FrameEnd();
        }

        glDeleteVertexArrays(1, &cursor_vao);
        glDeleteBuffers(1, &cursor_vbo);

        Renderer_Destroy();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    } catch (const std::exception &e) {
        fprintf(stderr, "Fatal error: %s", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}
