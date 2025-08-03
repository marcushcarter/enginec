#ifndef SPRITE_CLASS_H
#define SPRITE_CLASS_H

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image/stb_image_resize.h>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

typedef struct {
    GLuint textureArray;
    int width;
    int height;
    int count;
} Sprite;

typedef struct {
    vec2 position;
    vec2 size;
    float rotation;
    vec4 color;
    int layer;
} SpriteInstance;

Sprite Sprite_Init(const char** filenames, int count);

// void Sprite_Draw(Sprite* sprite, Shader* shader, SpriteInstance* inst, VAO* vao);

void Sprite_Draw(Sprite* sprite, Shader* shader, SpriteInstance* inst, mat4 projection, VAO* vao);

#endif