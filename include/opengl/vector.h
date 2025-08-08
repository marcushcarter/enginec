#ifndef VERTEX_VECTOR_CLASS_H
#define VERTEX_VECTOR_CLASS_H

#include <cglm/cglm.h>
#include <glad/glad.h>
#include "texture.h"
#include "camera.h"


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
void VertexVector_Copy(Vertex* vertices, size_t count, VertexVector* outVec);

typedef struct {
    GLuint* data;
    size_t size;
    size_t capacity;
} GLuintVector;

void GLuintVector_Init(GLuintVector* vec);
void GLuintVector_Push(GLuintVector* vec, GLuint value);
void GLuintVector_Free(GLuintVector* vec);
void GLuintVector_Copy(GLuint* data, size_t count, GLuintVector* outVec);

typedef struct {
    Texture* data;
    size_t size;
    size_t capacity;
} TextureVector;

void TextureVector_Init(TextureVector* vec);
void TextureVector_Push(TextureVector* vec, Texture value);
void TextureVector_Free(TextureVector* vec);
void TextureVector_Copy(Texture* textures, size_t count, TextureVector* outVec);

typedef struct {
    Camera** data;
    size_t size;
    size_t capacity;
} CameraVector;

void CameraVector_Init(CameraVector* vec);
void CameraVector_Push(CameraVector* vec, Camera* cam);
Camera* CameraVector_Get(CameraVector* vec, size_t index);
void CameraVector_RemoveAt(CameraVector* vec, size_t index);
void CameraVector_Free(CameraVector* vec);

#endif