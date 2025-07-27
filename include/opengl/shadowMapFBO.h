#ifndef SHADOW_MAP_FBO_CLASS_H
#define SHADOW_MAP_FBO_CLASS_H

#include <glad/glad.h>
#include "opengl/FBO.h"

typedef struct {
    GLuint fbo;
    GLuint depthTextureArray;
    int width;
    int height;
    int layers;
} ShadowMapFBO;

ShadowMapFBO ShadowMapFBO_Init(int width, int height, int layers);
void ShadowMapFBO_BindLayer(ShadowMapFBO* smfbo, int layer);
void ShadowMapFBO_Unbind(void);
void ShadowMapFBO_Delete(ShadowMapFBO* smfbo);

#endif
