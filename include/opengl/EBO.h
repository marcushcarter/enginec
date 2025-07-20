#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include <glad/glad.h>
#include "opengl/vector.h"

typedef struct {
    GLuint ID;
} EBO;

EBO EBO_InitRaw(GLuint* indices, GLsizeiptr size);
EBO EBO_Init(GLuintVector* indices);
void EBO_Bind(EBO* ebo);
void EBO_Unbind();
void EBO_Delete(EBO* ebo);

#endif