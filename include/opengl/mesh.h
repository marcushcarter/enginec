#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include <string.h>

#include "opengl/VAO.h"
#include "opengl/EBO.h"
#include "opengl/camera.h"
#include "opengl/texture.h"

typedef struct {
    VertexVector* vertices;
    GLuintVector* indices;
    TextureVector* textures;
    VAO vao;
} Mesh;

Mesh Mesh_Init(VertexVector* vertices, GLuintVector* indices, TextureVector* textures);
void Mesh_Draw(Mesh* mesh, Shader* shader, Camera* camera);

#endif