#ifndef VERTEX_VECTOR_CLASS_H
#define VERTEX_VECTOR_CLASS_H

#include <cglm/cglm.h>
#include <glad/glad.h>
#include "opengl/texture.h"


typedef struct {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 texUV;
} Vertex;

typedef struct {
    Vertex* data;
    size_t size;
    size_t capacity;
} VertexVector;

void VertexVector_Init(VertexVector* vec);
void VertexVector_Push(VertexVector* vec, Vertex value);
void VertexVector_Free(VertexVector* vec);

typedef struct {
    GLuint* data;
    size_t size;
    size_t capacity;
} GLuintVector;

void GLuintVector_Init(GLuintVector* vec);
void GLuintVector_Push(GLuintVector* vec, GLuint value);
void GLuintVector_Free(GLuintVector* vec);

typedef struct {
    Texture* data;
    size_t size;
    size_t capacity;
} TextureVector;

void TextureVector_Init(TextureVector* vec);
void TextureVector_Push(TextureVector* vec, Texture value);
void TextureVector_Free(TextureVector* vec);


#endif