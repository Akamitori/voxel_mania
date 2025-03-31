#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
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

const std::string LOCAL_FILE_DIR("data/");
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
    const float new_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    PerspectiveMatrixUpdate(app_data.perspective_matrix, app_data.FOV, new_aspect_ratio);
    app_data.view_dirty = true;
    glViewport(0, 0, width, height);
}

void Draw_Cursor(const unsigned int cursorProgram, const unsigned int cursor_vao) {
    glUseProgram(cursorProgram);
    glBindVertexArray(cursor_vao);
    glDrawArrays(GL_LINES, 0, 8);
    glBindVertexArray(0);
    glUseProgram(0);
}

void InitializeCursorVBO(const std::array<float, 8> &cursor, unsigned int &cursor_vao, unsigned int &cursor_vbo) {
    glGenVertexArrays(1, &cursor_vao);
    glGenBuffers(1, &cursor_vbo);

    glBindVertexArray(cursor_vao);

    // Bind and set VBO
    glBindBuffer(GL_ARRAY_BUFFER, cursor_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cursor.size(), cursor.data(), GL_STATIC_DRAW);

    // Define the vertex attributes (position)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Unbind the VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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


    constexpr int screenWidth = 800, screenHeight = 600;
    // Create window
    SDL_Window *window = SDL_CreateWindow("Hello World - VAO and VBO", screenWidth, screenHeight,
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

    Matrix4D world_space_matrix(1);

    world_space_matrix[1] = Vector4D(0, 0, 1, 0);
    world_space_matrix[2] = Vector4D(0, -1, 0, 0);
    world_space_matrix = translate(world_space_matrix, Vector3D(0, 4, 0));

    AppData appData;
    appData.view_dirty = true;
    appData.FOV = 45;
    appData.perspective_matrix = PerspectiveMatrix(appData.FOV, 0.1f, 100.f,
                                                   static_cast<float>(screenWidth) / static_cast<float>(screenHeight));

    appData.camera_matrix = CameraLookAtMatrix(appData.camera);

    const Matrix4D triangle_model_view_space(1);
    appData.view_projection_matrix = appData.perspective_matrix * appData.camera_matrix * world_space_matrix *
                                     triangle_model_view_space;
    glViewport(0, 0, 800, 600);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);


    // Build and compile shader program
    const unsigned int shaderProgram = InitializeProgram("program1");
    const unsigned int cursorProgram = InitializeProgram("cursor_program");

    const GLint perspectiveMatrixUnif = glGetUniformLocation(shaderProgram, "perspectiveMatrix");
    const GLint cursor_color_uniform = glGetUniformLocation(cursorProgram, "cursor_color");
    const GLint voxel_color = glGetUniformLocation(shaderProgram, "voxel_color");
    
    constexpr Vector4D color(1, 0, 0, 1);
    glUseProgram(cursorProgram);
    glUniform4fv(cursor_color_uniform, 1, &color.x);

    const cube my_cube;

    constexpr float centerX = static_cast<float>(screenWidth) / 2.0f;
    constexpr float centerY = static_cast<float>(screenHeight) / 2.0f;
    const std::array cursor{
        normalize_coord(centerX - 10, static_cast<float>(screenWidth)),
        normalize_coord(centerY, static_cast<float>(screenHeight)),
        normalize_coord(centerX + 10, static_cast<float>(screenWidth)),
        normalize_coord(centerY, static_cast<float>(screenHeight)),

        normalize_coord(centerX, static_cast<float>(screenWidth)),
        normalize_coord(centerY - 10, static_cast<float>(screenHeight)),
        normalize_coord(centerX, static_cast<float>(screenWidth)),
        normalize_coord(centerY + 10, static_cast<float>(screenHeight))
    };

    glUseProgram(shaderProgram);
    constexpr Vector4D r(0.5, 0.5, 0.5, 1);
    glUniform4fv(voxel_color, 1, &r.x);
    glUniformMatrix4fv(perspectiveMatrixUnif, 1,GL_FALSE, &appData.view_projection_matrix[0].x);

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
        }
        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(world_vao);

        if (appData.view_dirty) {
            appData.view_projection_matrix = appData.perspective_matrix * appData.camera_matrix * world_space_matrix
                                             * triangle_model_view_space;

            glUniformMatrix4fv(perspectiveMatrixUnif, 1,GL_FALSE, &appData.view_projection_matrix[0].x);
            appData.view_dirty = false;
        }

        glDrawElements(GL_TRIANGLES, 36,GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);

        Draw_Cursor(cursorProgram, cursor_vao);

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
    return 0;
}
