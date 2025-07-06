#include "opengl/EBO.h"

EBO EBO_Init(GLuint* indices, GLsizeiptr size) {
    EBO ebo;
    glGenBuffers(1, &ebo.ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    return ebo;
}

void EBO_Bind(EBO* ebo) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->ID);
}

void EBO_Unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO_Delete(EBO* ebo) {
    glDeleteBuffers(1, &ebo->ID);
}