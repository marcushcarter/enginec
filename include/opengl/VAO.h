#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include <glad/glad.h>
#include "opengl/VBO.h"

typedef struct {
    GLuint ID;
} VAO;

VAO VAO_Init();
void VAO_LinkVBO(VAO* vao, VBO vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
void VAO_Bind(VAO* vao);
void VAO_Unbind();
void VAO_Delete(VAO* vao);

#endif