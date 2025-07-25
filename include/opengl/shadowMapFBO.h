#ifndef SHADOW_MAP_FBO_CLASS_H
#define SHADOW_MAP_FBO_CLASS_H

#include <glad/glad.h>
#include "opengl/FBO.h"

typedef struct {
    GLuint fbo;
    GLuint depthTexture;
    int width;
    int height;
} ShadowMapFBO;

ShadowMapFBO ShadowMapFBO_Init(int width, int height);
void ShadowMapFBO_Bind(ShadowMapFBO* smfbo);
void ShadowMapFBO_Unbind(void);
void ShadowMapFBO_Delete(ShadowMapFBO* smfbo);

#endif
