#include "VBO.h"

VBO VBO_InitRaw(GLfloat* vertices, GLsizeiptr size) {
    VBO vbo;
    glGenBuffers(1, &vbo.ID);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.ID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    return vbo;
}

VBO VBO_Init(VertexVector* vertices) {
    VBO vbo;
    glGenBuffers(1, &vbo.ID);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.ID);
    glBufferData(GL_ARRAY_BUFFER, vertices->size * sizeof(Vertex), vertices->data, GL_STATIC_DRAW);
    return vbo;
}

void VBO_Bind(VBO* vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo->ID);
}

void VBO_Unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO_Delete(VBO* vbo) {
    glDeleteBuffers(1, &vbo->ID);
}