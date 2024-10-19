#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>  // This is needed for glm::value_ptr

#include "Camera.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>


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
unsigned int compileShader(unsigned int type, const std::string &source) {
    const unsigned int id = glCreateShader(type);
    auto source_c_str = source.c_str();
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
        fprintf(stderr, "%s\n", e.what());
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

struct AppData {
    Camera camera{};
    glm::mat4 camera_matrix{};
    glm::mat4 perspective_matrix{};
    glm::mat4 view_projection_matrix{};
    float FOV = 45;
    bool view_dirty{};
};


void camera_input_handling(GLFWwindow *window, const int key, int, const int action, int) {
    auto *appData = static_cast<AppData *>(glfwGetWindowUserPointer(window));

    bool camera_changed = false;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        camera_changed = true;
        switch (key) {
            case GLFW_KEY_W: {
                MoveCameraZ(appData->camera, 1);
                break;
            }
            case GLFW_KEY_S: {
                MoveCameraZ(appData->camera, -1);
                break;
            }
            case GLFW_KEY_A: {
                MoveCameraX(appData->camera, -1);
                break;
            }
            case GLFW_KEY_D: {
                MoveCameraX(appData->camera, 1);
                break;
            }
            default: {
                camera_changed = false;
                break;
            }
        }
    }

    if (camera_changed) {
        appData->camera_matrix = CameraLookAtMatrix(appData->camera);
        appData->view_dirty = true;
    }
}


void window_size_callback(GLFWwindow *window, const int width, const int height) {
    auto *appData = static_cast<AppData *>(glfwGetWindowUserPointer(window));
    const float new_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    PerspectiveMatrixUpdate(appData->perspective_matrix, appData->FOV, new_aspect_ratio);
    appData->view_dirty = true;
    glViewport(0, 0, width, height);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    GLFWwindow *window = glfwCreateWindow(800, 600, "Hello World - VAO and VBO", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    AppData appData;
    glfwSetWindowUserPointer(window, &appData);

    glfwSetKeyCallback(window, camera_input_handling);
    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    int screenWidth = 800, screenHeight = 600;
    appData.FOV = 45;
    appData.perspective_matrix = PerspectiveMatrix(appData.FOV, 0.1f, 100.f,
                                                   static_cast<float>(screenWidth) / static_cast<float>(screenHeight));
    appData.camera_matrix = CameraLookAtMatrix(appData.camera);

    glm::mat4 triangle_model_view_space=glm::translate(glm::mat4(1),glm::vec3(0,0,1));
    
    appData.view_projection_matrix = appData.perspective_matrix * appData.camera_matrix * triangle_model_view_space;
    glViewport(0, 0, 800, 600);


    // Build and compile shader program
    unsigned int shaderProgram = InitializeProgram("program1");
    unsigned int cursorProgram = InitializeProgram("cursor_program");

    GLint perspectiveMatrixUnif = glGetUniformLocation(shaderProgram, "perspectiveMatrix");
    GLint cursor_color_uniform = glGetUniformLocation(cursorProgram, "cursor_color");


    glm::vec4 color(1, 0, 0, 1);

    glUseProgram(cursorProgram);
    glUniform4fv(cursor_color_uniform, 1, glm::value_ptr(color));

    // Set up vertex data (triangle)
    float triangle_in_local_space[] = {
        1, 0, 0.f,
        0.5f, 1, 0.f,
        0, 0, 0.f
    };
    
    float centerX = static_cast<float>(screenWidth) / 2.0f;
    float centerY = static_cast<float>(screenHeight) / 2.0f;
    float cursor[]{
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
    glUniformMatrix4fv(perspectiveMatrixUnif, 1,GL_FALSE, glm::value_ptr(appData.view_projection_matrix));

    glfwSetWindowSizeCallback(window, window_size_callback);

    // Create and bind a VAO and VBO
    unsigned int world_vao, world_vbo;

    glGenVertexArrays(1, &world_vao);
    glGenBuffers(1, &world_vbo);


    // Bind VAO
    glBindVertexArray(world_vao);

    // Bind and set VBO
    glBindBuffer(GL_ARRAY_BUFFER, world_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_in_local_space), triangle_in_local_space, GL_STATIC_DRAW);

    // Define the vertex attributes (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    // Unbind the VAO
    glBindVertexArray(0);


    unsigned int cursor_vao, cursor_vbo;
    glGenVertexArrays(1, &cursor_vao);
    glGenBuffers(1, &cursor_vbo);

    glBindVertexArray(cursor_vao);

    // Bind and set VBO
    glBindBuffer(GL_ARRAY_BUFFER, cursor_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cursor), cursor, GL_STATIC_DRAW);

    // Define the vertex attributes (position)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Unbind the VAO
    glBindVertexArray(0);

    // Rendering loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(world_vao);
        glfwGetWindowSize(window, &screenWidth, &screenHeight);
        if (appData.view_dirty) {
            appData.view_projection_matrix = appData.perspective_matrix * appData.camera_matrix * triangle_model_view_space;
            glUniformMatrix4fv(perspectiveMatrixUnif, 1,GL_FALSE, glm::value_ptr(appData.view_projection_matrix));
            appData.view_dirty = false;
        }
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glUseProgram(0);

        glUseProgram(cursorProgram);
        glBindVertexArray(cursor_vao);
        glDrawArrays(GL_LINES, 0, 4);
        glBindVertexArray(0);
        glUseProgram(0);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &world_vao);
    glDeleteBuffers(1, &world_vbo);
    glDeleteVertexArrays(1, &cursor_vao);
    glDeleteBuffers(1, &cursor_vbo);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}
