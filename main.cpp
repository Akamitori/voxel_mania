#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <Bounding.h>
#include <Clipping.h>
#include <format>
#include <Transformations.h>

#include <GL/glew.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_init.h>

#include "Camera.h"
#include "AppData.h"
#include "InputHandling.h"
#include "data/cube.h"
#include "Matrix4D.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "Trigonometry.h"

const std::string LOCAL_FILE_DIR("data/");
constexpr int initial_screen_width = 800, initial_screen_height = 600;
// Vertex Shader source code

std::string FindFileOrThrow(const std::string &strBasename, const std::string &program_id) {
    std::string strFilename = LOCAL_FILE_DIR + program_id + '/' + strBasename;
    const std::ifstream testFile(strFilename.c_str());
    if (testFile.is_open())
        return strFilename;

    std::cerr << "Could not find the file " + strBasename + " for program with id : " + program_id;
    throw;
}

// Function to compile shaders
unsigned int compileShader(const unsigned int type, const std::string &source) {
    const unsigned int id = glCreateShader(type);
    const auto source_c_str = source.c_str();
    glShaderSource(id, 1, &source_c_str, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed\n" << infoLog << std::endl;
    }

    return id;
}

GLuint LoadShader(const GLenum eShaderType, const std::string &strShaderFilename, const std::string &programId) {
    const std::string strFilename = FindFileOrThrow(strShaderFilename, programId);
    std::ifstream shaderFile(strFilename.c_str());
    std::stringstream shaderData;
    shaderData << shaderFile.rdbuf();
    shaderFile.close();

    try {
        return compileShader(eShaderType, shaderData.str());
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
}


// Function to link vertex and fragment shaders into a program
unsigned int InitializeProgram(const std::string &program_id) {
    std::vector<GLuint> shaders;

    shaders.push_back(LoadShader(GL_FRAGMENT_SHADER, "frag.frag", program_id));
    shaders.push_back(LoadShader(GL_VERTEX_SHADER, "vert.vert", program_id));
    GLuint shaderProgram = glCreateProgram();

    std::ranges::for_each(shaders, [shaderProgram](const GLuint &s_id) {
        glAttachShader(shaderProgram, s_id);
    });
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed\n" << infoLog << std::endl;
    }


    std::ranges::for_each(shaders,glDeleteShader);

    return shaderProgram;
}

float normalize_coord(const float value, const float max) {
    return 2 * value / max - 1;
}

