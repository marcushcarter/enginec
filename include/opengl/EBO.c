#include "opengl/EBO.h"

EBO EBO_InitRaw(GLuint* indices, GLsizeiptr size) {
    EBO ebo;
    glGenBuffers(1, &ebo.ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    return ebo;
}

EBO EBO_Init(GLuintVector* indices) {
    EBO ebo;
    glGenBuffers(1, &ebo.ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size * sizeof(GLuint), indices->data, GL_STATIC_DRAW);
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