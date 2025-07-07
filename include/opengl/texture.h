#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <stdbool.h>

#include "opengl/shader.h"

typedef struct {
    GLuint ID;
    GLenum type;
} Texture;

Texture Texture_Init(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);
void Texture_texUnit(Shader* shader, const char* uniform, GLuint unit);
void Texture_Bind(Texture* texture);
void Texture_Unbind(Texture* texture);
void Texture_Delete(Texture* texture);

#endif