void handle_window_resize(AppData &app_data, const int width, const int height) {
    glViewport(0, 0, width, height);
    const float new_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    PerspectiveMatrixUpdate(app_data.perspective_matrix, app_data.FOV, new_aspect_ratio);
    app_data.view_dirty = true;
    app_data.screen_height = height;
    app_data.screen_width = width;
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

void calculate_frustum_planes_and_check_for_visibility(const Matrix4D &world_space_matrix, AppData &appData,
                                                       const Matrix4D &triangle_model_view_space,
                                                       const cube &my_cube) {
    Vector3D vertex_data[8];

    build_frustum_polyhedron(
        inverse(appData.camera_matrix),
        1 / tan(DegreeToRadians(appData.FOV) * 0.5f),
        static_cast<float>(appData.screen_width) / static_cast<float>(appData.screen_height),
        appData.z_near, appData.z_far, &appData.frustum_polyhedron);

    for (int i = 0, j = 0; i < 8 && j < 24; i++, j += 3) {
        auto point = Vector3D{
            my_cube.vertex_data[j],
            my_cube.vertex_data[j + 1],
            my_cube.vertex_data[j + 2]
        };

        // get the cube to global space
        vertex_data[i] = transform_point(world_space_matrix, transform_point(triangle_model_view_space, point));
    }

    // check if the cube is contained in the frustum
    auto a = calculate_axis_aligned_bounding_box(8, vertex_data);
    auto result = AxisAlignedBoxVisible(6, appData.frustum_polyhedron.plane, a);
    std::cout << "Box contained : " << result << std::endl;
}

int main() {
    // Initialize GLFW
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


    // Create window
    SDL_Window *window = SDL_CreateWindow("Hello World - VAO and VBO", initial_screen_width, initial_screen_height,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Failed to create SDL window. Error :" << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    const SDL_GLContext open_gl_context = SDL_GL_CreateContext(window);

    if (!open_gl_context) {
        std::cerr << "Failed to create OpenGL context. Error : " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    if (const GLenum err = glewInit(); err != GLEW_OK) {
        std::cerr << "glewInitFailed: " << glewGetErrorString(err) << std::endl;

        SDL_GL_DestroyContext(open_gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

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

    // left in just for the theory. in practice we don't need it ;)
    Matrix4D world_space_matrix(1);

    AppData appData;
    appData.screen_width = 800;
    appData.screen_height = 600;
    appData.view_dirty = true;
    appData.FOV = 45;
    appData.z_near = 0.1;
    appData.z_far = 100;
    appData.perspective_matrix = PerspectiveMatrix(appData.FOV, appData.z_near, appData.z_far,
                                                   static_cast<float>(appData.screen_width) / static_cast<float>(appData
                                                       .screen_height));

    appData.ortho_projection_matrix = MakeOrthoProjection(-10, 10, 0, 10, -20, 20);

    appData.camera_matrix = CameraLookAtMatrix(appData.camera);
    appData.look_at_matrix_inverse = inverse(appData.camera_matrix);

    // TODO move triangle_model_view_space out of the view projection matrix at some point
    Matrix4D triangle_model_view_space(1);
    appData.view_projection_matrix = appData.perspective_matrix * appData.camera_matrix * world_space_matrix *
                                     triangle_model_view_space;
    glViewport(0, 0, appData.screen_width, appData.screen_height);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // we should map values that are further away to the lower part of [0,1] for better precision
    // this is because [0,1/2] has better precision from [1/2,1] due to the nature of floating point numbers
    // in practice -> higher values => closer to the camera
    glDepthFunc(GL_GEQUAL);
    glDepthRange(0.0f, 1.0f);

    // open gl should expect depth values from [0,1]
    // for the above depth mapping to work properly on the projection level
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

    //glDisable(GL_DEPTH_TEST);


    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    // Build and compile shader program
    const unsigned int world_geometry_program = InitializeProgram("program1");
    const unsigned int cursorProgram = InitializeProgram("cursor_program");

    const GLint world_perspectiveMatrixUnif = glGetUniformLocation(world_geometry_program, "perspectiveMatrix");
    const GLint cursor_color_uniform = glGetUniformLocation(cursorProgram, "cursor_color");
    const GLint voxel_color = glGetUniformLocation(world_geometry_program, "voxel_color");

    Vector4D cursor_color(1, 0, 0, 1);
    glUseProgram(cursorProgram);
    glUniform4fv(cursor_color_uniform, 1, &cursor_color.x);

    const float centerX = static_cast<float>(appData.screen_width) / 2.0f;
    const float centerY = static_cast<float>(appData.screen_height) / 2.0f;
    const std::array cursor{
        normalize_coord(centerX - 10, static_cast<float>(appData.screen_width)),
        normalize_coord(centerY, static_cast<float>(appData.screen_height)),
        -1.0f,

        normalize_coord(centerX + 10, static_cast<float>(appData.screen_width)),
        normalize_coord(centerY, static_cast<float>(appData.screen_height)),
        -1.0f,

        normalize_coord(centerX, static_cast<float>(appData.screen_width)),
        normalize_coord(centerY - 10, static_cast<float>(appData.screen_height)),
        -1.0f,

        normalize_coord(centerX, static_cast<float>(appData.screen_width)),
        normalize_coord(centerY + 10, static_cast<float>(appData.screen_height)),
        -1.0f,
    };


    cube my_cube{};

    glUseProgram(world_geometry_program);
    constexpr Vector4D r(0.5, 0.5, 0.5, 1);
    glUniform4fv(voxel_color, 1, &r.x);

    GLuint world_vao;
    glGenVertexArrays(1, &world_vao);
    glBindVertexArray(world_vao);

    GLuint cubes_vbo;
    glGenBuffers(1, &cubes_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cubes_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(my_cube.vertex_data), my_cube.vertex_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);


    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint cubes_vbe;
    glGenBuffers(1, &cubes_vbe);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubes_vbe);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(my_cube.vertex_indices), my_cube.vertex_indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    unsigned int cursor_vao;
    unsigned int cursor_vbo;
    InitializeCursorVBO(cursor, cursor_vao, cursor_vbo);

    bool keepRunning = true;
    // Rendering loop
    while (keepRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_WINDOW_RESIZED: {
                    handle_window_resize(appData, event.window.data1, event.window.data2);
                    break;
                }
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                    keepRunning = false;
                    break;
                }
                case SDL_EVENT_KEY_DOWN: {
                    KeyDown(event.key.scancode, appData);
                    break;
                }
                default: {
                    break;
                }
            }

            ImGui_ImplSDL3_ProcessEvent(&event); // Forward your event to backend
        }
        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        // set the clear value to the value assosiated with the "furthest" object
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        if (appData.view_dirty) {
            appData.view_projection_matrix = appData.perspective_matrix * appData.camera_matrix * world_space_matrix *
                                             triangle_model_view_space;

            glUseProgram(world_geometry_program);
            glBindVertexArray(world_vao);
            glUniformMatrix4fv(world_perspectiveMatrixUnif, 1,GL_FALSE, &appData.view_projection_matrix[0].x);
            appData.view_dirty = false;

            calculate_frustum_planes_and_check_for_visibility(world_space_matrix, appData, triangle_model_view_space,
                                                              my_cube);
        }

        glUseProgram(world_geometry_program);
        glBindVertexArray(world_vao);
        glDrawElements(GL_TRIANGLES, 36,GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);

        Draw_Cursor(cursorProgram, cursor_vao);

        // (Where your code calls SDL_PollEvent())


        // (After event loop)
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing Dear ImGui context. Refer to examples app!");

        // Verify ABI compatibility between caller code and compiled version of Dear ImGui. This helps detects some build issues.
        IMGUI_CHECKVERSION();

        Vector4D &t = triangle_model_view_space.column_vectors[3];
        float *p_as_float3 = reinterpret_cast<float *>(&t);


        if (ImGui::InputFloat3("test", p_as_float3, "%.3f")
        ) {
            appData.view_dirty = true;
        }
        // Show demo window! :)

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &world_vao);
    glDeleteBuffers(1, &cubes_vbo);
    glDeleteVertexArrays(1, &cursor_vao);
    glDeleteBuffers(1, &cursor_vbo);

    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(open_gl_context);
    SDL_Quit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
