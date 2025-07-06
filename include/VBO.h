#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <glad/glad.h>

typedef struct {
    GLuint ID;
} VBO;

VBO VBO_Init(GLfloat* vertices, GLsizeiptr size);
void VBO_Bind(VBO* vbo);
void VBO_Unbind();
void VBO_Delete(VBO* vbo);

#endif