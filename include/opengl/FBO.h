#ifndef FBO_CLASS_H
#define FBO_CLASS_H

#include <stdio.h>
#include <glad/glad.h>
#include "opengl/shader.h"
#include "VBO.h"
#include "VAO.h"

typedef struct {
    GLuint fbo;
    GLuint texture;
    GLuint rbo;
    VAO vao;
    VBO vbo;
    int width, height;
} FBO;

FBO FBO_Init(int width, int height);
FBO FBO_InitMultiSample(int width, int height, int samples);
void FBO_Resize(FBO* fbo, int newWidth, int newHeight);
FBO FBO_InitShadowMap(int width, int height, float clampColor[4]);
void FBO_Bind(FBO* fb);
void FBO_BindTexture(FBO* fb, Shader* shader);
void FBO_Unbind();
void FBO_Delete(FBO* fb);
void FBO_Draw(FBO* fb);
void FBO_Clear(int width, int height);

typedef struct {
    GLuint fbo;
    GLuint depthTextureArray;
    int width, height;
    int layers;
} ShadowMapFBO;

ShadowMapFBO ShadowMapFBO_Init(int width, int height, int layers);
void ShadowMapFBO_BindLayer(ShadowMapFBO* smfbo, int layer);
void ShadowMapFBO_Delete(ShadowMapFBO* smfbo);

#endif