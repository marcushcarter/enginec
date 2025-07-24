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
} FBO;

FBO FBO_Init(int width, int height);
FBO FBO_InitMultiSample(int width, int height, int samples);
FBO FBO_InitShadowMap(int width, int height, float clampColor[4]);
void FBO_Bind(FBO* fb);
void FBO_BindTexture(FBO* fb);
void FBO_Unbind();
void FBO_Delete(FBO* fb);

#endif