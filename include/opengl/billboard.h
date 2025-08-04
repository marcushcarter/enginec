#ifndef BILLBOARD_CLASS_H
#define BILLBOARD_CLASS_H

#include <cglm/cglm.h>
#include <glad/glad.h>

typedef struct {
    vec3 position;
    vec2 size;
    GLuint textureID;
} Billboard;



#endif