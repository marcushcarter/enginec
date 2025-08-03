#include "FBO.h"

FBO FBO_Init(int width, int height) {
    FBO fb;
    fb.width = width;
    fb.height = height;

    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

    glGenTextures(1, &fb.texture);
    glBindTexture(GL_TEXTURE_2D, fb.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture, 0);

    glGenRenderbuffers(1, &fb.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR: Framebuffer is not complete1\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLfloat vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
         1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right

        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
         1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right
         1.0f,  1.0f,   1.0f, 1.0f   // top-right
    };

    fb.vao = VAO_Init();
    VAO_Bind(&fb.vao);
    fb.vbo = VBO_InitRaw(vertices, sizeof(vertices));
    VAO_LinkAttrib(&fb.vbo, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&fb.vbo, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    
    return fb;

}

FBO FBO_InitMultiSample(int width, int height, int samples) {
    FBO fb;
    fb.width = width;
    fb.height = height;

    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

    glGenTextures(1, &fb.texture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb.texture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fb.texture, 0);

    glGenRenderbuffers(1, &fb.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb.rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR: Framebuffer is not complete1\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fb;
}

void FBO_Bind(FBO* fb) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
    glClear(GL_COLOR_BUFFER_BIT);
}

void FBO_BindTexture(FBO* fb, Shader* shader) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb->texture);
    glUniform1i(glGetUniformLocation(shader->ID, "screenTexture"), 0);
}

void FBO_Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO_Delete(FBO* fb) {
    glDeleteFramebuffers(1, &fb->fbo);
    glDeleteTextures(1, &fb->texture);
    glDeleteRenderbuffers(1, &fb->rbo);
    *fb = (FBO){0};
}

void FBO_Draw(FBO* fb) {
    VAO_Bind(&fb->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

ShadowMapFBO ShadowMapFBO_Init(int width, int height, int layers) {
    ShadowMapFBO smfbo = {0};
    smfbo.width = width;
    smfbo.height = height;
    smfbo.layers = layers;

    glGenFramebuffers(1, &smfbo.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, smfbo.fbo);

    glGenTextures(1, &smfbo.depthTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, smfbo.depthTextureArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32,
                 width, height, layers, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, smfbo.depthTextureArray, 0, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return smfbo;
}

void ShadowMapFBO_BindLayer(ShadowMapFBO* smfbo, int layer) {
    glBindFramebuffer(GL_FRAMEBUFFER, smfbo->fbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, smfbo->depthTextureArray, 0, layer);
   
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}

void ShadowMapFBO_Delete(ShadowMapFBO* smfbo) {
    glDeleteFramebuffers(1, &smfbo->fbo);
    glDeleteTextures(1, &smfbo->depthTextureArray);
}
