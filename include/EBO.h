#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include <glad/glad.h>

typedef struct {
    GLuint ID;
} EBO;

EBO EBO_Init(GLuint* indices, GLsizeiptr size);
void EBO_Bind(EBO* ebo);
void EBO_Unbind();
void EBO_Delete(EBO* ebo);

#endif