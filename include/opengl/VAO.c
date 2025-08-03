#include "VAO.h"

VAO VAO_Init() {
    VAO vao;
    glGenVertexArrays(1, &vao.ID);
    return vao;
}

VAO VAO_InitQuad() {
    
    GLfloat vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right

        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right
        1.0f,  1.0f,   1.0f, 1.0f   // top-right
    };
    
    VAO vao = VAO_Init();
    VAO_Bind(&vao);
    VBO vbo = VBO_InitRaw(vertices, sizeof(vertices));
    VAO_LinkAttrib(&vbo, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&vbo, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();

    return vao;
}

void VAO_LinkAttrib(VBO* vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    VBO_Bind(vbo);
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    VBO_Unbind();
}

void VAO_Bind(VAO* vao) {
    glBindVertexArray(vao->ID);
}

void VAO_DrawQuad(VAO* vao) {
    VAO_Bind(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void VAO_Unbind() {
    glBindVertexArray(0);
}

void VAO_Delete(VAO* vao) {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao->ID);
}