#include "ShaderLoader.h"

#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <GL/glew.h>


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