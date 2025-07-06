#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct {
    GLuint ID;
} Shader;

void Shader_compileErrors(unsigned int shader, const char* type);
char* get_file_contents(const char* filename);
Shader Shader_Init(const char* vertexFile, const char* fragmentFile);
void Shader_Activate(Shader* shader);
void Shader_Delete(Shader* shader);

#endif