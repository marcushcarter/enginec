#include "shader_class.h"

char* get_file_contents(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("File open failed");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    if (length < 0) {
        perror("ftell failed");
        fclose(file);
        return NULL;
    }

    char* buffer = malloc(length + 1);
    if (!buffer) {
        perror("malloc failed");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;

}

Shader Shader_Init(const char* vertexFile, const char* fragmentFile) {
    Shader shader = {0};

    const char* vertexSource = get_file_contents(vertexFile);
    const char* fragmentSource = get_file_contents(fragmentFile);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    shader.ID = glCreateProgram();
    glAttachShader(shader.ID, vertexShader);
    glAttachShader(shader.ID, fragmentShader);
    glLinkProgram(shader.ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    free((void*)vertexSource);
    free((void*)fragmentSource);

    return shader;

}

void Shader_Activate(Shader* shader) {
    glUseProgram(shader->ID);
}

void Shader_Delete(Shader* shader) {
    glDeleteProgram(shader->ID);
}