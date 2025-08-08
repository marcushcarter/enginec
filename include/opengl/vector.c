#include "vector.h"
#include <stdlib.h>
#include <string.h>

// ==============================
// VertexVector
// ==============================

#define INITIAL_VERTEX_CAPACITY 8

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

void VertexVector_Copy(Vertex* vertices, size_t count, VertexVector* outVec) {
    VertexVector_Init(outVec);
    for (size_t i = 0; i < count; i++) {
        VertexVector_Push(outVec, vertices[i]);
    }
}

// ==============================
// GLuintVector
// ==============================

#define INITIAL_GLUINT_CAPACITY   8

void GLuintVector_Init(GLuintVector* vec) {
    vec->data = (GLuint*)malloc(sizeof(GLuint) * INITIAL_GLUINT_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_GLUINT_CAPACITY;
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

void GLuintVector_Copy(GLuint* data, size_t count, GLuintVector* outVec) {
    GLuintVector_Init(outVec);
    for (size_t i = 0; i < count; i++) {
        GLuintVector_Push(outVec, data[i]);
    }
}

// ==============================
// TextureVector
// ==============================

#define INITIAL_TEXTURE_CAPACITY 8

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

void TextureVector_Copy(Texture* textures, size_t count, TextureVector* outVec) {
    TextureVector_Init(outVec);
    for (size_t i = 0; i < count; i++) {
        TextureVector_Push(outVec, textures[i]);
    }
}

// ==============================
// CameraVector
// ==============================

#define INITIAL_CAMERA_CAPACITY 10

void CameraVector_Init(CameraVector* vec) {
    vec->data = (Camera**)malloc(sizeof(Camera*) * INITIAL_CAMERA_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_CAMERA_CAPACITY;
}

void CameraVector_Push(CameraVector* vec, Camera* cam) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (Camera**)realloc(vec->data, sizeof(Camera*) * vec->capacity);
    }
    vec->data[vec->size++] = cam;
}

Camera* CameraVector_Get(CameraVector* vec, size_t index) {
    return (index < vec->size) ? vec->data[index] : NULL;
}

void CameraVector_Remove(CameraVector* vec, Camera* cam) {
    for (size_t i = 0; i < vec->size; i++) {
        if (vec->data[i] == cam) {
            CameraVector_RemoveAt(vec, i);
            return;
        }
    }
}

void CameraVector_RemoveAt(CameraVector* vec, size_t index) {
    if (index >= vec->size) return;
    free(vec->data[index]);
    for (size_t i = index; i < vec->size - 1; i++) {
        vec->data[i] = vec->data[i + 1];
    }
    vec->size--;
}

size_t CameraVector_IndexOf(CameraVector* vec, Camera* cam) {
    for (size_t i = 0; i < vec->size; i++) {
        if (vec->data[i] == cam) {
            return i;
        }
    }
    return SIZE_MAX;
}

void CameraVector_Free(CameraVector* vec) {
    for (size_t i = 0; i < vec->size; i++) {
        free(vec->data[i]);
    }
    free(vec->data);
    vec->data = NULL;
    vec->size = vec->capacity = 0;
}