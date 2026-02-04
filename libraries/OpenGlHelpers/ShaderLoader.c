#include "ShaderLoader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <GL/glew.h>


const char *LOCAL_FILE_DIR = "data/";
const int LOCAL_FILE_DIR_LEN = sizeof(LOCAL_FILE_DIR) - 1;

// Function to compile shaders
unsigned int compileShader(const unsigned int type, const char *source, char *info_log, const int info_log_size) {
    const unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        int required_log_size = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &required_log_size);

        char *error_buffer = malloc(sizeof(char) * required_log_size);
        glGetShaderInfoLog(id, required_log_size, NULL, error_buffer);


        const char *prefix = "Shader compilation failed:\n";
        const int prefix_len = (int) strlen(prefix);
        
        int max_error_characters = info_log_size - prefix_len - 1;

        if (max_error_characters <= 0) {
            max_error_characters = 0;
        }

        if (max_error_characters >= required_log_size) {
            snprintf(info_log, info_log_size, "%s%s", prefix, error_buffer);
        } else {
            const char *suffix = "...[TRUNCATED]";
            const int suffix_len = (int)strlen(suffix);
            int error_chars_to_copy = max_error_characters - suffix_len;

            if (error_chars_to_copy <= 0) {
                error_chars_to_copy = 0;
            }

            snprintf(info_log, info_log_size, "%s%.*s%s", prefix, error_chars_to_copy, error_buffer, suffix);
        }

        glDeleteShader(id);
        free(error_buffer);
        return 0;
    }

    return id;
}

GLuint LoadShader(const GLenum eShaderType, const char *strShaderFilename, const char *programId, char *info_log,
                  const int info_log_size) {
    const size_t file_name_buffer_size = strlen(strShaderFilename) + strlen(programId) + LOCAL_FILE_DIR_LEN + 2;
    char *full_file_path = (char *) malloc(sizeof(char) * file_name_buffer_size);

    GLuint shader_id = 0;

    if (!full_file_path) {
        sprintf(info_log, "Failed to allocated text buffer for %s", strShaderFilename);
        goto EXIT;
    }

    strcpy(full_file_path, LOCAL_FILE_DIR);
    strcat(full_file_path, programId);
    strcat(full_file_path, "/");
    strcat(full_file_path, strShaderFilename);

    FILE *fp = fopen(full_file_path, "rb");

    if (fp == NULL) {
        sprintf(info_log, "Cannot open %s(at %s): %s", strShaderFilename, full_file_path, strerror(errno));
        goto CLEAN_FULL_FILE_PATH;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        sprintf(info_log, "Seek to end of file failed");
        goto CLOSE_FILE;
    }

    long file_size = ftell(fp);
    if (file_size == -1L) {
        sprintf(info_log, "ftell failed");
        goto CLOSE_FILE;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        sprintf(info_log, "ftell failed");
        goto CLOSE_FILE;
    }

    char *total_buffer = (char *) malloc(file_size + 1);
    if (!total_buffer) {
        sprintf(info_log, "Failed to allocate buffer for reading shader: %s", full_file_path);
        goto CLOSE_FILE;
    }


    size_t read_size = fread(total_buffer, 1, file_size, fp);
    if (read_size != file_size) {
        sprintf(info_log, "fread failed: expected %ld, got %zu", file_size, read_size);
        goto CLEAN_FILE_BUFFER;
    }

    total_buffer[read_size] = '\0';

    shader_id = compileShader(eShaderType, total_buffer, info_log, info_log_size);


CLEAN_FILE_BUFFER:
    free(total_buffer);
CLOSE_FILE:
    if (fclose(fp) == EOF) {
        fprintf(stderr, "Failed to close file :%s", full_file_path);
    }
CLEAN_FULL_FILE_PATH:
    free(full_file_path);

EXIT:

    return shader_id;
}


// Function to link vertex and fragment shaders into a program
unsigned int InitializeProgram(const char *program_id) {
    const int shader_count = 2;
    GLuint shaders[2];

    char infoLog[1024 * 10];
    const int buffer_size = 1024 * 10;
    if ((shaders[0] = LoadShader(GL_FRAGMENT_SHADER, "frag.frag", program_id, infoLog, buffer_size)) == 0) {
        fprintf(stderr, "Failed to load fragment shader for program %s -> %s \n", program_id, infoLog);
        return 0;
    }

    if ((shaders[1] = LoadShader(GL_VERTEX_SHADER, "vert.vert", program_id, infoLog, buffer_size)) == 0) {
        fprintf(stderr, "Failed to load vertex shader for program %s -> %s \n", program_id, infoLog);
        return 0;
    }

    GLuint shaderProgram = glCreateProgram();

    for (size_t i = 0; i < shader_count; ++i) {
        glAttachShader(shaderProgram, shaders[i]);
    }

    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Shader linking failed: %s \n", infoLog);
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    for (size_t i = 0; i < shader_count; ++i) {
        glDeleteShader(shaders[i]);
    }

    return shaderProgram;
}
