#ifndef FBO_CLASS_H
#define FBO_CLASS_H

#include <stdio.h>
#include <glad/glad.h>
#include "opengl/shader.h"

typedef struct {
    GLuint fbo;
    GLuint texture;
    GLuint rbo;
    int width, height;
} Framebuffer;

Framebuffer Framebuffer_Init(int width, int height);
Framebuffer Framebuffer_InitMultiSample(int width, int height, int samples);
void Framebuffer_Bind(Framebuffer* fb);
void Framebuffer_BindTexture(Framebuffer* fb);
void Framebuffer_Unbind();
void Framebuffer_Delete(Framebuffer* fb);

#endif