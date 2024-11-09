#include <fstream>
#include <GL/glew.h>
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

#include "data/cube.h"


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
            case GLFW_KEY_D: {
                MoveCameraX(appData->camera, 1);
                break;
            }
            case GLFW_KEY_A: {
                MoveCameraX(appData->camera, -1);
                break;
            }
            case GLFW_KEY_SPACE: {
                MoveCameraY(appData->camera,1);
                break;
            }
            case GLFW_KEY_LEFT_SHIFT: {
                MoveCameraY(appData->camera,-1);
                break;
            }
            case GLFW_KEY_RIGHT: {
                RotateCamera(appData->camera,1,0);
                break;
            }
            case GLFW_KEY_LEFT: {
                RotateCamera(appData->camera,-1,0);
                break;
            }
            case GLFW_KEY_UP: {
                RotateCamera(appData->camera,0,1);
                break;
            }
            case GLFW_KEY_DOWN: {
                RotateCamera(appData->camera,0,-1);
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
        std::cout<<"camera pos : "<<appData->camera.position<< std::endl;
    }
}


void window_size_callback(GLFWwindow *window, const int width, const int height) {
    auto *appData = static_cast<AppData *>(glfwGetWindowUserPointer(window));
    const float new_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    PerspectiveMatrixUpdate(appData->perspective_matrix, appData->FOV, new_aspect_ratio);
    appData->view_dirty = true;
    glViewport(0, 0, width, height);
}

void Draw_Cursor(unsigned int cursorProgram, unsigned int cursor_vao) {
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

    GLenum err=glewInit();
    if(err!=GLEW_OK) {
        std::cerr << "glewInitFailed: " << glewGetErrorString(err) <<std::endl;
        return -1;
    }

    glm::mat4 world_space_matrix(1);

    world_space_matrix[1] = glm::vec4(0, 0, -1, 0);
    world_space_matrix[2] = glm::vec4(0, 1, 0, 0);
    
    int screenWidth = 800, screenHeight = 600;
    appData.FOV = 45;
    appData.perspective_matrix = PerspectiveMatrix(appData.FOV, 0.1f, 100.f,
                                                   static_cast<float>(screenWidth) / static_cast<float>(screenHeight));
    //RotateCamera(appData.camera,0);
    appData.camera_matrix = CameraLookAtMatrix(appData.camera);

    
    glm::mat4 triangle_model_view_space(1);
    triangle_model_view_space=glm::translate(triangle_model_view_space,glm::vec3(2,5,0));
    triangle_model_view_space=glm::mat4(1);
    
    


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
    unsigned int shaderProgram = InitializeProgram("program1");
    unsigned int cursorProgram = InitializeProgram("cursor_program");

    GLint perspectiveMatrixUnif = glGetUniformLocation(shaderProgram, "perspectiveMatrix");
    GLint cursor_color_uniform = glGetUniformLocation(cursorProgram, "cursor_color");
    GLint voxel_color = glGetUniformLocation(shaderProgram, "voxel_color");


    glm::vec4 color(1, 0, 0, 1);
    glUseProgram(cursorProgram);
    glUniform4fv(cursor_color_uniform, 1, glm::value_ptr(color));


    cube my_cube;

    float centerX = static_cast<float>(screenWidth) / 2.0f;
    float centerY = static_cast<float>(screenHeight) / 2.0f;
    std::array cursor{
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
    glm::vec4 r(0.5, 0.5, 0.5, 1);
    glUniform4fv(voxel_color, 1, glm::value_ptr(r));
    glUniformMatrix4fv(perspectiveMatrixUnif, 1,GL_FALSE, glm::value_ptr(appData.view_projection_matrix));

    glfwSetWindowSizeCallback(window, window_size_callback);


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

    // Rendering loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  


        glUseProgram(shaderProgram);
        glBindVertexArray(world_vao);

        //triangle_model_view_space=glm::rotate(triangle_model_view_space,glm::radians(1.f), glm::vec3(0,0,1));
        
        //appData.view_dirty=true;
        if (appData.view_dirty) {
            appData.view_projection_matrix = appData.perspective_matrix * appData.camera_matrix * world_space_matrix
                                             * triangle_model_view_space;

            glUniformMatrix4fv(perspectiveMatrixUnif, 1,GL_FALSE, glm::value_ptr(appData.view_projection_matrix));
            appData.view_dirty = false;
        }

        glDrawElements(GL_TRIANGLES, 36,GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glUseProgram(0);

        Draw_Cursor(cursorProgram, cursor_vao);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &world_vao);
    glDeleteBuffers(1, &cubes_vbo);
    glDeleteVertexArrays(1, &cursor_vao);
    glDeleteBuffers(1, &cursor_vbo);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}
