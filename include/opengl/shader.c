#include "shader.h"
#include <string.h>

void Shader_compileErrors(unsigned int shader, const char* type) {
    GLint hasCompiled;
    char infolog[1024];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetShaderInfoLog(shader, 1024, NULL, infolog);
            printf("SHADER_COMPILATION_ERROR for: %s\n%s\n", type, infolog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetProgramInfoLog(shader, 1024, NULL, infolog);
            printf("SHADER_LINKING_ERROR for: %s\n%s\n", type, infolog);
        }
    }
}

char* get_file_contents(const char* filename) {
    FILE* file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* buffer = malloc(length + 1);

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;

}

Shader Shader_Init(const char* vertexFile, const char* fragmentFile, const char* geometryFile) {
    Shader shader = {0};

    const char* vertexSource = get_file_contents(vertexFile);
    const char* fragmentSource = get_file_contents(fragmentFile);
    const char* geometrySource = NULL;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    Shader_compileErrors(vertexShader, "VERTEX");
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    Shader_compileErrors(fragmentShader, "FRAGMENT");

    GLuint geometryShader = 0;
    if (geometryFile != NULL) {
        geometrySource = get_file_contents(geometryFile);
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geometrySource, NULL);
        glCompileShader(geometryShader);
        Shader_compileErrors(geometryShader, "GEOMETRY");
    }

    shader.ID = glCreateProgram();
    glAttachShader(shader.ID, vertexShader);
    glAttachShader(shader.ID, fragmentShader);
    if (geometryFile != NULL) glAttachShader(shader.ID, geometryShader);
    glLinkProgram(shader.ID);
    Shader_compileErrors(shader.ID, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryFile != NULL) glDeleteShader(geometryShader);

    free((void*)vertexSource);
    free((void*)fragmentSource);
    if (geometryFile != NULL) free((void*)geometrySource);

    return shader;

}

void Shader_Activate(Shader* shader) {
    glUseProgram(shader->ID);
}

void Shader_Delete(Shader* shader) {
    glDeleteProgram(shader->ID);
}