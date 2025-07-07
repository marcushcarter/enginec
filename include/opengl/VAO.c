#include "opengl/VAO.h"

VAO VAO_Init() {
    VAO vao;
    glGenVertexArrays(1, &vao.ID);
    return vao;
}

void VAO_LinkAttrib(VAO* vao, VBO* vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    VBO_Bind(vbo);
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    VBO_Unbind();
}

void VAO_Bind(VAO* vao) {
    glBindVertexArray(vao->ID);
}

void VAO_Unbind() {
    glBindVertexArray(0);
}

void VAO_Delete(VAO* vao) {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao->ID);
}