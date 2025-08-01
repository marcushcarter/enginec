#ifndef UBO_CLASS_H
#define UBO_CLASS_H

#include <stdio.h>
#include <glad/glad.h>
#include "opengl/shader.h"

typedef struct {
    GLuint ID;
    GLuint bindingPoint;
} UBO;

UBO UBO_Init(GLsizeiptr size, GLuint bindingPoint);
void UBO_Bind(UBO* ubo);
void UBO_Unbind();
void UBO_Delete(UBO* ubo);

#endif