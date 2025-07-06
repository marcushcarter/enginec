#include "VAO.h"

VAO VAO_Init() {
    VAO vao;
    glGenVertexArrays(1, &vao.ID);
    return vao;
}

void VAO_LinkVBO(VAO* vao, VBO vbo, GLuint layout) {
    VBO_Bind(&vbo);
    glVertexAttribPointer(layout, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
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