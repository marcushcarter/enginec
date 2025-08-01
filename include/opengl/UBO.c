#include "UBO.h"

UBO UBO_Init(GLsizeiptr size, GLuint bindingPoint) {
    UBO ubo;
    ubo.bindingPoint = bindingPoint;
    glGenBuffers(1, &ubo.ID);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo.ID);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo.ID);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return ubo;
}

void UBO_Bind(UBO* ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo->ID);
}

void UBO_Unbind() {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBO_Delete(UBO* ubo) {
    glDeleteBuffers(1, &ubo->ID);
}