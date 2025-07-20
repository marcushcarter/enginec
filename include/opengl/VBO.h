#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "opengl/vector.h"


typedef struct {
    GLuint ID;
} VBO;

VBO VBO_InitRaw(GLfloat* vertices, GLsizeiptr size);
VBO VBO_Init(VertexVector* vertices);
void VBO_Bind(VBO* vbo);
void VBO_Unbind();
void VBO_Delete(VBO* vbo);

#endif