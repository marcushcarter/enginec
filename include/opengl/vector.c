#include "opengl/vector.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_VERTEX_CAPACITY 8
#define INITIAL_UINT_CAPACITY   8
#define INITIAL_TEXTURE_CAPACITY 8

// ==============================
// VertexVector
// ==============================

void VertexVector_Init(VertexVector* vec) {
    vec->data = (Vertex*)malloc(sizeof(Vertex) * INITIAL_VERTEX_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_VERTEX_CAPACITY;
}

void VertexVector_Push(VertexVector* vec, Vertex value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (Vertex*)realloc(vec->data, sizeof(Vertex) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void VertexVector_Free(VertexVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

// ==============================
// GLuintVector
// ==============================

void GLuintVector_Init(GLuintVector* vec) {
    vec->data = (GLuint*)malloc(sizeof(GLuint) * INITIAL_UINT_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_UINT_CAPACITY;
}

void GLuintVector_Push(GLuintVector* vec, GLuint value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (GLuint*)realloc(vec->data, sizeof(GLuint) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void GLuintVector_Free(GLuintVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

// ==============================
// TextureVector
// ==============================

void TextureVector_Init(TextureVector* vec) {
    vec->data = (Texture*)malloc(sizeof(Texture) * INITIAL_TEXTURE_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_TEXTURE_CAPACITY;
}

void TextureVector_Push(TextureVector* vec, Texture value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (Texture*)realloc(vec->data, sizeof(Texture) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void TextureVector_Free(TextureVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}
