#include "opengl/shadowMapFBO.h"

ShadowMapFBO ShadowMapFBO_Init(int width, int height) {
    ShadowMapFBO smfbo = {0};
    smfbo.width = width;
    smfbo.height = height;

    glGenFramebuffers(1, &smfbo.fbo);

    glGenTextures(1, &smfbo.depthTexture);
    glBindTexture(GL_TEXTURE_2D, smfbo.depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, smfbo.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, smfbo.depthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return smfbo;
}

void ShadowMapFBO_Bind(ShadowMapFBO* smfbo) {
    glViewport(0, 0, smfbo->width, smfbo->height);
    glBindFramebuffer(GL_FRAMEBUFFER, smfbo->fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMapFBO_Delete(ShadowMapFBO* smfbo) {
    glDeleteFramebuffers(1, &smfbo->fbo);
    glDeleteTextures(1, &smfbo->depthTexture);
}
